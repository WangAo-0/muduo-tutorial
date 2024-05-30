#include "RelayServer.h"
#include "MessageHeader.h"
#include "Utils.h"
#include "muduo/base/Logging.h"
#include "muduo/net/Callbacks.h"
#include <bits/stdint-uintn.h>
#include <functional>

RelayServer::RelayServer(EventLoop *loop, const InetAddress &listenAddr,
                         int numThreads, bool enableWaterMark)
    : server_(loop, listenAddr, "RelayServer"), loop_(loop) {
  if (enableWaterMark) {
    enableHighWaterMark();
  }
  server_.setConnectionCallback(
      std::bind(&RelayServer::onConnection, this, _1));
  server_.setMessageCallback(
      std::bind(&RelayServer::onMessage, this, _1, _2, _3));
  // server_.setMessageCallback(
  //     std::bind(&RelayServer::onImmediateForward, this, _1, _2, _3));
  server_.setThreadNum(numThreads);
}

void RelayServer::start() { server_.start(); }

/**
 * @brief 检查输出缓冲区水位是否下降了
 * 1. 如果下降了，继续接受数据
 * @param conn
 */
void RelayServer::onWriteComplete(const TcpConnectionPtr &conn) {
  if (isEnableHighWaterMark() &&
      conn->outputBuffer()->readableBytes() < highWaterMark()) {
    LOG_INFO << conn->name()
             << " : 低了,outbuffer:" << conn->outputBuffer()->readableBytes()
             << ",inputbuffer :" << conn->inputBuffer()->readableBytes();
    conn->startRead();
    conn->setWriteCompleteCallback(WriteCompleteCallback());
  }
}

/**
 * @brief 发送缓冲区达到水位时
 * 1. 暂停接受数据
 * 2. 注册回调函数，当缓冲区数据量降到一定程度时，继续接受数据
 */
void RelayServer::onHighWaterMarkCallback(const TcpConnectionPtr &conn) {
  LOG_INFO << "RelayServer - HighWaterMarkCallback";
  conn->stopRead();
  conn->setWriteCompleteCallback(
      std::bind(&RelayServer::onWriteComplete, this, conn));
}

void RelayServer::onConnection(const TcpConnectionPtr &conn) {
  if (conn->connected()) {
    conn->setHighWaterMarkCallback(
        std::bind(&RelayServer::onHighWaterMarkCallback, this, conn),
        highWaterMark());
    LOG_INFO << "RelayServer - New connection from "
             << conn->peerAddress().toIpPort();

    // 发送一个欢迎消息，告诉客户端连接成功
    // @deprecated(旧代码采用停等 依赖这个启动)
    // 同时也是一个启动消息，触发客户端的接收消息，接受完消息后，客户端会开始发送消息
    MessageHeader header;
    string message = "Welcome to RelayServer!";
    header.flag = 2;                       // 标志位
    header.messageLength = message.size(); // 数据长度
    header.senderID = 0;                   // 发送方ID,0是服务器
    header.targetID = 0;                   // 目标ID
    header.sendTime = muduo::Timestamp::now().microSecondsSinceEpoch();
    char headerAndData[sizeof(MessageHeader) + message.size()];
    memcpy(headerAndData, &header, sizeof(MessageHeader));
    memcpy(headerAndData + sizeof(MessageHeader), message.c_str(),
           message.size());
    conn->send(headerAndData, sizeof(MessageHeader) + message.size());
  } else {
    // 找到conn->name()对应的id，再从clientMaps_中删除conn,最后再删除
    conn->getLoop()->runInLoop([this, conn]() {
      {
        tbb::concurrent_hash_map<uint32_t, TcpConnectionPtr>::accessor ac;
        if (clientsMap_.find(ac, conn->id_)) {
          clientsMap_.erase(conn->id_);
        }
        count++;
      }
      LOG_INFO << "RelayServer - Connection from "
               << conn->peerAddress().toIpPort() << " is down";
    });
    // if (count == 100)
    //   server_.getLoop()->quit();
    // 接收到A关闭了写端，记录一下，当B也关闭写端时，关闭连接
  }
}

void RelayServer::onImmediateForward(const TcpConnectionPtr &conn, Buffer *buf,
                                     Timestamp time) {
  if (conn->id_ != 0) { // 说明创建成了
    {
      tbb::concurrent_hash_map<uint32_t, TcpConnectionPtr>::accessor ac;
      if (clientsMap_.find(ac, conn->peer_id_)) {
        string data = buf->retrieveAllAsString();
        ac->second->send(data);
        return;
      }
    }
  }

  if (conn->id_ == 0 && buf->readableBytes() >= sizeof(MessageHeader)) {
    MessageHeader header;
    memcpy(&header, buf->peek(), sizeof(MessageHeader));
    // 记录客户端ID
    if (header.flag == 1) { // 连接建立发送消息
      {
        tbb::concurrent_hash_map<uint32_t, TcpConnectionPtr>::accessor ac;
        if (clientsMap_.insert(ac, header.senderID)) {
          ac->second = conn;
          conn->id_ = header.senderID;
          conn->peer_id_ = header.targetID;
          buf->retrieve(sizeof(MessageHeader));
        }
      }
    }
  }
}

/**
 * @brief 中继服务器获取数据，再转发
 *
 * @param conn
 * @param buf
 * @param time
 */
void RelayServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf,
                            Timestamp time) {
  // 检查缓冲区中是否只少有一个报头大小的数据
  while (buf->readableBytes() >= sizeof(MessageHeader)) {
    // 获取报头
    MessageHeader header;
    memcpy(&header, buf->peek(), sizeof(MessageHeader));

    // 记录客户端ID
    if (header.flag == 1) { // 连接建立发送消息
      {
        tbb::concurrent_hash_map<uint32_t, TcpConnectionPtr>::accessor ac;
        if (clientsMap_.insert(ac, header.senderID)) {
          ac->second = conn;
          conn->id_ = header.senderID;
        }
      }
      buf->retrieve(sizeof(MessageHeader));
    } else if (header.flag == 0) { // 建立后发送消息
      // 检查数据是否完整，不完整则等下一次读取
      if (buf->readableBytes() < sizeof(MessageHeader) + header.messageLength) {
        break;
      }

      // 完整，则找到目标ID对应的客户端，发送消息
      {
        // FixMe: 没加锁
        // std::lock_guard<std::mutex> lock(myMutex);
        tbb::concurrent_hash_map<uint32_t, TcpConnectionPtr>::accessor ac;
        if (clientsMap_.find(ac, header.targetID)) {
          buf->retrieve(sizeof(MessageHeader));
          // 获取数据
          string message = buf->retrieveAsString(header.messageLength);
          header.flag = 2;
          char headerAndData[message.size() + sizeof(MessageHeader)];
          memcpy(headerAndData, &header, sizeof(MessageHeader));
          memcpy(headerAndData + sizeof(MessageHeader), message.c_str(),
                 message.size());
          // 发送数据
          ac->second->send(headerAndData,
                           sizeof(MessageHeader) + message.size());
        } else {
          break;
        }
      }
    } else // 基本不会走这里
    {
      LOG_WARN << "Invalid flag: " << static_cast<int>(header.flag);
      buf->retrieve(sizeof(MessageHeader));
    }
  }
}

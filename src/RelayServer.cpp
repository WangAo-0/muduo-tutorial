#include "RelayServer.h"
#include "MessageHeader.h"
#include "muduo/base/Logging.h"
#include "Utils.h"

RelayServer::RelayServer(EventLoop *loop, const InetAddress &listenAddr,
                         int numThreads)
    : server_(loop, listenAddr, "RelayServer") {
  server_.setConnectionCallback(
      std::bind(&RelayServer::onConnection, this, _1));
  server_.setMessageCallback(
      std::bind(&RelayServer::onMessage, this, _1, _2, _3));
  server_.setThreadNum(numThreads);
}

void RelayServer::start() { server_.start(); }

void RelayServer::onConnection(const TcpConnectionPtr &conn) {
  LOG_INFO << "RelayServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");

  // 发送一个欢迎消息，告诉客户端连接成功
  // 同时也是一个启动消息，触发客户端的接收消息，接受完消息后，客户端会开始发送消息
  MessageHeader header;
  string message = "Welcome to RelayServer!";
  header.flag = 2;                      // 标志位
  header.messageLength = message.size(); // 数据长度
  header.senderID = 0;                  // 发送方ID,0是服务器
  header.targetID = 0;                  // 目标ID
  header.sendTime = muduo::Timestamp::now().microSecondsSinceEpoch();
  char headerAndData[sizeof(MessageHeader) + message.size()];
  memcpy(headerAndData, &header, sizeof(MessageHeader));
  memcpy(headerAndData + sizeof(MessageHeader), message.c_str(), message.size());
  conn->send(headerAndData, sizeof(MessageHeader) + message.size());
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
      clientsMap_[header.senderID] = conn;
      buf->retrieve(sizeof(MessageHeader));
    } else if (header.flag == 0) { // 建立后发送消息

      // 检查数据是否完整，不完整则等下一次读取
      if (buf->readableBytes() < sizeof(MessageHeader) + header.messageLength) {
        break;
      }

      // 完整，则找到目标ID对应的客户端，发送消息
      auto it = clientsMap_.find(header.targetID);
      if (it != clientsMap_.end()) {
        buf->retrieve(sizeof(MessageHeader));

        // 获取数据
        string message = buf->retrieveAsString(header.messageLength);
        header.flag = 2;
        char headerAndData[message.size()+sizeof(MessageHeader)];
        memcpy(headerAndData, &header, sizeof(MessageHeader));
        memcpy(headerAndData+sizeof(MessageHeader), message.c_str(), message.size());
        // 发送数据
        it->second->send(headerAndData, sizeof(MessageHeader)+message.size());
      } else {
        // 1. 直接抛弃消息
        LOG_WARN << "Target client not found: " << header.targetID;
        buf->retrieve(sizeof(MessageHeader) + header.messageLength);

        // 2. 抛弃消息，返回错误信息给发送方
        // LOG_WARN << "Target client not found: " << header.targetID;
        // buf->retrieve(sizeof(MessageHeader) + header.messageLength);
        // string errorMessage = "Target client "+
        // std::to_string(header.targetID) + " is offline. " ;
        // conn->send(errorMessage);

        // 3. 保存消息，等目标ID上线后再发送
        // LOG_WARN << "Target client not found: " << header.targetID;
        // buf->retrieve(sizeof(MessageHeader));
        // string message = buf->retrieveAsString(header.messageLength);
        // offlineMessages_[header.targetID].push_back(message);
      }
    } else // 基本不会走这里
    {
      LOG_WARN << "Invalid flag: " << static_cast<int>(header.flag);
      buf->retrieve(sizeof(MessageHeader));
    }
  }
}

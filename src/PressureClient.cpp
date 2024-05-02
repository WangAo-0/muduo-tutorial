#include "PressureClient.h"
#include "Utils.h"
#include "muduo/base/Logging.h"

PressureClient::PressureClient(muduo::net::EventLoop *loop,
                               const muduo::net::InetAddress &serverAddr,
                               const uint32_t &id, int messageSize,
                               int messageCount, bool unlimitedSend)
    : client_(loop, serverAddr, std::to_string(id)), id_(id),
      messageSize_(messageSize), sendMessageCount_(messageCount),
      recvMessageCount_(messageCount), unlimitedSend_(unlimitedSend) {
  client_.setConnectionCallback(
      std::bind(&PressureClient::onConnection, this, std::placeholders::_1));
  client_.setMessageCallback(
      std::bind(&PressureClient::onMessage, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3));
  client_.setWriteCompleteCallback(
      std::bind(&PressureClient::onWriteComplete, this, std::placeholders::_1));
}

void sendNextMessage(const muduo::net::TcpConnectionPtr &conn,
                     const uint32_t &id, int &messageSize) {
  MessageHeader header;
  header.senderID = id;
  header.flag = Constants::SEND_MSG;
  header.targetID = id % 2 == 0 ? id - 1 : id + 1;
  header.sendTime = muduo::Timestamp::now().microSecondsSinceEpoch();
  std::vector<char> data(messageSize, 'a');
  header.messageLength = data.size();

  muduo::net::Buffer buffer;
  buffer.append(&header, sizeof(MessageHeader)); // 添加 header 到缓冲区
  buffer.append(data.data(), data.size());       // 添加数据到缓冲区

  conn->send(&buffer); // 一次性发送所有数据
}

/**
 * @brief 当所有发送的消息都发送完毕后，偶数的客户端先关闭连接
 * 因为是奇数先发的，所以奇数的客户端需要等待偶数的客户端发送完消息后，再关闭连接
 *
 * @param conn
 */
void PressureClient::onWriteComplete(const muduo::net::TcpConnectionPtr &conn) {

  // 2000,
  // 2
  // 1 : 1
  // 2 : 0
  // 最开始有一条，所以，应该是3：-1，2000：-1
  LOG_INFO << "sendMessageCount_ " << sendMessageCount_ << " Client " << id_
           << " onWriteComplete";
  if (unlimitedSend_) {
    sendNextMessage(conn, id_, messageSize_);
    sendMessageCount_--;
    return;
  }
  // 只发送100条数据，当发送完毕后且接收完毕后，关闭连接
  if (sendMessageCount_ == -1) {
    // LOG_INFO << "Client " << id_ << " sendNextMessage " << conn->bytesSend_;
    if (recvMessageCount_ == 0) {
      if (conn->connected()) {
        conn->forceClose();
        LOG_INFO << "WOAINI";
      }
    }
    // conn->shutdown();
    return;
  }

  // 检查数据Buffer是否过多，如果过多则停止发送，并且最好是再监听可写事件，但是muduo库这里没有提供
  sendNextMessage(conn, id_, messageSize_);
  sendMessageCount_--;
}

void PressureClient::connect() { client_.connect(); }

void PressureClient::onConnection(const muduo::net::TcpConnectionPtr &conn) {
  // 处理连接建立的逻辑
  if (conn->connected()) {
    isConnected_ = true;
    MessageHeader header;
    header.senderID = id_;
    header.flag = Constants::CONN_MSG;
    char headerChars[sizeof(MessageHeader)];
    memcpy(headerChars, &header, sizeof(MessageHeader));
    conn->send(headerChars, sizeof(MessageHeader));
  } else {
    // 连接断开的逻辑，这里简单打印一下，实际应用中可能需要重连，或者其他处理，比如记录日志，或者通知其他模块，等等
    LOG_INFO << "Client " << id_ << " disconnected";
    isConnected_ = false;
    forceCloseCalled_ = false;
    if (closeCallback_) {
      closeCallback_();
    }
  }
}

/**
 * @brief 处理完整的消息
 *
 * @param conn
 * @param buf
 * @param receiveTime
 * @param header
 */
void PressureClient::doHandleReadyMessage(
    const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf,
    muduo::Timestamp receiveTime, MessageHeader &header) {

  buf->retrieve(sizeof(MessageHeader));
  std::string message = buf->retrieveAsString(header.messageLength);
  remainingMessageSize_ = 0;

  // 2. 从消息中解析出发送时间，计算延迟
  muduo::Timestamp sendTime(header.sendTime);
  double delay = timeDifference(receiveTime, sendTime);

  // 3. 打印消息
  if (header.senderID == Constants::SERVER_ID) {
    // 服务器发来的消息，说明连接建立成功
    LOG_INFO << "RelaySever :" << message << " Client " << id_
             << " , delay: " << delay << "ms";
  } else {
    // 客户端发来的消息
    LOG_INFO << " Client " << header.senderID << " to "
             << "Client " << id_ << " : " << message << " , delay: " << delay
             << "ms";
    recvMessageCount_--;
    LOG_INFO << "Client " << id_
             << " recvMessageCount_ = " << recvMessageCount_;
    // 接收先完成，则等待发送端关闭
    // 发送先完成，则等待接收端关闭
    if (recvMessageCount_ == 0 && sendMessageCount_ == -1) {
      if (conn->connected()) {
        conn->forceClose();
        LOG_INFO << "WOAINI";
      }
    }
  }
}

/**
 * @brief 处理消息
 *
 * @param conn
 * @param buf
 * @param receiveTime
 * @return true
 * @return false
 */
bool PressureClient::handleMessage(const muduo::net::TcpConnectionPtr &conn,
                                   muduo::net::Buffer *buf,
                                   muduo::Timestamp receiveTime) {

  MessageHeader header;
  memcpy(&header, buf->peek(), sizeof(header));

  // 1. 是否有缓存消息，如果有，先处理缓存消息
  if (remainingMessageSize_ > 0) {
    if (buf->readableBytes() >= remainingMessageSize_) {
      doHandleReadyMessage(conn, buf, receiveTime, header);
      return true;
    } else
      return false;
  }
  // 2. 处理新消息
  else {
    // TODO 如果有很多次的话，可能需要告知服务器端，然后重连或者断开连接
    if (header.flag != Constants::SEVR_MSG) {
      // 接收消息的flag都是2，如果不是2，说明有错误
      LOG_ERROR << "Error message flag: " << header.flag;
      buf->retrieveAll();
      return false;
    }

    // 如果消息完整，处理消息
    if (buf->readableBytes() >= sizeof(MessageHeader) + header.messageLength) {
      doHandleReadyMessage(conn, buf, receiveTime, header);
      return true;
    } else {
      remainingMessageSize_ = header.messageLength + sizeof(MessageHeader);
      LOG_INFO << "Message not complete";
      return false;
    }
  }
}

void PressureClient::onMessage(const muduo::net::TcpConnectionPtr &conn,
                               muduo::net::Buffer *buf,
                               muduo::Timestamp receiveTime) {
  while (buf->readableBytes() >= sizeof(MessageHeader)) {
    bool hasReadyMessage = handleMessage(conn, buf, receiveTime);
    if (!hasReadyMessage) {
      break;
    }
  }
}

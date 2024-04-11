#include "PressureClient.h"
#include "MessageHeader.h"
#include "muduo/base/Logging.h"
PressureClient::PressureClient(muduo::net::EventLoop *loop,
                               const muduo::net::InetAddress &serverAddr,
                               const uint32_t &id, int messageSize)
    : client_(loop, serverAddr, std::to_string(id)), id_(id),
      messageSize_(messageSize) {
  client_.setConnectionCallback(
      std::bind(&PressureClient::onConnection, this, std::placeholders::_1));
  client_.setMessageCallback(
      std::bind(&PressureClient::onMessage, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3));
}

void PressureClient::connect() { client_.connect(); }

void PressureClient::onConnection(const muduo::net::TcpConnectionPtr &conn) {
  // 处理连接建立的逻辑
  if (conn->connected()) {
    MessageHeader header;
    header.senderID = id_;
    header.flag = 1;
    char headerChars[sizeof(MessageHeader)];
    memcpy(headerChars, &header, sizeof(MessageHeader));
    conn->send(headerChars, sizeof(MessageHeader));
  } else {
    // 连接断开的逻辑，这里简单打印一下，实际应用中可能需要重连，或者其他处理，比如记录日志，或者通知其他模块，等等
  }
}

void PressureClient::onMessage(const muduo::net::TcpConnectionPtr &conn,
                               muduo::net::Buffer *buf,
                               muduo::Timestamp receiveTime) {
  while (buf->readableBytes() >= sizeof(MessageHeader)) {
    MessageHeader header;
    memcpy(&header, buf->peek(), sizeof(header));
    if (header.flag != 2) {
      LOG_ERROR << "Error message flag: " << header.flag;
      buf->retrieveAll();
      // TODO 如果有很多次的话，可能需要告知服务器端，然后重连或者断开连接
      break;
    }

    if (buf->readableBytes() >= sizeof(MessageHeader) + header.messageLength) {
      buf->retrieve(sizeof(MessageHeader));
      std::string message = buf->retrieveAsString(header.messageLength);

      // 从消息中解析出发送时间
      muduo::Timestamp sendTime(header.sendTime);
      // 计算延迟
      double delay = timeDifference(receiveTime, sendTime);
      if (header.senderID == 0) {
        // 服务器发来的消息，说明连接建立成功
        LOG_INFO << "RelaySever :" << message << " Client " << id_
                 << " , delay: " << delay << "ms";
      } else {
        // 客户端发来的消息
        LOG_INFO << "Client " << header.senderID << " to "
                 << "Client " << id_ << " : " << message
                 << " , delay: " << delay << "ms";
      }
    } else {
      // 消息不完整，等待下次读取
      break;
    }
  }

  // 发送消息，ID为偶，则发给ID - 1,ID为奇，则发给ID + 1
  MessageHeader header;
  header.senderID = id_;
  header.flag = 0;
  header.targetID = id_ % 2 == 0 ? id_ - 1 : id_ + 1;
  header.sendTime = muduo::Timestamp::now().microSecondsSinceEpoch();
  std::string message("Hello, I'm client " + std::to_string(id_));
  header.messageLength = message.size();

  char headerAndData[message.size() + sizeof(MessageHeader)];
  memcpy(headerAndData, &header, sizeof(MessageHeader));
  memcpy(headerAndData + sizeof(MessageHeader), message.c_str(),
         message.size());
  conn->send(headerAndData, sizeof(MessageHeader) + message.size());
}

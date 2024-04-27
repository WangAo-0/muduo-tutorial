// message_header.h

#ifndef MESSAGE_HEADER_H
#define MESSAGE_HEADER_H

#include <algorithm>
#include <cstdint>

struct MessageHeader {
  MessageHeader() {
    std::fill_n(reinterpret_cast<char *>(this), sizeof(*this), 0);
  }
  // 标志位
  // 连接建立前客户端发送消息1
  // 建立后客户端发送消息0
  // 服务器发送消息类型是2

  uint8_t flag;
  uint32_t senderID;      // 发送者的ID，0 是服务器
  uint32_t messageLength; // 消息内容的长度
  uint32_t targetID;      // 目标ID
  int64_t sendTime;       // 源发送时间
};

#endif // MESSAGE_HEADER_H

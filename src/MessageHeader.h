// message_header.h

#ifndef MESSAGE_HEADER_H
#define MESSAGE_HEADER_H

#include <cstdint>

struct MessageHeader {
  // 标志位,连接建立发送消息1,建立后发送消息0,2是接收消息类型
  uint8_t flag;
  uint32_t senderID;      // 发送者的ID，0 是服务器
  uint32_t messageLength; // 消息内容的长度
  uint32_t targetID;      // 目标ID
  int64_t sendTime;      // 源发送时间
};

#endif // MESSAGE_HEADER_H

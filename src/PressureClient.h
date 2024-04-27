#ifndef PRESSURECLIENT_H
#define PRESSURECLIENT_H

#include <muduo/base/Timestamp.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpClient.h>
#include "MessageHeader.h"

class PressureClient {
public:
  PressureClient(muduo::net::EventLoop *loop,
                 const muduo::net::InetAddress &serverAddr, const uint32_t &id,
                 int messageSize, int messageCount);

  void connect();

  typedef std::function<void()> CloseCallback;
  void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }
  bool  isConnected() const { return isConnected_; }  
  bool forceCloseCalled() const { return forceCloseCalled_; }

private:
  const uint32_t id_;
  void onConnection(const muduo::net::TcpConnectionPtr &conn);
  void onMessage(const muduo::net::TcpConnectionPtr &conn,
                 muduo::net::Buffer *buf, muduo::Timestamp time);
  void onWriteComplete(const muduo::net::TcpConnectionPtr &conn);
  CloseCallback closeCallback_;

  muduo::net::TcpClient client_;
  int messageSize_;
  int sendMessageCount_;
  int recvMessageCount_ ;
  uint32_t remainingMessageSize_ = 0;
  bool isConnected_ = false;
  bool forceCloseCalled_ = false;

  bool handleMessage(const muduo::net::TcpConnectionPtr &conn,
                     muduo::net::Buffer *buf, muduo::Timestamp receiveTime);
  void doHandleReadyMessage(const muduo::net::TcpConnectionPtr &conn,
                                    muduo::net::Buffer *buf,
                                    muduo::Timestamp receiveTime, MessageHeader &header);
};

#endif // PRESSURECLIENT_H

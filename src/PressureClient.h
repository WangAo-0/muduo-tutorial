#ifndef PRESSURECLIENT_H
#define PRESSURECLIENT_H

#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/Buffer.h>
#include <muduo/base/Timestamp.h>

class PressureClient {
public:
    PressureClient(muduo::net::EventLoop* loop,
           const muduo::net::InetAddress& serverAddr,
           const uint32_t &id,
           int messageSize );

    void connect();
private: 
    const uint32_t id_;
    void onConnection(const muduo::net::TcpConnectionPtr& conn);
    void onMessage(const muduo::net::TcpConnectionPtr& conn,
                   muduo::net::Buffer* buf,
                   muduo::Timestamp time);

    muduo::net::TcpClient client_;
    int messageSize_;
};

#endif // PRESSURECLIENT_H

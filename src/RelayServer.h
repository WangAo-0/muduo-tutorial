#ifndef RELAY_SERVER_H
#define RELAY_SERVER_H

#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"
#include <cstddef>
#include <map>
#include <deque>


using namespace muduo;
using namespace muduo::net;

class RelayServer
{
public:
    RelayServer(EventLoop* loop, const InetAddress& listenAddr,int numThreads, bool enableHighWaterMark = false);
    void start();
    size_t highWaterMark() const { return highWaterMark_; }
    bool isEnableHighWaterMark() const { return enableHighWaterMark_; }

private:
    static const size_t MAX_BYETES = 1280 * 1024; // 10条128KB的消息
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);
    void onWriteComplete(const TcpConnectionPtr& conn);
    void onHighWaterMarkCallback(const TcpConnectionPtr &conn);
    TcpServer server_;
    // Record all clients.
    std::map<uint32_t, TcpConnectionPtr> clientsMap_;
    std::deque<TcpConnectionPtr> readyToCloseClientsMap_;
    size_t highWaterMark_;
    bool enableHighWaterMark_;
    void enableHighWaterMark() { enableHighWaterMark_ = true; highWaterMark_ = MAX_BYETES ;}
    // void setHighWaterMark(size_t highWaterMark) { highWaterMark_ = highWaterMark; }


    // std::map<uint32_t, std::string> offlineMessages_;
};

#endif // RELAY_SERVER_H

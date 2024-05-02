#ifndef RELAY_SERVER_H
#define RELAY_SERVER_H

#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"
#include <map>
#include <deque>


using namespace muduo;
using namespace muduo::net;

class RelayServer
{
public:
    RelayServer(EventLoop* loop, const InetAddress& listenAddr,int numThreads);
    void start();

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);
    void onWriteComplete(const TcpConnectionPtr& conn);

    TcpServer server_;
    // Record all clients.
    std::map<uint32_t, TcpConnectionPtr> clientsMap_;
    std::deque<TcpConnectionPtr> readyToCloseClientsMap_;
    // std::map<uint32_t, std::string> offlineMessages_;
};

#endif // RELAY_SERVER_H

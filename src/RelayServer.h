#ifndef RELAY_SERVER_H
#define RELAY_SERVER_H

#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"
#include <cstddef>
#include <deque>
#include <map>
#include <mutex>
#include <tbb/concurrent_hash_map.h>

using namespace muduo;
using namespace muduo::net;

class RelayServer {
public:
  RelayServer(EventLoop *loop, const InetAddress &listenAddr, int numThreads,
              bool enableHighWaterMark = false);
  void start();
  size_t highWaterMark() const { return highWaterMark_; }
  bool isEnableHighWaterMark() const { return enableHighWaterMark_; }

private:
  static const size_t MAX_BYETES = 2*1024; // 10条128KB的消息
  void onImmediateForward(const TcpConnectionPtr &conn, Buffer *buf,
                            Timestamp time) ;

  void onConnection(const TcpConnectionPtr &conn);
  void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);
  void onWriteComplete(const TcpConnectionPtr &conn);
  void onHighWaterMarkCallback(const TcpConnectionPtr &conn);
  TcpServer server_;
  muduo::net::EventLoop *loop_;
  // Record all clients.
  // FIXME 应该存储的是唯一的key, conn作为value, 但这要求用户请求的sendId
  // 是唯一的 name , conn server

  tbb::concurrent_hash_map<uint32_t, TcpConnectionPtr> clientsMap_;
  tbb::concurrent_hash_map<std::string, uint32_t> clientsNameMap_;
  // std::map<uint32_t, TcpConnectionPtr> clientsMap_;
  // std::map<uint32_t, uint32_t> clientPairMap_;
  // std::map<std::string ,uint32_t> clientsNameMap_;
  // std::shared_mutex myMutex;
  std::mutex myMutex;
  std::deque<TcpConnectionPtr> readyToCloseClientsMap_;
  size_t highWaterMark_;
  bool enableHighWaterMark_;
  void enableHighWaterMark() {
    enableHighWaterMark_ = true;
    highWaterMark_ = MAX_BYETES;
  }
  int count = 0;
  // void setHighWaterMark(size_t highWaterMark) { highWaterMark_ =
  // highWaterMark; }

  // std::map<uint32_t, std::string> offlineMessages_;
};

#endif // RELAY_SERVER_H

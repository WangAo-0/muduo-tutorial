#ifndef STRESSGENERATOR_H
#define STRESSGENERATOR_H

#include "PressureClient.h"
#include <memory>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <vector>

class StressGenerator {
public:
  StressGenerator(muduo::net::EventLoop *loop,
                  const muduo::net::InetAddress &serverAddr, int sessionCount,
                  int messageSize, int messageCount);
  void connect();

private:
  muduo::net::EventLoop *loop_;
  muduo::net::InetAddress serverAddr_;
  int sessionCount_;
  int messageSize_;
  int messageCount_;
  void onClientClose();
  std::vector<std::unique_ptr<PressureClient>> clients_;
  std::vector<std::unique_ptr<PressureClient>> pendingDestruction_;

  void onLoopIteration();
};

#endif // STRESSGENERATOR_H

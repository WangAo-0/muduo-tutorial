#include "StressGenerator.h"
#include "PressureClient.h"
#include "muduo/base/Logging.h"
#include <functional>
#include <iostream>
// #include "muduo/net/EventLoopThreadPool.h"
// #include "muduo/net/EventLoop.h"

StressGenerator::StressGenerator(muduo::net::EventLoop *loop,
                                 const muduo::net::InetAddress &serverAddr,
                                 int stop,
                                 int sessionCount, int messageSize,
                                 int messageCount, int threadCount)
    : loop_(loop), serverAddr_(serverAddr), sessionCount_(sessionCount),
      messageSize_(messageSize), messageCount_(messageCount),
      threadPool_(new muduo::net::EventLoopThreadPool(loop, "PressureClientPoll")) {
        stop_ = stop==1?true:false;
  threadPool_->setThreadNum(threadCount);

  // clients_.reserve(sessionCount);

  // for (int i = 1; i <= sessionCount; ++i) {
  //   PressureClient *client =
  //       new PressureClient(loop, serverAddr, i, messageSize, messageCount_);
  //   client->setCloseCallback(std::bind(&StressGenerator::onClientClose,
  //   this)); clients_.emplace_back(client);
  // }
  // loop_->runEvery(1.0, std::bind(&StressGenerator::onLoopIteration, this));
}

void StressGenerator::start() {
  // 获取当前started_的值并将其设置为1，然后检查原始值是否为0
  if (started_.getAndSet(1) == 0) {
    threadPool_->start(threadInitCallback_);
    clients_.reserve(sessionCount_);
    for (int i = 1; i <= sessionCount_; ++i) {
      muduo::net::EventLoop *ioLoop = threadPool_->getNextLoop();
      PressureClient *client = new PressureClient(ioLoop, serverAddr_, i,
                                                  messageSize_, messageCount_,stop_);
      client->setCloseCallback(
          std::bind(&StressGenerator::onClientClose, this));
      clients_.emplace_back(client);
      ioLoop->runInLoop(std::bind(&PressureClient::connect, client));
    }
  }
}

// void StressGenerator::onLoopIteration() {
//   // Destroy all disconnected clients
//   for (auto it = clients_.begin(); it != clients_.end();) {
//     if (!(*it)->isConnected() && !(*it)->forceCloseCalled()) {
//       it = clients_.erase(it);
//     } else {
//       ++it;
//     }
//   }
// }

void StressGenerator::onClientClose() {
  --sessionCount_;
  LOG_INFO << "sessionCount_ = " << sessionCount_;
  if (sessionCount_ == 0) {
    std::cout << "All clients are disconnected, stop the stress generator."
              << std::endl;
    // All clients are disconnected, stop the stress generator.
    // loop_->quit();
    loop_->runAfter(1.0, [this] { loop_->quit(); });
    // 或者像baseloop发送一个写事件，监听到可读后，关闭
  }
}

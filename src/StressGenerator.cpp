#include "StressGenerator.h"
#include "muduo/base/Logging.h"
#include <iostream>

StressGenerator::StressGenerator(muduo::net::EventLoop *loop,
                                 const muduo::net::InetAddress &serverAddr,
                                 int sessionCount, int messageSize,
                                 int messageCount, bool nonStop)
    : loop_(loop), serverAddr_(serverAddr), sessionCount_(sessionCount),
      messageSize_(messageSize), messageCount_(messageCount),
      nonStop_(nonStop) {
  clients_.reserve(sessionCount);

  for (int i = 1; i <= sessionCount; ++i) {
    PressureClient *client = new PressureClient(
        loop, serverAddr, i, messageSize, messageCount_, nonStop_);
    client->setCloseCallback(std::bind(&StressGenerator::onClientClose, this));
    clients_.emplace_back(client);
  }
  // loop_->runEvery(1.0, std::bind(&StressGenerator::onLoopIteration, this));
}

void StressGenerator::connect() {
  for (auto &client : clients_) {
    client->connect();
  }
}

void StressGenerator::onLoopIteration() {
  // Destroy all disconnected clients
  for (auto it = clients_.begin(); it != clients_.end();) {
    if (!(*it)->isConnected() && !(*it)->forceCloseCalled()) {
      it = clients_.erase(it);
    } else {
      ++it;
    }
  }
}

void StressGenerator::onClientClose() {
  --sessionCount_;
  LOG_INFO << "sessionCount_ = " << sessionCount_;
  if (sessionCount_ == 0) {
    std::cout << "All clients are disconnected, stop the stress generator."
              << std::endl;
    // All clients are disconnected, stop the stress generator.
    // loop_->quit(); // 这样会导致很多其他组件没有完成析构
    // loop_->runInLoop([this] { loop_->quit(); });// 也不对
    loop_->queueInLoop([this] { loop_->quit(); });
    // loop_->runAfter(0, [this] { loop_->quit(); });
  }
}

#include "StressGenerator.h"

StressGenerator::StressGenerator(muduo::net::EventLoop *loop,
                                 const muduo::net::InetAddress &serverAddr,
                                 int sessionCount, int messageSize)
    : loop_(loop), serverAddr_(serverAddr), sessionCount_(sessionCount),
      messageSize_(messageSize) {
  clients_.reserve(sessionCount);

  for (int i = 1; i <= sessionCount; ++i) {
    clients_.emplace_back(new PressureClient(loop, serverAddr, i, messageSize));
  }
}

void StressGenerator::connect() {
  for (auto &client : clients_) {
    client->connect();
  }
}

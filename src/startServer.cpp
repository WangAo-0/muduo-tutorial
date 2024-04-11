#include "RelayServer.h"
#include "muduo/base/AsyncLogging.h"
#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include <cstdlib> // for std::atoi

int kRollSize = 500 * 1000 * 1000;

std::unique_ptr<muduo::AsyncLogging> g_asyncLog;

void asyncOutput(const char *msg, int len) { g_asyncLog->append(msg, len); }

void setLogging(const char *argv0) {
  muduo::Logger::setOutput(asyncOutput);
  char name[256];
  strncpy(name, argv0, 256);
  g_asyncLog.reset(new muduo::AsyncLogging(::basename(name), kRollSize));
  g_asyncLog->start();
}

int main(int argc, char *argv[]) {
  setLogging(argv[0]);
  int numThreads = 0; 
  int port = 8000;  

  if (argc > 1) {
    port = std::atoi(argv[1]);
  }
  if (argc > 2) {
    numThreads = std::atoi(argv[2]);
  }

  muduo::net::EventLoop loop;
  muduo::net::InetAddress listenAddr(port);

  RelayServer server(&loop, listenAddr, numThreads);
  server.start();

  loop.loop(); // Enter the event loop
  return 0;
}

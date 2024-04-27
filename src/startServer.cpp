#include "RelayServer.h"
#include "muduo/base/AsyncLogging.h"
#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include <cstdlib> // for std::atoi
#include <iostream>
#include <signal.h>

int kRollSize = 500 * 1000 * 1000;

std::unique_ptr<muduo::AsyncLogging> g_asyncLog;
muduo::net::EventLoop *g_loop = nullptr;

void asyncOutput(const char *msg, int len) { g_asyncLog->append(msg, len); }

void setLogging(const char *argv0) {
  muduo::Logger::setOutput(asyncOutput);
  char name[256];
  strncpy(name, argv0, 256);
  g_asyncLog.reset(new muduo::AsyncLogging(::basename(name), kRollSize));
  g_asyncLog->start();
}

void setArgs(int argc, char *argv[], int &port, int &numThreads) {
  if (argc > 1) {
    port = std::atoi(argv[1]);
  }
  if (argc > 2) {
    numThreads = std::atoi(argv[2]);
  }
}

void handle_sigint(int sig) {
  std::cout << "Caught signal " << sig << ", cleaning up and exiting...\n";
  // 关闭资源,这里关闭主循环后，是不是会自动关闭其他资源？
  g_loop->quit();
  // 正常退出
  exit(0);
}

int main(int argc, char *argv[]) {
  signal(SIGINT, handle_sigint);
  if (argc == 2) {
    if (strcmp(argv[1], "-h") == 0) {
      printf("Usage: %s port thread_num\n Or defult args : \n port :8000 "
             "numThreads:0 \n",
             argv[0]);
      return 0;
    }
  }
  setLogging(argv[0]);
  int numThreads = 0;
  int port = 8000;
  setArgs(argc, argv, port, numThreads);

  // muduo::net::EventLoop loop;
  // muduo::net::InetAddress listenAddr(port);
  // RelayServer server(&loop, listenAddr, numThreads);
  // server.start();

  // loop.loop(); // Enter the event loop
  EventLoop loop;
  g_loop = &loop;  // Set the global pointer
  muduo::net::InetAddress listenAddr(port);
  RelayServer server(&loop, listenAddr, numThreads);
  server.start();

  g_loop->loop(); // Enter the event loop
  return 0;
}
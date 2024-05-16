#include "StressGenerator.h"
#include "muduo/base/AsyncLogging.h"
#include "muduo/base/Logging.h"
#include <muduo/base/CurrentThread.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpClient.h>
#include <stdlib.h> // for atoi()
#include <unistd.h> // for getpid()

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

void setArgs(int argc, char *argv[], int &threadNum, int &stop,int &startId,
             int &sessionCount, int &messageSize, int &port, 
             int &messageCount) {
  if (argc > 1) {
    int *args[] = {&threadNum,   &stop, &startId,     &sessionCount,
                   &messageSize, &port, &messageCount};
    int numArgs = sizeof(args) / sizeof(args[0]);

    for (int i = 0; i < numArgs && i < argc - 1; ++i) {
      *args[i] = atoi(argv[i + 1]);
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc == 2) {
    if (strcmp(argv[1], "-h") == 0) {
      printf("Usage: %s [threadNums] [stop(1)] [startId] [session_count] "
             "[message_size] [port]  [message_count]\n"
             "Default Args :0 1 0 100 13 8000 100\n",
             argv[0]);
      return 0;
    }
  }
  setLogging(argv[0]);
  // muduo::Logger::setLogLevel(muduo::Logger::WARN);
  muduo::Logger::setLogLevel(muduo::Logger::INFO);
  int threadNums = 0;
  int stop = 1; // 1: stop, 0: Non-stop
  int startId = 0;
  int sessionCount = 100;
  int messageSize = 13;
  int port = 8000;
  int messageCount = 100;
  setArgs(argc, argv, threadNums, stop, startId, sessionCount, messageSize,
          port, messageCount);

  LOG_INFO << "pid = " << getpid() << ", tid = " << muduo::CurrentThread::tid();
  muduo::net::EventLoop loop;
  muduo::net::InetAddress serverAddr("127.0.0.1", port); // 服务器地址需要修改

  StressGenerator generator(&loop, serverAddr, stop, startId, sessionCount,
                            messageSize, messageCount, threadNums);
  // generator.connect();
  generator.start(); // 这里其实就已经开始工作了
  loop.loop(); // 只起到一个阻塞进程，让出cpu，并且最后关闭的作用
}

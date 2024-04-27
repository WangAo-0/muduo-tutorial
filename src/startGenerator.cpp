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

void setArgs(int argc, char *argv[], int &sessionCount, int &messageSize,
             int &port, int &messageCount) {
  //   if (argc != 5) {
  //     fprintf(stderr, "Usage: %s session_count message_size port\n",
  //     argv[0]); return 1;
  //   }
  if (argc > 1) {
    sessionCount = atoi(argv[1]);
  }

  if (argc > 2) {
    messageSize = atoi(argv[2]);
  }

  if (argc > 3) {
    port = atoi(argv[3]);
  }

  if (argc > 4) {
    messageCount = atoi(argv[4]);
  }
}

int main(int argc, char *argv[]) {
  if(argc == 2){
    if(strcmp(argv[1], "-h") == 0){
      printf("Usage: %s session_count message_size port message_count\n Or default args : \n session_count:100 \n messageSize=13\n port=8000\n message_count=100\n", argv[0]);
      return 0;
    }
  }
  setLogging(argv[0]);
  // muduo::Logger::setLogLevel(muduo::Logger::FATAL);

  int port = 8000;
  int sessionCount = 100; // atoi(argv[1]);
  int messageSize = 13;   // atoi(argv[2]);
  int messageCount = 100; // atoi(argv[3]);

  setArgs(argc, argv, sessionCount, messageSize, port, messageCount);

  LOG_INFO << "pid = " << getpid() << ", tid = " << muduo::CurrentThread::tid();
  muduo::net::EventLoop loop;
  muduo::net::InetAddress serverAddr("127.0.0.1", port); // 服务器地址需要修改

  StressGenerator generator(&loop, serverAddr, sessionCount, messageSize,
                            messageCount);
  generator.connect();

  loop.loop();
}

#include <muduo/base/CurrentThread.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpClient.h>
#include "StressGenerator.h"
#include "muduo/base/Logging.h"
#include <stdlib.h> // for atoi()
#include <unistd.h> // for getpid()
#include "muduo/base/AsyncLogging.h"

int kRollSize = 500*1000*1000;

std::unique_ptr<muduo::AsyncLogging> g_asyncLog;

void asyncOutput(const char* msg, int len)
{
  g_asyncLog->append(msg, len);
}

void setLogging(const char* argv0)
{
  muduo::Logger::setOutput(asyncOutput);
  char name[256];
  strncpy(name, argv0, 256);
  g_asyncLog.reset(new muduo::AsyncLogging(::basename(name), kRollSize));
  g_asyncLog->start();
}

int main(int argc, char *argv[]) {
  setLogging(argv[0]);
  // muduo::Logger::setLogLevel(muduo::Logger::FATAL);

  int port = 8000;
  int sessionCount = 100; // atoi(argv[1]);
  int messageSize = 13;  // atoi(argv[2]);
                         //   if (argc != 4) {
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

  LOG_INFO << "pid = " << getpid() << ", tid = " << muduo::CurrentThread::tid();
  muduo::net::EventLoop loop;
  muduo::net::InetAddress serverAddr("127.0.0.1", port); // 服务器地址需要修改

  StressGenerator generator(&loop, serverAddr, sessionCount, messageSize);
  generator.connect();

  loop.loop();
}

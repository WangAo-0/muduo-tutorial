#ifndef STRESSGENERATOR_H
#define STRESSGENERATOR_H

#include "PressureClient.h"
#include "muduo/base/Atomic.h"
#include "muduo/net/EventLoopThreadPool.h"
#include <memory>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <string>
#include <vector>

class StressGenerator {
public:
  typedef std::function<void(muduo::net::EventLoop *)> ThreadInitCallback;

  StressGenerator(muduo::net::EventLoop *loop,
                  const muduo::net::InetAddress &serverAddr,int stop,int startId, int sessionCount,
                  int messageSize, int messageCount, int threadCount = 0);
  muduo::net::EventLoop *getLoop() const { return loop_; }

  /// Set the number of threads for handling input.
  ///
  /// Always accepts new connection in loop's thread.
  /// Must be called before @c start
  /// @param numThreads
  /// - 0 means all I/O in loop's thread, no thread will created.
  ///   this is the default value.
  /// - 1 means all I/O in another thread.
  /// - N means a thread pool with N threads, new connections
  ///   are assigned on a round-robin basis.
  // void setThreadNum(int numThreads);

  void setThreadInitCallback(const ThreadInitCallback &cb) {
    threadInitCallback_ = cb;
  }
  /// valid after calling start()
  std::shared_ptr<muduo::net::EventLoopThreadPool> threadPool() {
    return threadPool_;
  }
  void start();
  int startId_;

private:
  muduo::net::EventLoop *loop_;
  muduo::net::InetAddress serverAddr_;
  int sessionCount_;
  int messageSize_;
  int messageCount_;
  void onClientClose();
  std::shared_ptr<muduo::net::EventLoopThreadPool> threadPool_;
  muduo::AtomicInt32 started_;
  ThreadInitCallback threadInitCallback_;

  std::vector<std::unique_ptr<PressureClient>> clients_;
  std::vector<std::unique_ptr<PressureClient>> pendingDestruction_;
  bool stop_;
  void onLoopIteration();
};

#endif // STRESSGENERATOR_H

// #include <muduo/net/EventLoop.h>
#ifndef IOEVENTLOOP_H
#define IOEVENTLOOP_H
#include <liburing.h>
// #include <muduo/net/Poller.h>
#include "muduo/net/Channel.h"
#include <muduo/net/Poller.h>
#include <sys/eventfd.h>
class IOEventLoop : public muduo::net::EventLoop {
public:
  IOEventLoop();
  void handleRead();
  struct io_uring_sqe *getASQE();
  struct io_uring &iouring() { return iouring_; };

  ~IOEventLoop();

private:
  std::unique_ptr<muduo::net::Poller> poller_;
  struct io_uring iouring_;
  int eventFd_;
  std::unique_ptr<muduo::net::Channel> channel_;
  int test_count;
};

#endif
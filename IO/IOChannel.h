#ifndef IOCHANNEL_H
#define IOCHANNEL_H

#include <functional>

class IOChannel {
public:
  IOChannel(){};
  typedef std::function<void(struct io_uring_cqe *cqe)> EventCallback;
  void setReadCompleteCallback(EventCallback cb);
  void setWriteCompleteCallback(EventCallback cb);
  void setCallback(EventCallback cb);
  void handleEvent(struct io_uring_cqe *cqe);
  ~IOChannel(){};

private:
  EventCallback readCompleteCallback_;
  EventCallback writeCompleteCallback_;
  EventCallback callback_;
};
#endif
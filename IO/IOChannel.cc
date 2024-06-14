#include "IOChannel.h"
#include <iostream>
void IOChannel::setReadCompleteCallback(EventCallback cb) {
  readCompleteCallback_ = cb;
}
void IOChannel::setWriteCompleteCallback(EventCallback cb) {
  writeCompleteCallback_ = cb;
}
void IOChannel::setCallback(EventCallback cb) { callback_ = cb; }

void IOChannel::handleEvent(struct io_uring_cqe *cqe) {
  if (callback_) {
    callback_(cqe);
  } else {
    std::cout << "IOChannel::handleEvent" << std::endl;
  }
}
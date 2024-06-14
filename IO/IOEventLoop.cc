#include "IOEventLoop.h" // 在IOEventLoop类中
// #include "File.h"
#include "IOChannel.h"
#include "muduo/base/Logging.h"
#include <functional>

int createEventfd() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG_SYSERR << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

struct io_uring_sqe *IOEventLoop::getASQE() {
  return io_uring_get_sqe(&iouring_);
}

IOEventLoop::IOEventLoop()
    : poller_(muduo::net::Poller::newDefaultPoller(this)),
      eventFd_(createEventfd()),
      channel_(new muduo::net::Channel(this, eventFd_)) {
  if (io_uring_queue_init(256, &iouring_, 0) < 0) {
    throw std::runtime_error("Failed to initialize io_uring");
  }
  io_uring_register_eventfd(&iouring_, eventFd_);
  channel_->setReadCallback(std::bind(&IOEventLoop::handleRead, this));
  channel_->enableReading();
}

IOEventLoop::~IOEventLoop() {
  io_uring_queue_exit(&iouring_);
  channel_->disableAll();
  channel_->remove();
  ::close(eventFd_);
}

/**
 * @brief io_uring完成了IO事件，即当 io_uring 中的 I/O 操作完成时，
 * 内核会自动向这个关联的 eventfd 写入一个特定的值（通常是1），
 * 以此作为通知机制，告知用户空间的应用程序有 I/O 事件已经完成。
 *
 * 当检测到eventfd有事件时，你可以调用io_uring_cq_ready或io_uring_peek_cqe等函数
 * 来获取完成队列中的Completion Queue Entry
 * (CQE)。这个CQE包含了I/O操作的结果（如读写的字节数、错误码等），
 * 以及通过io_uring_sqe_set_data设置的用户数据指针。
 *
 * 手动“回调”：在获取到CQE后，你可以根据用户数据指针访问之前关联的对象或上下文，
 * 并在此基础上实现自己的逻辑处理，这等效于回调函数的执行。
 * 根据传入的cqe和arg（即之前设置的用户数据指针）来判断并处理I/O完成的情况。
 *
 */
void IOEventLoop::handleRead() {
  // 当eventfd可读时，说明有I/O操作完成
  // 从iouring的完成队列中获取完成的I/O操作，并调用相应的回调函数
  // iouring_.getCompletedEvents();

  // 获取IO完成的任务类型
  struct io_uring_cqe *cqe;
  while ((io_uring_peek_cqe(&iouring_, &cqe)) >= 0) {
    // IoUringContext *ctx =
    //     static_cast<IoUringContext *>(io_uring_cqe_get_data(cqe));
    // auto file = static_cast<File *>(io_uring_cqe_get_data(cqe));
    auto ioChannel = static_cast<IOChannel *>(io_uring_cqe_get_data(cqe));
    // ctx->callback(cqe);
    ioChannel->handleEvent(cqe);

    io_uring_cqe_seen(&iouring_, cqe);
    test_count++;
    if (test_count == 2) {
      // LOG_INFO << "test_count==2";
      quit();
      // queueInLoop([this] { this->quit(); });
    }
  }
}
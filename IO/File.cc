#include "File.h"
#include "IOChannel.h"

#include <fcntl.h>
#include <functional>
#include <muduo/base/Logging.h>

File::File(IOEventLoop *loop, const std::string &filename)
    : loop_(loop), filename_(filename), readBuffer_(1024),
      readChannel_(new IOChannel()), writeChannel_(new IOChannel()) {

  readChannel_->setCallback(
      std::bind(&File::handleRead, this, std::placeholders::_1));
  writeChannel_->setCallback(
      std::bind(&File::handleWrite, this, std::placeholders::_1));
}

File::~File() {
  if (fd_ >= 0) {
    close(fd_);
  }
}

void File::registerFile() {
  fd_ = open(filename_.c_str(), O_RDWR | O_CREAT, 0666);
  if (fd_ < 0) {
    perror("open");
    return;
  }
  io_uring_register_files(&loop_->iouring(), &fd_, 1);
}
void File::readFile(unsigned lens, __u64 offset) {
  // 使用io_uring预提交一个读取操作
  struct io_uring_sqe *sqe = loop_->getASQE();
  if (!sqe) {
    // 错误处理：队列已满或其它错误
    LOG_ERROR << "readFile : Failed to get SQE";
    return;
  }

  // 假设我们有一个缓冲区用于读取数据
  // io_uring_prep_read_fixed(sqe, 0, readBuffer_.data(), lens, offset, 0);

  io_uring_prep_read(sqe, 0, readBuffer_.data(), lens, offset);

  // 设置sqe完成时的回调函数
  uint32_t flags = IOSQE_FIXED_FILE | IOSQE_ASYNC;

  io_uring_sqe_set_data(sqe, readChannel_.get());
  io_uring_sqe_set_flags(sqe, flags);

  // 提交到io_uring
  io_uring_submit(&loop_->iouring());
}

void File::handleRead(struct io_uring_cqe *cqe) {
  // 读取操作完成，处理结果
  if (cqe->res < 0) {
    int error_code = -cqe->res; // 获取错误码
    LOG_ERROR << "Failed to read file: " << filename_
              << ", Error: " << strerror(error_code);
  } else if (cqe->res == 0) {
    LOG_INFO << "EOF reached";
  } else if (cqe->res > 0) {
    // 读取成功，处理数据
    // LOG_INFO << "Read " << cqe->res << " bytes from file: " << filename_
    //          << " data: " << readBuffer_.data();
  }
}

void File::handleWrite(struct io_uring_cqe *cqe) {
  // 写入操作完成，处理结果
  if (cqe->res < 0) {
    // 错误处理
    LOG_ERROR << "Failed to write file: " << filename_;
  } else {
    // 写入成功，处理结果
    // LOG_INFO << "Wrote " << cqe->res << " bytes to file: " << filename_;
  }
}

void File::writeFile(const std::string &data) {
  // 提交写入操作给iouring，并设置完成事件
  // 准备写入的数据缓冲区
  // 注意：转换const，因为io_uring接口期望非const指针
  // struct iovec iov = {.iov_base = const_cast<char *>(data.data()),
  //                     .iov_len = data.size()};

  // 准备提交队列条目（SQE）
  uint32_t flags = IOSQE_FIXED_FILE | IOSQE_ASYNC;
  struct io_uring_sqe *sqe = io_uring_get_sqe(&loop_->iouring());
  if (!sqe) {
    LOG_ERROR << "writeFile : Failed to get SQE";
    return;
  }
  // writeBuffer_.append(data);
  // writeBuffer_.writableBytes();
  io_uring_prep_write(sqe, 0, const_cast<char *>(data.data()), data.size(), 0);
  io_uring_sqe_set_data(sqe, writeChannel_.get());

  io_uring_sqe_set_flags(sqe, flags);

  // 提交写操作到io_uring
  if (io_uring_submit(&loop_->iouring()) < 0) {
    int error_code =
        -errno; // 获取错误码，注意这里的errno是全局变量，保存了最近一次系统调用的错误码
    LOG_ERROR << "writeFile: Failed to submit write operation, error code: "
              << error_code << ", error message: " << strerror(error_code);
  }
}

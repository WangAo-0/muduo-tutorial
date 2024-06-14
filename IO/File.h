#ifndef FILE_H
#define FILE_H
#include "IOChannel.h"
#include "IOEventLoop.h"
#include "muduo/net/Buffer.h"

class File {
public:
  File(IOEventLoop *loop, const std::string &filename);
  void registerFile();
  void readFile(unsigned nbytes, __u64 offset);
  void writeFile(const std::string &data);
  ~File();

  void handleWrite(struct io_uring_cqe *cqe);
  void handleRead(struct io_uring_cqe *cqe);
  std::vector<char> readBuffer_;
  muduo::net::Buffer writeBuffer_;

private:
  IOEventLoop *loop_;
  int fd_;
  const std::string filename_;
  std::shared_ptr<IOChannel> readChannel_;
  std::shared_ptr<IOChannel> writeChannel_;
};


#endif
#ifndef STRESSGENERATOR_H
#define STRESSGENERATOR_H

#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include "PressureClient.h"
#include <vector>
#include <memory>

class StressGenerator {
public:
    StressGenerator(muduo::net::EventLoop *loop,
                    const muduo::net::InetAddress &serverAddr,
                    int sessionCount, int messageSize);

    void connect();

private:
    muduo::net::EventLoop *loop_;
    muduo::net::InetAddress serverAddr_;
    int sessionCount_;
    int messageSize_;
    std::vector<std::unique_ptr<PressureClient>> clients_;
};

#endif // STRESSGENERATOR_H

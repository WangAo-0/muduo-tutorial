Examples of Muduo network library

0) src             - source file
1) bazel           - build with Bazel
2) cmake-submodule - build with CMake, checking out muduo as a git submodule
3) cmake           - build with CMake
2) makefile        - build with Makefile

1) build with bazel
cd bazel
bazel build -c opt :all

2) build with CMake with git submodule
git submodule update --init
mkdir build
cd build
cmake ../cmake-submodule
make

Assuming Muduo is installed in $HOME/build/debug-install

3) build with CMake
mkdir build
cd build
cmake ../cmake
make
# echo binary is in ./bin/

4) build with GNU make
cd makefile
make
# echo binary is in ./

### 中继服务器 版本1.0 
#### 特性
- 客户端发送连接请求，中继服务器acceptor可读事件触发建立连接后 ，发送一个欢迎信息给客户端
- 服务器建立连接后，客户端connector可写事件触发，发送消息告知服务器自己的ID。这里服务器还没发欢迎信息时，客户端就已经触发了connector可写事件
- 这里服务器发送欢迎信息和客户端发送ID顺序是不确定的，但是基本上差不多同时发送
- 客户端接收到欢迎信息后，奇数ID客户端先发送消息，另一个客户端接收到消息后再发送一个消息，如此循环，客户端完成一定量消息发送后就关闭写端，
- server转发时目标客户端不在线，暂存在内存的buffer中



### 中继服务器 版本2.0
#### 目标  
 1. 压力发生器中客户端随机发送数据，不停等
 2. 转发时目标不在的话，当前是缓存在buffer，最好能实现定时写入文件
 3. 两个压力发生器：
    - 第一个创建稳定的2000会话
    - 第二个创建第一定数据随机的会话（比如发送4，5个消息后退出）
 3. 如何优雅的退出

#### 实现
 1. 

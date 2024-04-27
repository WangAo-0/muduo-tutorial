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

中继服务器 版本1.0 

- 客户端发送连接请求，中继服务器acceptor可读事件触发建立连接后 ，发送一个欢迎信息给客户端
- 服务器建立连接后，客户端connector可写事件触发，发送消息告知服务器自己的ID。这里服务器还没发欢迎信息时，客户端就已经触发了connector可写事件
- 这里服务器发送欢迎信息和客户端发送ID顺序是不确定的，但是基本上差不多同时发送
- 客户端接收到欢迎信息后，奇数ID客户端先发送消息，另一个客户端接收到消息后再发送一个消息，如此循环

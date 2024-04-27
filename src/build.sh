#!/bin/bash

# 指定头文件和库文件的路径
INCLUDE_PATH=/home/oliver/muduo-tutorial/cmake-submodule/build/release-install-cpp11/include
LIB_PATH=/home/oliver/muduo-tutorial/cmake-submodule/build/release-install-cpp11/lib

# 指定源文件
generator_src_files=("startGenerator.cpp" "StressGenerator.cpp" "PressureClient.cpp")
server_src_files=("startServer.cpp" "RelayServer.cpp")

# 使用g++编译源文件并链接到muduo库
g++ ${generator_src_files[@]} -I$INCLUDE_PATH -L$LIB_PATH -lmuduo_base -lmuduo_net -lmuduo_http -o startGenerator
g++ ${server_src_files[@]} -I$INCLUDE_PATH -L$LIB_PATH -lmuduo_base -lmuduo_net -lmuduo_http -o startServer

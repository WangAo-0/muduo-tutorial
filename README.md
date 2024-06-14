# 基于io_uring的文件读写

### 文件夹结构说明
```
├── IO  # 使用io_uring扩展的类
│   ├── File.cc
│   ├── File.h
│   ├── IOChannel.cc
│   ├── IOChannel.h
│   ├── IOEventLoop.cc
│   └── IOEventLoop.h
├── docs # 作业要求
├── README.md # 项目说明
├── cmake-submodule # muduo子模块
│   ├── CMakeLists.txt # gtest测试的构建
│   ├── build # muduo编译完成的文件
│   └── muduo # muduo源码
└── gtest # gtest测试
    └── test.cc
``` 
### 项目构建说明
指定CMake源目录：./cmake-submodule,用该目录下的CMakeLists.txt构建项目（会顺序构建以下两个模块）
1. 构建muduo子模块
2. 构建gtest测试模块

cmake_minimum_required(VERSION 3.0)

project(iouring_gtest CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()
# tests/CMakeLists.txt
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})


add_subdirectory(muduo)

# Set the common libraries.
link_libraries(muduo_net muduo_base pthread rt uring)

# Set the source files for startGenerator.
set(iouring_src
 ../IO/File.cc
 ../IO/IOEventLoop.cc
 ../gtest/test.cc
 ../IO/IOChannel.cc
)
add_executable(iotest ${iouring_src})
target_include_directories(iotest PRIVATE muduo uring)
target_link_libraries(iotest ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
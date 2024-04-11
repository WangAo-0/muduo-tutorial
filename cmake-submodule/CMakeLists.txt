cmake_minimum_required(VERSION 3.0)

project(Relay CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

add_subdirectory(muduo)

# Set the common libraries.
link_libraries(muduo_net muduo_base pthread rt)

# Set the source files for startGenerator.
set(START_GENERATOR_SRC
  ../src/startGenerator.cpp 
  ../src/StressGenerator.cpp 
  ../src/PressureClient.cpp
)
add_executable(startGenerator ${START_GENERATOR_SRC})
target_include_directories(startGenerator PRIVATE muduo)

# Set the source files for startServer.
set(START_SERVER_SRC
  ../src/startServer.cpp 
  ../src/RelayServer.cpp 
)
add_executable(startServer ${START_SERVER_SRC})
target_include_directories(startServer PRIVATE muduo)

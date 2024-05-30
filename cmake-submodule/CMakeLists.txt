cmake_minimum_required(VERSION 3.0)

project(Relay CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")


add_subdirectory(muduo)

# Set the common libraries.
link_libraries(muduo_net muduo_base pthread rt)

find_package(TBB REQUIRED)
# Set the source files for startGenerator.
set(START_GENERATOR_SRC
  ../src/startGenerator.cpp 
  ../src/StressGenerator.cpp 
  ../src/PressureClient.cpp
)
SET(CMAKE CXX FLAGS "${CMAKE_CXX_FLAGS} -pg")
add_executable(startGenerator ${START_GENERATOR_SRC})
target_compile_options(startGenerator PRIVATE -pg) # Add -pg option here

target_include_directories(startGenerator PRIVATE muduo)
target_link_libraries(startGenerator TBB::tbb)


# Set the source files for startServer.
set(START_SERVER_SRC
  ../src/startServer.cpp 
  ../src/RelayServer.cpp 
)
SET(CMAKE CXX FLAGS "${CMAKE_CXX_FLAGS} -pg")

add_executable(startServer ${START_SERVER_SRC})
target_compile_options(startServer PRIVATE -pg) # Add -pg option here
target_include_directories(startServer PRIVATE muduo)
target_link_libraries(startServer TBB::tbb)


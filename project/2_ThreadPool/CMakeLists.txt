cmake_minimum_required(VERSION 3.13)

# 工程名称
project(ThreadPool VERSION 1.0)

# 设置
set(CMAKE_CXX_STANDARD 11)

# 添加头文件
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

# 添加可执行文件
add_executable(threadPool  ./threadPool.cpp)



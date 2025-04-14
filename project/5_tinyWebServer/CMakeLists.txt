cmake_minimum_required(VERSION 3.13)

# 工程名称
project(tinyWebServer VERSION 1.0)

# 设置
set(CMAKE_CXX_STANDARD 11)

# 添加头文件
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

# 部分头文件的实现
file(GLOB_RECURSE SRC_CPP_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cppcode/*.cpp")

# 设置生成的可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# 添加可执行文件
add_executable(main  ./main.cpp ${SRC_CPP_FILE})

# 连接
target_link_libraries(main PUBLIC pthread mysqlclient)


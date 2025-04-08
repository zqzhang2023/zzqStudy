cmake_minimum_required(VERSION 3.13)

# 工程名称
project(tinyhttpd VERSION 1.0)

# 设置
set(CMAKE_CXX_STANDARD 11)


# 添加可执行文件
add_executable(httpd  ./httpd.c)

# 连接
target_link_libraries(httpd PUBLIC pthread)


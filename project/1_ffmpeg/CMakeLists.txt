cmake_minimum_required(VERSION 3.13)

# 工程名称
project(videotranscode VERSION 1.0)

# 设置
set(CMAKE_CXX_STANDARD 11)

# 添加头文件
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/include")

# 部分头文件的实现
file(GLOB_RECURSE SRC_CPP_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# 库的位置
set(LIB_PATH_ZZQ "${CMAKE_CURRENT_SOURCE_DIR}/lib/libffmpeg-zzq.so")

# 添加可执行文件
add_executable(transcode  ./transcode.cpp ${SRC_CPP_FILE})

# 连接
target_link_libraries(transcode PUBLIC ${LIB_PATH_ZZQ})


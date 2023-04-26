cmake_minimum_required(VERSION 3.10.0)

project(mymuduo)

# mymuduo最终编译成so动态库，设置动态库路径，放在根目录的lib下
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置调试信息 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC")
include_directories(./src)
# 定义参与编译的源代码文件
aux_source_directory(./src/tools SRC_LIST)
aux_source_directory(./src/net SRC_LIST)
# 编译生成动态库
add_library(mymuduo SHARED ${SRC_LIST})


# 脱离boost库 采取c++11标准
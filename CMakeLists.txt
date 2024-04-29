cmake_minimum_required(VERSION 3.15)
project(STL)

# 设置 C++ 标准为 C++11
set(CMAKE_CXX_STANDARD 11)

# 设置可执行文件输出的目录
set(HOME D:/workspace/STL)
set(EXECUTABLE_OUTPUT_PATH ${HOME}/bin)
set(SRC ${HOME}/src)
set(TEST ${HOME}/test)
# 添加头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 搜索 src 目录下的源文件
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_LIST)

# 创建可执行文件，名称为 test，源文件由 SRC_LIST 变量提供
add_executable(memory_test ${TEST}/memory_test.cpp)

# 另一种搜索源文件的方式，将搜索到的所有 .cpp 文件赋值给 SRC_LIST 变量
# file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
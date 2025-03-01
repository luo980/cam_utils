# CMakeLists.txt：放在工程根目录下

cmake_minimum_required(VERSION 3.10)
project(cam_utils)

# 设置 C++ 标准为 C++11（可根据需要修改）
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 指定第三方头文件目录，不对第三方源码做任何修改
# 使得 src 中文件可以 #include "httplib.h" 或 "httplib/xxx.h"
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/httplib)

add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/sdl EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/yaml-cpp)

# 添加源文件目录下的摄像头程序实现
add_executable(cam_utils src/cam_sdl3_httpd.cpp)

configure_file(${CMAKE_SOURCE_DIR}/config/config.yaml ${CMAKE_BINARY_DIR}/config.yaml COPYONLY)

target_link_libraries(cam_utils PRIVATE pthread SDL3::SDL3 yaml-cpp)

# 其他库请按需要继续添加，例如 OpenSSL 或其他依赖

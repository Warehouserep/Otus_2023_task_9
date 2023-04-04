cmake_minimum_required(VERSION 3.9)
project(async)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -pthread)

add_library(async SHARED context.h context.cpp async.cpp async.h)

add_executable(async_cli main.cpp)

target_link_libraries(async_cli async)
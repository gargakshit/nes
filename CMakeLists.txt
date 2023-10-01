cmake_minimum_required(VERSION 3.26)
project(nes)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(nes ${SOURCES})

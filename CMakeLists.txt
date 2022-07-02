cmake_minimum_required(VERSION 3.16)
project(RISC_V_tomasulo)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

add_executable(code main.cpp Decoder.hpp CPU.cpp unit.cpp ansi.cpp)

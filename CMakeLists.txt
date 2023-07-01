cmake_minimum_required(VERSION 3.16)
project(RISC-V-SIMU)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)

add_executable(code
        main.cpp)

cmake_minimum_required(VERSION 3.16)
project(RISC-V-SIMU)

set(CMAKE_CXX_STANDARD 14)

add_executable(code
        main.cpp src/load_store_buffer.hpp src/parser.hpp src/program.hpp src/reorder_buffer.hpp src/reservation_station.hpp src/system.hpp src/utils.hpp)

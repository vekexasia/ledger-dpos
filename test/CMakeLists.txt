cmake_minimum_required(VERSION 3.17)
project(test C)

set(CMAKE_C_STANDARD 11)

add_executable(test
        ../src/lisk/lisk32_address.c
        ../src/lisk/lisk32_address.h
        test.c)

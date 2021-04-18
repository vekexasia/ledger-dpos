cmake_minimum_required(VERSION 3.17)
project(test C)

set(CMAKE_C_STANDARD 11)

add_executable(test
        ../src/lisk/lisk_address_lisk32.c
        ../src/lisk/lisk_address_lisk32.h
        test.c)

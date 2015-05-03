cmake_minimum_required(VERSION 3.2)
project(PiSwitch)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

set(SOURCE_FILES main.c)
add_executable(PiSwitch ${SOURCE_FILES})
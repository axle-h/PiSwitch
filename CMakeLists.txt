cmake_minimum_required(VERSION 3.2)
project(PiSwitch)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")


find_package(BCM2835 REQUIRED)

include_directories(include ${BCM2835_INCLUDE_DIR})



set(SOURCE_FILES main.c)

add_executable(PiSwitch ${SOURCE_FILES})

target_link_libraries(PiSwitch ${BCM2835_LIBRARIES})
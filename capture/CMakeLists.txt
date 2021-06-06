cmake_minimum_required(VERSION 3.20)
project(capture C)

set(CMAKE_C_STANDARD 11)

add_executable(capture-01
        01/main.c)

target_link_libraries(capture-01 pcap)

add_executable(capture-02
        02/main.c)

target_link_libraries(capture-02 pcap)
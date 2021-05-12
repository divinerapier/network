cmake_minimum_required(VERSION 3.20)
project(thundering_herd C)

set(CMAKE_C_STANDARD 11)

add_executable(accpet
        accept/main.c)

add_executable(epoll
        epoll/main.c)

add_executable(epoll2
        epoll2/main.c)

add_executable(epoll3
        epoll3/main.c)

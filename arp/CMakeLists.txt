cmake_minimum_required(VERSION 3.20)
project(arp C)

set(CMAKE_C_STANDARD 11)

add_executable(arp-request
        arp-request/main.c)

add_executable(arp-reply
        arp-reply/main.c)

#target_link_libraries(arp-request pcap)

cmake_minimum_required(VERSION 3.28)
project(WorkShopCN C)

set(CMAKE_C_STANDARD 11)

add_executable(WorkShopCN main.c
        server.c
        client.c)

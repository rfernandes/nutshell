cmake_minimum_required(VERSION 2.6)
project(nutshell)

add_executable(nutshell main.cpp)

install(TARGETS nutshell RUNTIME DESTINATION bin)

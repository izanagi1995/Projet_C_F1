cmake_minimum_required(VERSION 3.6)
project(Projet_C_F1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.c)
add_executable(Projet_C_F1 ${SOURCE_FILES})
cmake_minimum_required(VERSION 3.13)
project(Scheme C)

set(CMAKE_C_STANDARD 99)

add_executable(Scheme main.c src/header/object.h src/object.c src/header/error.h src/error.c src/header/read.h)
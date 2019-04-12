cmake_minimum_required(VERSION 3.10)
project(Scheme C)

set(CMAKE_C_STANDARD 99)

add_executable(Toy-Scheme main.c src/header/object.h src/object.c src/header/error.h src/error.c src/header/read.h src/read.c src/header/eval.h src/eval.c src/header/environment.h src/environment.c src/header/apply.h src/apply.c src/header/builtin.h src/builtin.c src/header/write.h src/write.c)
cmake_minimum_required(VERSION 3.10)
project(Scheme C)

set(CMAKE_C_STANDARD 99)

set(TOY_SCHEME_SOURCES
    main.c
    src/object.c
    src/error.c
    src/read.c
    src/eval.c
    src/environment.c
    src/apply.c
    src/builtin.c
    src/write.c
)

add_executable(Toy-Scheme ${TOY_SCHEME_SOURCES})

install(TARGETS Toy-Scheme
        RUNTIME DESTINATION bin)

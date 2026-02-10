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

find_path(READLINE_INCLUDE_DIR readline/readline.h)
find_library(READLINE_LIBRARY NAMES readline edit)
find_library(TERMCAP_LIBRARY NAMES ncurses curses termcap tinfo)

if(READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
    target_include_directories(Toy-Scheme PRIVATE ${READLINE_INCLUDE_DIR})
    target_link_libraries(Toy-Scheme PRIVATE ${READLINE_LIBRARY})
    if(TERMCAP_LIBRARY)
        target_link_libraries(Toy-Scheme PRIVATE ${TERMCAP_LIBRARY})
    endif()
    target_compile_definitions(Toy-Scheme PRIVATE HAVE_READLINE=1)
    message(STATUS "REPL line editing enabled with ${READLINE_LIBRARY}")
else()
    message(STATUS "readline/libedit not found; REPL falls back to raw input")
endif()

install(TARGETS Toy-Scheme
        RUNTIME DESTINATION bin)

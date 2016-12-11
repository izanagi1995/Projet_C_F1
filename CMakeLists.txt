cmake_minimum_required(VERSION 3.6)
project(Projet_C_F1)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
    defs.h
    func.c
    func.h
    curses-utils.c
    curses-utils.h
    main.c)

add_executable(Projet_C_F1 ${SOURCE_FILES})
target_link_libraries(Projet_C_F1 pthread ncurses)
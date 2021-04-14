cmake_minimum_required(VERSION 3.5.1)
project(sc_perf C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

    add_library(
            sc_perf SHARED
            sc_perf.c
            sc_perf.h)

    target_include_directories(sc_perf PUBLIC ${CMAKE_CURRENT_LIST_DIR})

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -pedantic -Werror -Wextra -Wall")
endif ()

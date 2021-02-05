cmake_minimum_required(VERSION 3.5.1)
project(sc_perf C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_executable(sc_perf perf_example.c sc_perf.h sc_perf.c)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -pedantic -Werror")
endif ()


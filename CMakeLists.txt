cmake_minimum_required(VERSION 3.5.1)
project(sc_lib C)
include(CTest)
include(CheckCCompilerFlag)

enable_testing()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif ()

message(STATUS "Build type ${CMAKE_BUILD_TYPE}")

add_subdirectory(array)
add_subdirectory(buffer)
add_subdirectory(condition)
add_subdirectory(crc32)
add_subdirectory(heap)
add_subdirectory(ini)
add_subdirectory(linked-list)
add_subdirectory(logger)
add_subdirectory(map)
add_subdirectory(memory-map)
add_subdirectory(mutex)
add_subdirectory(option)
add_subdirectory(queue)
add_subdirectory(perf)
add_subdirectory(sc)
add_subdirectory(signal)
add_subdirectory(socket)
add_subdirectory(string)
add_subdirectory(time)
add_subdirectory(timer)
add_subdirectory(thread)
add_subdirectory(uri)

# --------------------------------------------------------------------------- #
# --------------------- Test Configuration Start ---------------------------- #
# --------------------------------------------------------------------------- #

# ----------------------- - Code Coverage Start ----------------------------- #

if (${CMAKE_BUILD_TYPE} MATCHES "Coverage")
    if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options(--coverage)
        link_libraries(gcov)
    else()
        message(FATAL_ERROR "Only GCC is supported for coverage")
    endif()
endif ()

add_custom_target(coverage)
add_custom_command(
        TARGET coverage
        COMMAND lcov --capture --directory .
        --output-file coverage.info --rc lcov_branch_coverage=1 --rc lcov_excl_br_line='assert'
        COMMAND lcov --remove coverage.info '/usr/*' '*example*' '*test*'
        --output-file coverage.info --rc lcov_branch_coverage=1 --rc lcov_excl_br_line='assert'
        COMMAND lcov --list coverage.info --rc lcov_branch_coverage=1 --rc lcov_excl_br_line='assert'
)

# -------------------------- Code Coverage End ------------------------------ #

SET(MEMORYCHECK_COMMAND_OPTIONS
        "-q --log-fd=2 --trace-children=yes --track-origins=yes       \
         --leak-check=full --show-leak-kinds=all \
         --error-exitcode=255")

add_custom_target(valgrind ${CMAKE_COMMAND}
        -E env CTEST_OUTPUT_ON_FAILURE=1
        ${CMAKE_CTEST_COMMAND} -C $<CONFIG>
        --overwrite MemoryCheckCommandOptions=${MEMORYCHECK_COMMAND_OPTIONS}
        --verbose -T memcheck WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_custom_target(check ${CMAKE_COMMAND}
        -E env CTEST_OUTPUT_ON_FAILURE=1
        ${CMAKE_CTEST_COMMAND} -C $<CONFIG> --verbose
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_dependencies(coverage check)

# ----------------------- Test Configuration End ---------------------------- #

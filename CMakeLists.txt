cmake_minimum_required(VERSION 3.5.1)
project(sc_lib C)
include(CTest)
include(CheckCCompilerFlag)

enable_testing()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif ()

message(STATUS "Build type ${CMAKE_BUILD_TYPE}")

option(SC_BUILD_TEST "Build tests" ON)
if (NOT SC_BUILD_TEST)
    message(STATUS "Turned off building tests because SC is a subproject.")
    message(STATUS "Turned off coverage because SC is a subproject.")
endif ()

if (NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug" AND NOT ${CMAKE_BUILD_TYPE} MATCHES "Coverage")
    SET(SC_BUILD_TEST OFF CACHE BOOL "Turn off sc_lib tests" FORCE)
    message(STATUS "Not building tests, SC tests are supported in Debug build only.")
endif ()

option(SC_USE_WRAP "Use --wrap to test libc function failures" ON)
if (NOT SC_USE_WRAP)
    message(STATUS "Turned off --wrap.")
endif()

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
if (SC_BUILD_TEST)
    # ----------------------- - Code Coverage Start ----------------------------- #

    if (${CMAKE_BUILD_TYPE} MATCHES "Coverage")
        if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
            add_compile_options(--coverage)
            link_libraries(gcov)
        else ()
            message(FATAL_ERROR "Only GCC is supported for coverage")
        endif ()
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

endif ()
# ----------------------- Test Configuration End ---------------------------- #

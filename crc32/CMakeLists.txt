cmake_minimum_required(VERSION 3.5.1)
project(sc_crc32 C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(
        sc_crc32 SHARED
        sc_crc32.c
        sc_crc32.h)

target_include_directories(sc_crc32 PUBLIC ${CMAKE_CURRENT_LIST_DIR})

if (NOT CMAKE_C_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra -pedantic -Werror")
endif ()


# --------------------------------------------------------------------------- #
# --------------------- Test Configuration Start ---------------------------- #
# --------------------------------------------------------------------------- #
if (SC_BUILD_TEST)

    include(CTest)
    include(CheckCCompilerFlag)
    include(TestBigEndian)

    if (SC_CLANG_TIDY)
        message(STATUS "Enabled CLANG_TIDY")

        set(CMAKE_C_CLANG_TIDY
                clang-tidy;
                -line-filter=[{"name":"${PROJECT_NAME}.h"},{"name":"${PROJECT_NAME}.c"}];
                -checks=clang-analyzer-*,misc-*,portability-*,bugprone-*,-bugprone-implicit-widening-of-multiplication-result*;
                -warnings-as-errors=clang-analyzer-*,misc-*,portability-*,bugprone-*;)
    endif ()

    enable_testing()

    add_executable(${PROJECT_NAME}_test crc32_test.c sc_crc32.c)

    # detect x86
    check_c_compiler_flag(-msse4.2 HAVE_CRC32_HARDWARE)
    if (${HAVE_CRC32_HARDWARE})
        if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
            message(STATUS "CPU have -msse4.2, defined HAVE_CRC32C")
            target_compile_options(${PROJECT_NAME}_test PRIVATE -msse4.2)
            target_compile_definitions(${PROJECT_NAME}_test PRIVATE -DHAVE_CRC32C)
        endif ()
    endif ()

    # detect aarch64
    if (CMAKE_SYSTEM_PROCESSOR STREQUAL aarch64)
        message(STATUS "CPU = aarch64, defined HAVE_CRC32C, -march=armv8.1-a")
        target_compile_definitions(${PROJECT_NAME}_test PRIVATE -DHAVE_CRC32C)
        target_compile_options(${PROJECT_NAME}_test PRIVATE -march=armv8.1-a)
    endif ()

    # detect software version endianness
    test_big_endian(HAVE_BIG_ENDIAN)
    if (${HAVE_BIG_ENDIAN})
        message(STATUS "System is BIG ENDIAN")
        target_compile_definitions(${PROJECT_NAME}_test PRIVATE -DHAVE_BIG_ENDIAN)
    else ()
        message(STATUS "System is LITTLE ENDIAN")
    endif ()

    if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR
            "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang" OR
            "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")

        target_compile_options(${PROJECT_NAME}_test PRIVATE -fno-omit-frame-pointer)

        if (SANITIZER)
            target_compile_options(${PROJECT_NAME}_test PRIVATE -fsanitize=${SANITIZER})
            target_link_options(${PROJECT_NAME}_test PRIVATE -fsanitize=${SANITIZER})
        endif ()
    endif ()


    add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)

    SET(MEMORYCHECK_COMMAND_OPTIONS
            "-q --log-fd=2 --trace-children=yes --track-origins=yes       \
           --leak-check=full --show-leak-kinds=all  \
           --error-exitcode=255")

    add_custom_target(valgrind_${PROJECT_NAME} ${CMAKE_COMMAND}
            -E env CTEST_OUTPUT_ON_FAILURE=1
            ${CMAKE_CTEST_COMMAND} -C $<CONFIG>
            --overwrite MemoryCheckCommandOptions=${MEMORYCHECK_COMMAND_OPTIONS}
            --verbose -T memcheck WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

    add_custom_target(check_${PROJECT_NAME} ${CMAKE_COMMAND}
            -E env CTEST_OUTPUT_ON_FAILURE=1
            ${CMAKE_CTEST_COMMAND} -C $<CONFIG> --verbose
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

    # ----------------------- - Code Coverage Start ----------------------------- #

    if (${CMAKE_BUILD_TYPE} MATCHES "Coverage")
        if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
            target_compile_options(${PROJECT_NAME}_test PRIVATE --coverage)
            target_link_libraries(${PROJECT_NAME}_test gcov)
        else ()
            message(FATAL_ERROR "Only GCC is supported for coverage")
        endif ()
    endif ()

    add_custom_target(coverage_${PROJECT_NAME})
    add_custom_command(
            TARGET coverage_${PROJECT_NAME}
            COMMAND lcov --capture --directory .
            --output-file coverage.info --rc lcov_branch_coverage=1 --rc lcov_excl_br_line='assert'
            COMMAND lcov --remove coverage.info '/usr/*' '*example*' '*test*'
            --output-file coverage.info --rc lcov_branch_coverage=1 --rc lcov_excl_br_line='assert'
            COMMAND lcov --list coverage.info --rc lcov_branch_coverage=1 --rc lcov_excl_br_line='assert'
    )

    add_dependencies(coverage_${PROJECT_NAME} check_${PROJECT_NAME})

    # -------------------------- Code Coverage End ------------------------------ #
endif ()
# ----------------------- Test Configuration End ---------------------------- #


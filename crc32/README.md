### CRC32c function

- Same code from : https://stackoverflow.com/a/17646775
- Fixed some alignment issues, replaced asm code with compiler intrinsics
- Added aarch64 hardware support
- Requires architecture and endianness detection, CMAKE example is :

```cmake
## Cmake

include(CheckCCompilerFlag)
include (TestBigEndian)

# Detect x86 and sse4.2 support
check_c_compiler_flag(-msse4.2 HAVE_CRC32_HARDWARE)
if (${HAVE_CRC32_HARDWARE})
  message(STATUS "CPU have -msse4.2, defined HAVE_CRC32C")
  target_compile_options(${PROJECT_NAME}_test PRIVATE -msse4.2)
  target_compile_definitions(${PROJECT_NAME}_test PRIVATE -DHAVE_CRC32C)
endif ()

# Detect aarch64 and set march=armv8.1-a. armv7 doesn't have CRC32c instruction
# so, armv7 will be unsupported with this flag. 
if (CMAKE_SYSTEM_PROCESSOR STREQUAL aarch64)
  message(STATUS "CPU = aarch64, defined HAVE_CRC32C, -march=armv8.1-a")
  target_compile_definitions(${PROJECT_NAME}_test PRIVATE -DHAVE_CRC32C)
  target_compile_options(${PROJECT_NAME}_test PRIVATE -march=armv8.1-a)
endif()

# Detect software version endianness
test_big_endian(HAVE_BIG_ENDIAN)
if (${HAVE_BIG_ENDIAN})
  message(STATUS "System is BIG ENDIAN")
  target_compile_definitions(${PROJECT_NAME}_test PRIVATE -DHAVE_BIG_ENDIAN)
else()
  message(STATUS "System is LITTLE ENDIAN")
endif ()
```

```c

#include "sc_crc32.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
    uint32_t crc;
    const uint8_t buf[100] = {0};

    sc_crc32_init();

    // Partial calculation example
    crc = sc_crc32(0, buf, 10);
    crc = sc_crc32(crc, buf + 10, sizeof(buf) - 10);
    printf("crc : %u \n", crc);

    // Calculate at once
    crc = sc_crc32(0, buf, sizeof(buf));
    printf("crc : %u \n", crc);

    return 0;
}

```
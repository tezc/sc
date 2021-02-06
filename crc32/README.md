# CRC32c function

- Same code from : https://stackoverflow.com/a/17646775
- Fixed some alignment issues, replaced asm code with compiler intrinsics
- This is Crc32<b>c</b> algorithm, not Crc32

- Compile time switch to hardware version if supported  
  (crc32c instruction on x64), fallback to software version if not available
- You should compile with "-msse4.2" flag and define HAVE_CRC32C to use hardware  
  version
- Stackoverflow answer has runtime dispatch between hardware and software  
  versions if you'd like that.




```cmake
## Cmake

# Needs HAVE_CRC32C definition to enable CPU instruction usage.

## Only use hardware version in 64 bit architectures.
include(CheckCCompilerFlag)

if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    check_c_compiler_flag(-msse4.2 HAVE_CRC32_HARDWARE)
    if (${HAVE_CRC32_HARDWARE})
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4.2 -DHAVE_CRC32C")
    endif ()
endif()
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
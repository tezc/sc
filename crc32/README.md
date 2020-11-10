# CRC32c function

- Same code from : https://stackoverflow.com/a/17646775
- Fixed some alignment issues, replaced asm code with compiler intrinsics
- Just copy <b>sc_crc32.h</b> and <b>sc_crc32.c</b> to your project.

- Compile time switch to hardware version if supported (crc32c instruction on x64),
  fallback to software version if not available
- See CmakeLists.txt, it just checks "-msse4.2" flag. Stackoverflow answer has
  runtime dispatch between hardware and software versions if you'd like that.
  

 - This is Crc32<b>c</b> algorithm, not Crc32                                       
 - A faster version might be possible with 'PCLMULQDQ' instruction as explained here
 
 [[link]](https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/crc-iscsi-polynomial-crc32-instruction-paper.pdf)
 
  
  ```cmake
## Cmake

## Only use hardware version in 64 bit architectures.
if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    check_c_compiler_flag(-msse4.2 HAVE_CRC32_HARDWARE)
    if (${HAVE_CRC32_HARDWARE})
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4.2 -DSC_HAVE_CRC32_HARDWARE")
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

    sc_crc32_global_init();

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
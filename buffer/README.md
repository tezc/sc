# Buffer

#### Overview

- Buffer implementation for serializing data/protocol implementation.
- Provides put/get for binary data, 8/16/32/64 bit integers, double and strings.  
- Strings are kept length prefixed and null ended. So, no need to copy string  
  when you are reading, you just get the pointer. This is useful to avoid memory  
  allocation overhead.
- Integer operations are compiled into bounds check + a single MOV instruction  
  on x86. Buffer keeps data in Little Endian format, so on big endian systems,  
  integer put/get is bswap(byte swap) + MOV.
- Just copy <b>sc_buf.h</b> and <b>sc_buf.c</b> to your project.


##### Usage

```c
#include "sc_buf.h"
#include <stdio.h>

void basic()
{
    struct sc_buf buf;
    sc_buf_init(&buf, 1024);

    sc_buf_put_32(&buf, 16);
    sc_buf_put_str(&buf, "test");
    sc_buf_put_fmt(&buf, "value is %d", 3);

    printf("%d \n", sc_buf_get_32(&buf));
    printf("%s \n", sc_buf_get_str(&buf));
    printf("%s \n", sc_buf_get_str(&buf));

    sc_buf_term(&buf);
}

void error_check()
{
    uint32_t val, val2;
    struct sc_buf buf;

    sc_buf_init(&buf, 1024);
    sc_buf_put_32(&buf, 16);

    val = sc_buf_get_32(&buf);
    val2 = sc_buf_get_32(&buf); // This will set error flag in buffer;

    if (sc_buf_valid(&buf) == false) {
        printf("buffer is corrupt");
        exit(EXIT_FAILURE);
    }

    sc_buf_term(&buf);
}

int main()
{
    basic();
    error_check();

    return 0;
}
```
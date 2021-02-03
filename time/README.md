# Time functions

#### Overview

- Time and sleep functions for Unixes and Windows

##### Usage


```c

#include "sc_time.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
    uint64_t t = sc_time_ms();
    sc_time_sleep(1000);
    printf("%lu \n", (unsigned long) (sc_time_ms() - t));

    return 0;
}

```
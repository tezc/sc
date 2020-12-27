# Condition

#### Overview

- Condition wrapper.
- Provides passing data between signal and wait threads. Signal will mark the  
  condition 'done', so when another thread calls wait(), it won't be blocked, 
  it will return immediately with the user provided data.
- Just copy <b>sc_cond.h</b> and <b>sc_cond.c</b> to your project.

##### Usage

```c
#include "sc_cond.h"

#include <stdio.h>

int main()
{
    struct sc_cond cond;

    sc_cond_init(&cond);
    sc_cond_signal(&cond, "test"); // Call this on thread-1
    char* p = sc_cond_wait(&cond); // Call this on another thread.

    printf("%s \n", p);

    return 0;
}
```
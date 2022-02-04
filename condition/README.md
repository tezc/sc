### Condition

### Overview

- Condition wrapper for Posix and Windows.
- Normally, if no thread waits on a condition, signal is missed. This  
  implementation differs as it will keep a result variable when 'signal' is  
  called. Signal will mark the condition 'done'. When another thread calls  
  wait(), it won't be blocked, it will return immediately with the user  
  provided data.

### Usage

```c
#include "sc_cond.h"

#include <stdio.h>

int main()
{
    struct sc_cond cond;

    sc_cond_init(&cond); // Init once
    
    sc_cond_signal(&cond, "test"); // Call this on thread-1
    char* p = sc_cond_wait(&cond); // Call this on another thread.

    printf("%s \n", p); // Prints "test"
    
    sc_cond_term(&cond); // Destroy

    return 0;
}

```
### Timer

### Overview

- Hashed timing wheel implementation.
- Provides fast cancel(O(1)) and poll operations compared to a priority queue.
- Timers in the same hash slot are not ordered between each other. So, basically    
  this data structure trades accuracy for performance. Schedule a timer for  
  10000ms and another for 10001ms and you might see 10001ms timer expires  
  just before 10000ms timer. Default tick is 16 ms, so, timers in the same 16ms  
  interval may expire out of order.


### Usage


```c
#include "sc_timer.h"

#include <errno.h>
#include <stdio.h>
#include <time.h>

uint64_t time_ms();
void sleep_ms(uint64_t milliseconds);


void callback(void *arg, uint64_t timeout, void *data)
{
    struct sc_timer *timer = arg;
    char *timer_name = data;

    printf("timeout : %zu, data : %s \n", timeout, timer_name);
    // Schedule back
    sc_timer_add(timer, "timer1", 1000);
}

int main(int argc, char *argv[])
{
    uint64_t next_timeout;
    struct sc_timer timer;

    sc_timer_init(&timer, time_ms());
    sc_timer_add(&timer, "timer1", 1000);

    while (true) {
        next_timeout = sc_timer_timeout(&timer, time_ms(), &timer, callback);
        sleep_ms(next_timeout);
    }

    return 0;
}

```
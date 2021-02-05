# Timer

#### Overview

- Hashed timing wheel implementation.
- Provides fast cancel(O(1)) and poll operations compared to a priority queue.
- Timers in the same hash slot are not ordered between each other. So, basically    
  this data structure trades accuracy for performance. Schedule a timer for  
  10000ms and another for 10001ms and you might see 10001ms timer expires  
  just before 10000ms timer. 


##### Usage


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

/**
 * 
 * sleep() and time() functions for Windows and Posix. 
 * These functions are here just to make this example run on your platform.
 * 
 * Use your monotonic timer and sleep function of your target platform.
 * 
 */

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#else
    #include <sys/time.h>
#endif

uint64_t time_ms()
{
#if defined(_WIN32) || defined(_WIN64)
    //  System frequency does not change at run-time, cache it
    static int64_t frequency = 0;
    if (frequency == 0) {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        assert(freq.QuadPart != 0);
        frequency = freq.QuadPart;
    }
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return (int64_t)(count.QuadPart * 1000) / frequency;
#else
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (int64_t)((int64_t) ts.tv_sec * 1000 +
                     (int64_t) ts.tv_nsec / 1000000);
#endif
}

void sleep_ms(uint64_t milliseconds)
{
#if defined(_WIN32) || defined(_WIN64)
    Sleep(milliseconds);
#else
    int rc;
    struct timespec t;

    t.tv_sec = milliseconds / 1000;
    t.tv_nsec = (milliseconds % 1000) * 1000000;

    do {
        rc = nanosleep(&t, NULL);
    } while (rc != 0 && errno != EINTR);
#endif
}

```
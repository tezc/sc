# Mutex

### Overview 

- Mutex wrapper for Windows and POSIX systems.
- Just copy <b>sc_mutex.h</b> and <b>sc_mutex.c</b> to your project.

#### Usage


```c
#include "sc_mutex.h"

int main(int argc, char *argv[])
{
    struct sc_mutex mutex;

    sc_mutex_init(&mutex);

    sc_mutex_lock(&mutex);
    // Exclusive area.
    sc_mutex_unlock(&mutex);

    sc_mutex_term(&mutex);

    return 0;
}

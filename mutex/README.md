# Mutex

### Mutex wrapper 

- Basic mutex wrapper for Posix and Windows.

```c

#include "sc_mutex.h"

int main(int argc, char *argv[])
{
    struct sc_mutex mutex;

    sc_mutex_init(&mutex);

    sc_mutex_lock(&mutex);
    sc_mutex_unlock(&mutex);

    sc_mutex_term(&mutex);

    return 0;
}
```


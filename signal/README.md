### Signal handling

### Overview

- Signal handling to handle shutdown and fatal signals.
- Also provides signal safe snprintf/vsnprintf.
- On shutdown signal, writes 1 byte to a fd which is set by user, so your app   
  can shutdown properly. 
- Double shutdown signal handling: e.g user presses CTRL+C twice, exits without  
  waiting graceful shutdown.
- Fatal signal handling. Prints backtrace with program counter indicator. You   
  should compile with debug symbols or with -rdynamic for GCC.
- If log fd is set, it will print logs to your log file.


### Usage


```c
#include "sc_signal.h"

#include <stdio.h>
#include <unistd.h>

int main()
{
    char tmp[1];
    int fds[2];

    sc_signal_init();  // Call once in your application

    pipe(fds);

    sc_signal_shutdown_fd = fds[1];

    read(fds[0], tmp, sizeof(tmp));
    // Press CTRL+C.
    printf("Received shutdown signal \n");


    return 0;
}
```

### Backtrace on fatal signals

If 'HAVE_BACKTRACE' is defined, it will print backtrace on fatal signals. To  
detect if system has backtrace() function on CMAKE : 

```cmake
include(CheckCCompilerFlag)

check_c_source_compiles("
    #include <execinfo.h>
    #include <unistd.h>

    int main(int argc, char **argv) {
        void *array[10];
        size_t size = backtrace(array, 10);
        backtrace_symbols_fd(array, size, STDERR_FILENO);
        return 0;
}" HAVE_BACKTRACE)

FIND_LIBRARY(EXECINFO_LIBRARY NAMES execinfo)
IF (EXECINFO_LIBRARY)
    SET(CMAKE_REQUIRED_LIBRARIES "${EXECINFO_LIBRARY}")
ENDIF(EXECINFO_LIBRARY)

if (${HAVE_BACKTRACE})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_BACKTRACE")
endif ()

```
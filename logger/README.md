### Logger

### Overview

- Log destination can be stdout, file and user callback.
- You can pass logs to all destinations at the same time.
- Log files are rotated.
- Thread-safe, requires pthread.

### Usage


```c
#include "sc_log.h"

int log_callback(void *arg, enum sc_log_level level,
                               const char *fmt, va_list va)
{
    const char *my_app = arg;
    const char *level_str = sc_log_levels[level].str;

    fprintf(stdout, " %s received log : level = [%s] ", my_app, level_str);
    vfprintf(stdout, fmt, va);

    return 0;
}

int main(int argc, char *argv[])
{
    const char* my_app_name = "my app";

    sc_log_init(); // Call once when your app starts.

    //Default log-level is 'info' and default destination is 'stdout'
    sc_log_info("Hello world!\n");

    //Enable logging to file.
    sc_log_set_file("log.0.txt", "log-latest.txt");

    //stdout and file will get the log line
    sc_log_info("to stdout and file!\n");

    //Enable callback
    sc_log_set_callback((void*) my_app_name, log_callback);

    //stdout, file and callback will get the log line
    sc_log_info("to all!\n");

    sc_log_term(); // Call once on shutdown.

    return 0;
}

```

Output is like : 

```
[2021-02-03 04:46:22][INFO ][my app thread] (log_example.c:28) Hello world! 
[2021-02-03 04:46:22][INFO ][my app thread] (log_example.c:34) to stdout and file! 
[2021-02-03 04:46:22][INFO ][my app thread] (log_example.c:40) to all!
```

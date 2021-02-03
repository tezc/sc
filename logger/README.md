# Logger

### Overview

- Log destination can be stdout, file and user callback.
- Possible to get logs to all destinations at the same time.
- Log files are rotated.
- Thread-safe.
- Just copy <b>sc_log.h</b> and <b>sc_log.c</b> to your project.

#### Usage


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

    //sc_log_init(); is not thread-safe, it must be called by a single thread.
    sc_log_init();
    sc_log_set_thread_name("my-app-thread"); 

    //Default log-level is 'info' and default destination is 'stdout'
    sc_log_info("Hello world!");

    //Enable logging to file.
    sc_log_set_file("log.0.txt", "log-latest.txt");

    //stdout and file will get the log line
    sc_log_info("to stdout and file!");

    //Enable callback
    sc_log_set_callback(log_callback, (void*) my_app_name);

    //stdout, file and callback will get the log line
    sc_log_info("to all!");
    sc_log_info("to all!");

    //sc_log_term(); is not thread-safe, it must be called by a single thread.
    sc_log_term();

    return 0;
}

```

Output is like : 

```
[2021-02-03 04:46:22][INFO ][my app thread] (log_example.c:28) Hello world! 
[2021-02-03 04:46:22][INFO ][my app thread] (log_example.c:34) to stdout and file! 
[2021-02-03 04:46:22][INFO ][my app thread] (log_example.c:40) to all!
[2021-02-03 04:47:32][INFO ][my app thread] (log_example.c:41) to all! 
```

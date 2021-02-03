#define SC_LOG_PRINT_FILE_NAME

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

    //This will set a thread local buffer
    sc_log_set_thread_name("my app thread");

    //Default log-level is 'info' and default destination is 'stdout'
    sc_log_info("Hello world! \n");

    //Enable logging to file.
    sc_log_set_file("log.0.txt", "log-latest.txt");

    //stdout and file will get the log line
    sc_log_info("to stdout and file! \n");

    //Enable callback
    sc_log_set_callback(log_callback, (void*) my_app_name);

    //stdout, file and callback will get the log line
    sc_log_info("to all! \n");
    sc_log_info("to all! \n");

    //sc_log_term(); is not thread-safe, it must be called by a single thread.
    sc_log_term();

    return 0;
}

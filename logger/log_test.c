#define SC_LOG_PRINT_FILE_NAME
#include "sc_log.h"

#include <assert.h>
#include <stdarg.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
    #pragma warning(disable : 4996)
#endif

int callback(void *arg, enum sc_log_level level, const char *fmt, va_list va)
{
    *(int *) arg = *(int *) arg + 1;
    return 0;
}

void test1(void)
{
    int count = 0;

    sc_log_init();
    sc_log_set_callback(callback, &count);
    assert(sc_log_set_level("errrorr") == -1);
    sc_log_debug("test");
    assert(count == 0);

    sc_log_set_level("DEBUG");
    sc_log_debug("test");
    assert(count == 1);
    sc_log_info("test");
    assert(count == 2);
    sc_log_warn("test");
    assert(count == 3);
    sc_log_error("test");
    assert(count == 4);

    count = 0;
    sc_log_set_level("INFO");
    sc_log_debug("test");
    assert(count == 0);
    sc_log_info("test");
    assert(count == 1);
    sc_log_warn("test");
    assert(count == 2);
    sc_log_error("test");
    assert(count == 3);

    count = 0;
    sc_log_set_level("WARN");
    sc_log_debug("test");
    assert(count == 0);
    sc_log_info("test");
    assert(count == 0);
    sc_log_warn("test");
    assert(count == 1);
    sc_log_error("test");
    assert(count == 2);

    count = 0;
    sc_log_set_level("OFF");
    sc_log_debug("test");
    assert(count == 0);
    sc_log_info("test");
    assert(count == 0);
    sc_log_warn("test");
    assert(count == 0);
    sc_log_error("test");
    assert(count == 0);

    sc_log_set_level("INFO");
    sc_log_set_stdout(false);
    assert(sc_log_set_file("prev.txt", "current.txt") == 0);
    for (int i = 0; i < 100000; i++) {
        sc_log_error("testtesttesttesttesttesttesttesttesttesttesttest");
    }

    FILE *fp = fopen("prev.txt", "rb");
    assert(fp != NULL);
    fseek(fp, 0, SEEK_END);
    assert(ftell(fp) >= SC_LOG_FILE_SIZE);
    fclose(fp);

    sc_log_term();
}

#ifdef SC_HAVE_WRAP

#include <pthread.h>

int callback2(void *arg, enum sc_log_level level, const char *fmt, va_list va)
{
    vfprintf(stdout, fmt, va);
    vfprintf(stdout, fmt, va);

    return 0;
}

bool mock_fprintf = false;
FILE *fprintf_file;
int fprintf_count = 0;
int fprintf_ret = 0;
extern int __real_fprintf(FILE *stream, const char *format, ...);
int __wrap_fprintf(FILE *stream, const char *format, ...)
{
    int rc;
    va_list va;

    if (!mock_fprintf) {
        va_start(va, format);
        rc = vfprintf(stream, format, va);
        va_end(va);

        return rc;
    }

    fprintf_file = stream;
    fprintf_count++;
    return fprintf_ret;
}

bool mock_vfprintf = false;
FILE *vprintf_file;
int vfprintf_count = 0;
int vfprintf_ret = 0;
extern int __real_vfprintf(FILE *stream, const char *format, va_list arg);
int __wrap_vfprintf(FILE *stream, const char *format, va_list arg)
{
    if (!mock_vfprintf) {
        return __real_vfprintf(stream, format, arg);
    }
    vprintf_file = stream;
    vfprintf_count++;
    return vfprintf_ret;
}

bool mock_fopen = false;
extern FILE *__real_fopen(const char *filename, const char *format);
FILE *__wrap_fopen(const char *filename, const char *mode)
{
    if (!mock_fopen) {
        return __real_fopen(filename, mode);
    }

    return NULL;
}

bool mock_fclose = false;
extern int __real_fclose (FILE *__stream);
int __wrap_fclose (FILE *__stream)
{
    if (!mock_fclose) {
        return __real_fclose(__stream);
    }

    __real_fclose(__stream);
    return -1;
}

bool mock_localtime = false;
extern struct tm *__real_localtime(const time_t *timer);
struct tm *__wrap_localtime(const time_t *timer)
{
    if (!mock_localtime) {
        return __real_localtime(timer);
    }

    return NULL;
}

bool mock_attrinit = false;
extern int __real_pthread_mutexattr_init(pthread_mutexattr_t *attr);
int __wrap_pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
    if (!mock_attrinit) {
        return __real_pthread_mutexattr_init(attr);
    }

    return -1;
}

bool mock_mutexinit = false;
extern int __real_pthread_mutex_init(pthread_mutex_t *__mutex,
                                     const pthread_mutexattr_t *__mutexattr);
int __wrap_pthread_mutex_init(pthread_mutex_t *__mutex,
                              const pthread_mutexattr_t *__mutexattr)
{
    if (!mock_mutexinit) {
        return __real_pthread_mutex_init(__mutex, __mutexattr);
    }

    return -1;
}


void fail_test(void)
{
    mock_attrinit = true;
    assert(sc_log_init() < 0);
    mock_attrinit = false;
    mock_mutexinit = true;
    assert(sc_log_init() < 0);
    mock_mutexinit = false;
    assert(sc_log_init() == 0);

    mock_fprintf = true;
    mock_vfprintf = true;

    vfprintf_count = 0;
    sc_log_info("loggg");
    assert(vfprintf_count > 0);
    assert(vprintf_file == stdout);

    vfprintf_ret = -1;
    assert(sc_log_info("log") != 0);
    vfprintf_ret = 0;

    vfprintf_count = 0;
    sc_log_set_stdout(false);
    sc_log_info("loggg");
    assert(vfprintf_count == 0);

    sc_log_set_stdout(true);
    sc_log_set_file("tmp.txt", "tmp2.txt");
    sc_log_set_callback(callback2, NULL);

    fprintf_count = 0;
    vfprintf_count = 0;
    sc_log_info("loggg");
    assert(vfprintf_count + fprintf_count == 6);
    sc_log_set_callback(NULL, NULL);

    fprintf_count = 0;
    vfprintf_count = 0;
    sc_log_set_stdout(false);
    sc_log_set_file(NULL, NULL);
    sc_log_info("loggg");
    assert(vfprintf_count + fprintf_count == 0);

    sc_log_set_stdout(true);
    fprintf_ret = -1;
    assert(sc_log_info("test") == -1);
    fprintf_ret = 0;
    assert(sc_log_info("test") == 0);

    sc_log_set_stdout(false);
    sc_log_set_file("tmp.txt", "tmp2.txt");
    vfprintf_ret = -1;
    assert(sc_log_info("test") == -1);
    vfprintf_ret = 0;
    assert(sc_log_info("test") == 0);
    fprintf_ret = -1;
    assert(sc_log_info("test") == -1);
    fprintf_ret = 0;
    assert(sc_log_info("test") == 0);
    fprintf_ret = -1;
    assert(sc_log_set_file("tmp.txt", "tmp2.txt") == -1);
    fprintf_ret = 0;

    assert(sc_log_set_file(NULL, "test.txt") == 0);
    mock_fopen = true;
    assert(sc_log_set_file("prev.txt", "current.txt") == -1);
    mock_fopen = false;
    assert(sc_log_set_file("prev.txt", "current.txt") == 0);
    mock_localtime = true;
    assert(sc_log_error("test") == -1);
    mock_localtime = false;

    mock_vfprintf = false;
    mock_fprintf = false;
    mock_fopen = true;
    int failed = 0;
    for (int i = 0; i < 40000; i++) {
        failed = sc_log_error("testtesttesttesttesttesttesttesttesttestest");
        if (failed < 0) {
            break;
        }
    }
    assert(failed == -1);
    mock_fopen = false;

    assert(sc_log_set_file("prev.txt", "current.txt") == 0);
    mock_fclose = true;
    assert(sc_log_term() != 0);

    mock_fclose = false;
    mock_fprintf = false;
    mock_vfprintf = false;
    mock_localtime = false;
    mock_fopen = false;
}
#else
void fail_test(void)
{
}
#endif

int log_callback(void *arg, enum sc_log_level level,
                 const char *fmt, va_list va)
{
    const char *my_app = arg;
    const char *level_str = sc_log_levels[level].str;

    fprintf(stdout, " %s received log : level = [%s] ", my_app, level_str);
    vfprintf(stdout, fmt, va);

    return 0;
}

void example(void)
{
    const char* my_app_name = "my app";

    //sc_log_init() is not thread-safe, it must be called by a single thread.
    sc_log_init();

    //Default log-level is 'info' and default destination is 'stdout'
    sc_log_info("Hello world!\n");

    //Enable logging to file.
    sc_log_set_file("log.0.txt", "log-latest.txt");

    //stdout and file will get the log line
    sc_log_info("to stdout and file!\n");

    //Enable callback
    sc_log_set_callback(log_callback, (void*) my_app_name);

    //stdout, file and callback will get the log line
    sc_log_info("to all!\n");
    sc_log_info("to all!\n");

    //sc_log_term(); is not thread-safe, it must be called by a single thread.
    sc_log_term();
}

int main(int argc, char *argv[])
{
    sc_log_set_thread_name("My thread");

    fail_test();
    example();
    test1();

    return 0;
}

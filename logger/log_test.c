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
	(void) level;
	(void) fmt;
	(void) va;

	*(int *) arg = *(int *) arg + 1;
	return 0;
}

void test1(void)
{
	int count = 0;

	sc_log_init();
	sc_log_set_callback(&count, callback);
	assert(sc_log_set_level("errrorr") == -1);
	assert(sc_log_set_level("errox") == -1);
	assert(sc_log_set_level("err") == -1);
	sc_log_debug("test \n");
	assert(count == 0);

	assert(sc_log_set_level("debuG") == 0);
	sc_log_set_level("DEBUG");
	sc_log_debug("test \n");
	assert(count == 1);
	sc_log_info("test \n");
	assert(count == 2);
	sc_log_warn("test \n");
	assert(count == 3);
	sc_log_error("test \n");
	assert(count == 4);

	count = 0;
	assert(sc_log_set_level("iNfO") == 0);
	sc_log_set_level("INFO");
	sc_log_debug("test \n");
	assert(count == 0);
	sc_log_info("test \n");
	assert(count == 1);
	sc_log_warn("test \n");
	assert(count == 2);
	sc_log_error("test \n");
	assert(count == 3);

	count = 0;
	sc_log_set_level("WARN");
	sc_log_debug("test \n");
	assert(count == 0);
	sc_log_info("test \n");
	assert(count == 0);
	sc_log_warn("test \n");
	assert(count == 1);
	sc_log_error("test \n");
	assert(count == 2);

	count = 0;
	sc_log_set_level("OFF");
	sc_log_debug("test \n");
	assert(count == 0);
	sc_log_info("test \n");
	assert(count == 0);
	sc_log_warn("test \n");
	assert(count == 0);
	sc_log_error("test \n");
	assert(count == 0);

	sc_log_set_level("INFO");
	sc_log_set_stdout(false);
	assert(sc_log_set_file("prev.txt", "current.txt") == 0);
	for (int i = 0; i < 100000; i++) {
		sc_log_error("testtesttesttesttesttesttesttesttesttesttesttes"
			     "t");
	}

	FILE *fp = fopen("prev.txt", "rb");
	assert(fp != NULL);
	fseek(fp, 0, SEEK_END);
	assert(ftell(fp) >= SC_LOG_FILE_SIZE);
	fclose(fp);

	sc_log_term();

	sc_log_init();
	sc_log_term();
}

#ifdef SC_HAVE_WRAP

#include <errno.h>
#include <pthread.h>

int callback2(void *arg, enum sc_log_level level, const char *fmt, va_list va)
{
	(void) arg;
	(void) level;

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

bool mock_ftell = false;
extern long int __real_ftell(FILE *stream);
extern long int __wrap_ftell(FILE *stream)
{
	if (mock_ftell) {
		return -1;
	}

	return __real_ftell(stream);
}

bool mock_fclose = false;
extern int __real_fclose(FILE *__stream);
int __wrap_fclose(FILE *__stream)
{
	if (!mock_fclose) {
		return __real_fclose(__stream);
	}

	__real_fclose(__stream);
	return -1;
}

bool mock_localtime_r = false;
extern struct tm *__real_localtime_r(const time_t *timer, struct tm *res);
struct tm *__wrap_localtime_r(const time_t *timer, struct tm *res)
{
	if (!mock_localtime_r) {
		return __real_localtime_r(timer, res);
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
	sc_log_set_callback(NULL, callback2);

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
	assert(sc_log_set_file("test.txt", NULL) == 0);
	mock_fopen = true;
	assert(sc_log_set_file("prev.txt", "current.txt") == -1);
	mock_fopen = false;
	mock_ftell = true;
	assert(sc_log_set_file("prev.txt", "current.txt") == -1);
	mock_ftell = false;

	assert(sc_log_set_file("prev.txt", "current.txt") == 0);
	mock_localtime_r = true;
	assert(sc_log_error("test") == -1);
	mock_localtime_r = false;

	mock_vfprintf = false;
	mock_fprintf = false;
	mock_fopen = true;
	int failed = 0;
	for (int i = 0; i < 40000; i++) {
		failed = sc_log_error("testtesttesttesttesttesttesttesttesttest"
				      "est");
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
	mock_localtime_r = false;
	mock_fopen = false;
}
#else
void fail_test(void)
{
}
#endif

int log_callback(void *arg, enum sc_log_level level, const char *fmt,
		 va_list va)
{
	const char *my_app = arg;
	const char *level_str = sc_log_levels[level].str;

	fprintf(stdout, " %s received log : level = [%s] ", my_app, level_str);
	vfprintf(stdout, fmt, va);

	return 0;
}

void example(void)
{
	const char *my_app_name = "my app";

	sc_log_init(); // Call once when your app starts.

	// Default log-level is 'info' and default destination is 'stdout'
	sc_log_info("Hello world!\n");

	// Enable logging to file.
	sc_log_set_file("log.0.txt", "log-latest.txt");

	// stdout and file will get the log line
	sc_log_info("to stdout and file!\n");

	// Enable callback
	sc_log_set_callback((void *) my_app_name, log_callback);

	// stdout, file and callback will get the log line
	sc_log_info("to all!\n");

	sc_log_term(); // Call once on shutdown.
}

int main()
{
	sc_log_set_thread_name("My thread");

	fail_test();
	example();
	test1();

	return 0;
}

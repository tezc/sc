/*
 * MIT License
 *
 * Copyright (c) 2021 Ozan Tezcan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _XOPEN_SOURCE
    #define _XOPEN_SOURCE 700
#endif

#include "sc_log.h"

#include <ctype.h>
#include <errno.h>
#include <time.h>

#ifndef thread_local
    #if __STDC_VERSION__ >= 201112 && !defined __STDC_NO_THREADS__
        #define thread_local _Thread_local
    #elif defined _WIN32 && (defined _MSC_VER || defined __ICL ||              \
                             defined __DMC__ || defined __BORLANDC__)
        #define thread_local __declspec(thread)
    #elif defined __GNUC__ || defined __SUNPRO_C || defined __xlC__
        #define thread_local __thread
    #else
        #error "Cannot define  thread_local"
    #endif
#endif

#if __STDC_VERSION__ >= 201112 && !defined __STDC_NO_ATOMIC__
    #define SC_ATOMIC
    #include <stdatomic.h>

    #define sc_atomic _Atomic
    #define sc_atomic_store(var, val)                                          \
        (atomic_store_explicit(var, val, memory_order_relaxed))
    #define sc_atomic_load(var)                                                \
        (atomic_load_explicit(var, memory_order_relaxed))
#else
    #define sc_atomic
    #define sc_atomic_store(var, val) ((*(var)) = (val))
    #define sc_atomic_load(var)       (*(var))
#endif

thread_local char sc_name[32] = "Thread";

#if defined(_WIN32) || defined(_WIN64)

    #pragma warning(disable : 4996)
    #define strcasecmp _stricmp
    #define localtime_r(a, b) (localtime_s(b, a) == 0 ? b : NULL)
    #include <windows.h>

struct sc_log_mutex
{
    CRITICAL_SECTION mtx;
};

int sc_log_mutex_init(struct sc_log_mutex *mtx)
{
    InitializeCriticalSection(&mtx->mtx);
    return 0;
}

int sc_log_mutex_term(struct sc_log_mutex *mtx)
{
    DeleteCriticalSection(&mtx->mtx);
    return 0;
}

void sc_log_mutex_lock(struct sc_log_mutex *mtx)
{
    EnterCriticalSection(&mtx->mtx);
}

void sc_log_mutex_unlock(struct sc_log_mutex *mtx)
{
    LeaveCriticalSection(&mtx->mtx);
}

#else

    #include <pthread.h>

struct sc_log_mutex
{
    pthread_mutex_t mtx;
};

int sc_log_mutex_init(struct sc_log_mutex *mtx)
{
    int rc;

    pthread_mutexattr_t attr;
    pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

    mtx->mtx = mut;

    rc = pthread_mutexattr_init(&attr);
    if (rc != 0) {
        return rc;
    }

    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
    rc = pthread_mutex_init(&mtx->mtx, &attr);
    pthread_mutexattr_destroy(&attr);

    return rc;
}

void sc_log_mutex_term(struct sc_log_mutex *mtx)
{
    pthread_mutex_destroy(&mtx->mtx);
}

void sc_log_mutex_lock(struct sc_log_mutex *mtx)
{
    pthread_mutex_lock(&mtx->mtx);
}

void sc_log_mutex_unlock(struct sc_log_mutex *mtx)
{
    pthread_mutex_unlock(&mtx->mtx);
}

#endif

struct sc_log
{
    FILE *fp;
    const char *current_file;
    const char *prev_file;
    size_t file_size;

    struct sc_log_mutex mtx;
    sc_atomic enum sc_log_level level;

    bool to_stdout;

    void *arg;
    int (*cb)(void *, enum sc_log_level, const char *, va_list);
};

struct sc_log sc_log;

int sc_log_init(void)
{
    int rc;

    sc_log = (struct sc_log){0};

    sc_atomic_store(&sc_log.level, SC_LOG_INFO);
    sc_log.to_stdout = true;

    rc = sc_log_mutex_init(&sc_log.mtx);
    if (rc != 0) {
        errno = rc;
    }

    return rc;
}

int sc_log_term(void)
{
    int rc = 0;

    if (sc_log.fp) {
        rc = fclose(sc_log.fp);
        if (rc != 0) {
            rc = -1;
        }
    }

    sc_log_mutex_term(&sc_log.mtx);

    return rc;
}

void sc_log_set_thread_name(const char *name)
{
    strncpy(sc_name, name, sizeof(sc_name) - 1);
}

static int sc_strcasecmp(const char *a, const char *b)
{
    int n;

    for (;; a++, b++) {
        if (*a == 0 && *b == 0) {
            return 0;
        }

        if ((n = tolower(*a) - tolower(*b)) != 0) {
            return n;
        }
    }
}

int sc_log_set_level(const char *str)
{
    size_t count = sizeof(sc_log_levels) / sizeof(sc_log_levels[0]);

    for (size_t i = 0; i < count; i++) {
        if (sc_strcasecmp(str, sc_log_levels[i].str) == 0) {
#ifdef SC_ATOMIC
            sc_atomic_store(&sc_log.level, sc_log_levels[i].id);
#else
            sc_log_mutex_lock(&sc_log.mtx);
            sc_log.level = sc_log_levels[i].id;
            sc_log_mutex_unlock(&sc_log.mtx);
#endif
            return 0;
        }
    }

    return -1;
}

void sc_log_set_stdout(bool enable)
{
    sc_log_mutex_lock(&sc_log.mtx);
    sc_log.to_stdout = enable;
    sc_log_mutex_unlock(&sc_log.mtx);
}

int sc_log_set_file(const char *prev_file, const char *current_file)
{
    int rc = 0, saved_errno = 0;
    long size;
    FILE *fp = NULL;

    sc_log_mutex_lock(&sc_log.mtx);

    if (sc_log.fp != NULL) {
        fclose(sc_log.fp);
        sc_log.fp = NULL;
    }

    sc_log.prev_file = prev_file;
    sc_log.current_file = current_file;

    if (prev_file == NULL || current_file == NULL) {
        goto out;
    }

    fp = fopen(sc_log.current_file, "a+");
    if (fp == NULL || fprintf(fp, "\n") < 0 || (size = ftell(fp)) < 0) {
        goto error;
    }

    sc_log.file_size = (size_t) size;
    sc_log.fp = fp;

    goto out;

error:
    rc = -1;
    saved_errno = errno;

    if (fp != NULL) {
        fclose(fp);
    }
out:
    sc_log_mutex_unlock(&sc_log.mtx);
    errno = saved_errno;

    return rc;
}

void sc_log_set_callback(void *arg, int (*cb)(void *, enum sc_log_level,
                                              const char *, va_list))
{
    sc_log_mutex_lock(&sc_log.mtx);
    sc_log.arg = arg;
    sc_log.cb = cb;
    sc_log_mutex_unlock(&sc_log.mtx);
}

static int sc_log_print_header(FILE *fp, enum sc_log_level level)
{
    int rc;
    time_t t;
    struct tm result, *tm;

    t = time(NULL);
    tm = localtime_r(&t, &result);

    if (tm == NULL) {
        return -1;
    }

    rc = fprintf(fp, "[%d-%02d-%02d %02d:%02d:%02d][%-5s][%s] ",
                 tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour,
                 tm->tm_min, tm->tm_sec, sc_log_levels[level].str, sc_name);
    if (rc < 0) {
        return -1;
    }

    return 0;
}

static int sc_log_stdout(enum sc_log_level level, const char *fmt, va_list va)
{
    int rc;
    FILE *dest = level == SC_LOG_ERROR ? stderr : stdout;

    rc = sc_log_print_header(dest, level);
    if (rc < 0) {
        return -1;
    }

    rc = vfprintf(dest, fmt, va);
    if (rc < 0) {
        return -1;
    }

    return 0;
}

static int sc_log_file(enum sc_log_level level, const char *fmt, va_list va)
{
    int rc, size;

    rc = sc_log_print_header(sc_log.fp, level);
    if (rc < 0) {
        return -1;
    }

    size = vfprintf(sc_log.fp, fmt, va);
    if (size < 0) {
        return -1;
    }

    sc_log.file_size += size;

    if (sc_log.file_size > SC_LOG_FILE_SIZE) {
        fclose(sc_log.fp);
        (void) rename(sc_log.current_file, sc_log.prev_file);

        sc_log.fp = fopen(sc_log.current_file, "w+");
        if (sc_log.fp == NULL) {
            return -1;
        }

        sc_log.file_size = 0;
    }

    return rc;
}

int sc_log_log(enum sc_log_level level, const char *fmt, ...)
{
    int rc = 0;
    va_list va;

    // Use relaxed atomics to avoid locking cost, e.g DEBUG logs when
    // level=INFO will get away without any synchronization on most platforms.
#ifdef SC_ATOMIC
    enum sc_log_level curr;

    curr = sc_atomic_load(&sc_log.level);
    if (level < curr) {
        return 0;
    }
#endif

    sc_log_mutex_lock(&sc_log.mtx);

#ifndef SC_ATOMIC
    if (level < sc_log.level) {
        sc_log_mutex_unlock(&sc_log.mtx);
        return 0;
    }
#endif

    if (sc_log.to_stdout) {
        va_start(va, fmt);
        rc |= sc_log_stdout(level, fmt, va);
        va_end(va);
    }

    if (sc_log.fp != NULL) {
        va_start(va, fmt);
        rc |= sc_log_file(level, fmt, va);
        va_end(va);
    }

    if (sc_log.cb) {
        va_start(va, fmt);
        rc |= sc_log.cb(sc_log.arg, level, fmt, va);
        va_end(va);
    }

    sc_log_mutex_unlock(&sc_log.mtx);

    return rc;
}

/*
 * MIT License
 *
 * Copyright (c) 2020 Ozan Tezcan
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

#include "sc_log.h"

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)

    #pragma warning(disable : 4996)
    #define strcasecmp _stricmp

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
    pthread_mutexattr_t attr;
    pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

    mtx->mtx = mut;

    if (pthread_mutexattr_init(&attr) != 0 ||
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL) != 0 ||
        pthread_mutex_init(&mtx->mtx, &attr) != 0) {
        return -1;
    }

    pthread_mutexattr_destroy(&attr);
    return 0;
}

int sc_log_mutex_term(struct sc_log_mutex *mtx)
{
    return pthread_mutex_destroy(&mtx->mtx);
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
    enum sc_log_level level;

    bool to_stdout;
    sc_log_callback cb;
    void *arg;
};

struct sc_log sc_log;

int sc_log_init(void)
{
    int rc;

    rc = sc_log_mutex_init(&sc_log.mtx);
    if (rc != 0) {
        return -1;
    }

    sc_log.level = SC_LOG_INFO;
    sc_log.to_stdout = true;

    return 0;
}

int sc_log_term(void)
{
    int rc = 0;

    if (sc_log.fp) {
        rc = fclose(sc_log.fp);
    }

    sc_log_mutex_term(&sc_log.mtx);
    sc_log = (struct sc_log){0};

    return rc;
}

int sc_log_set_level(const char *str)
{
    size_t count = sizeof(sc_log_levels) / sizeof(struct sc_log_level_pair);

    for (size_t i = 0; i < count; i++) {
        if (strcasecmp(str, sc_log_levels[i].str) == 0) {
            sc_log_mutex_lock(&sc_log.mtx);
            sc_log.level = sc_log_levels[i].id;
            sc_log_mutex_unlock(&sc_log.mtx);

            return 0;
        }
    }

    return -1;
}

int sc_log_set_stdout(bool enable)
{
    sc_log_mutex_lock(&sc_log.mtx);
    sc_log.to_stdout = enable;
    sc_log_mutex_unlock(&sc_log.mtx);

    return 0;
}

int sc_log_set_file(const char *prev_file, const char *current_file)
{
    int rc = 0;
    long size;
    FILE *fp = NULL;

    sc_log_mutex_lock(&sc_log.mtx);

    if (sc_log.fp != NULL) {
        rc = fclose(sc_log.fp);
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

    sc_log.file_size = size;
    sc_log.fp = fp;

    goto out;

error:
    rc = -1;
    if (fp != NULL) {
        fclose(fp);
    }
out:
    sc_log_mutex_unlock(&sc_log.mtx);

    return rc;
}

int sc_log_set_callback(sc_log_callback cb, void *arg)
{
    sc_log_mutex_lock(&sc_log.mtx);
    sc_log.cb = cb;
    sc_log.arg = arg;
    sc_log_mutex_unlock(&sc_log.mtx);

    return 0;
}

static int sc_log_print_header(FILE *fp, enum sc_log_level level)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    if (tm == NULL) {
        fprintf(fp, "[ERROR] localtime() returns null! \n");
        return -1;
    }

    return fprintf(fp, "[%d-%02d-%02d %02d:%02d:%02d][%-5s] ",
                   tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour,
                   tm->tm_min, tm->tm_sec, sc_log_levels[level].str);
}

static int sc_log_stdout(enum sc_log_level level, const char *fmt, va_list va)
{
    int rc;

    rc = sc_log_print_header(stdout, level);
    if (rc < 0) {
        return -1;
    }

    return vfprintf(stdout, fmt, va);
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
            fprintf(stderr, "fopen() failed for [%s], (%s)\n",
                    sc_log.current_file, strerror(errno));
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

    sc_log_mutex_lock(&sc_log.mtx);

    if (level < sc_log.level) {
        sc_log_mutex_unlock(&sc_log.mtx);
        return 0;
    }

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

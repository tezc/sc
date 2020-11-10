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

#ifndef SC_LOG_H
#define SC_LOG_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum sc_log_level
{
    SC_LOG_DEBUG,
    SC_LOG_INFO,
    SC_LOG_WARN,
    SC_LOG_ERROR,
    SC_LOG_OFF,
};

// clang-format off
const static struct sc_log_level_pair
{
    const int id;
    const char *str;
} sc_log_levels[] = {
        {SC_LOG_DEBUG, "DEBUG"},
        {SC_LOG_INFO,  "INFO" },
        {SC_LOG_WARN,  "WARN" },
        {SC_LOG_ERROR, "ERROR"},
        {SC_LOG_OFF,   "OFF"  },
};
// clang-format on

/**
 * User callback
 *
 * @param arg   user provided data
 * @param level log level
 * @param fmt   format
 * @param va    arg list
 */
typedef int (*sc_log_callback)(void *arg, enum sc_log_level level,
                               const char *fmt, va_list va);

/**
 * Not thread-safe, should be called from a single thread.
 * @return '0' on success, negative value on error
 */
int sc_log_init(void);

/**
 * Not thread-safe, should be called from a single thread.
 * @return '0' on success, negative value on error
 */
int sc_log_term(void);

/**
 * Thread-safe.
 *
 * Valid values are 'DEBUG', 'INFO', 'WARN', 'ERROR', 'OFF'
 * @param level_str  level
 * @return           '0' on success, negative value on error
 */
int sc_log_set_level(const char *level_str);

/**
 * Thread-safe.
 *
 * @param enable 'true' to enable, 'false' will disable
 * @return        '0' on success, negative value on error
 */
int sc_log_set_stdout(bool enable);

/**
 * Thread-safe.
 *
 * Log file will be rotated. Logger will start writing into 'current_file',
 * when it grows larger than 'SC_LOG_FILE_SIZE', logger will rename
 * 'current_file' as 'prev_file' and create a new empty file at 'current_file'
 * again. So, latest logs will always be in the 'current_file'.
 *
 * e.g sc_log_set_file("/tmp/log.0.txt", "/tmp/log-latest.txt");
 *
 * To disable logging into file :
 *
 * sc_log_set_file(NULL, NULL);
 *
 * @param prev_file     file path for previous log file, 'NULL' to disable
 * @param current_file  file path for latest log file, 'NULL' to disable
 * @return
 */
int sc_log_set_file(const char *prev_file, const char *current_file);

/**
 * Thread-safe.
 * Logs can be reported to callback as well.
 *
 * @param cb  callback.
 * @param arg user arg.
 * @return    '0' on success, negative value on error
 */
int sc_log_set_callback(sc_log_callback cb, void *arg);

// Internal function
int sc_log_log(enum sc_log_level level, const char *fmt, ...);

/**
 * Max file size to rotate.
 */
#define SC_LOG_FILE_SIZE (2 * 1024 * 1024)

/**
 * Define SC_LOG_PRINT_FILE_NAME if you want to print file name and line number
 * in the log line.
 */
#ifdef SC_LOG_PRINT_FILE_NAME
    #define sc_log_ap(fmt, ...)                                                \
        "(%s:%d) " fmt "\n", strrchr("/" __FILE__, '/') + 1, __LINE__,         \
                __VA_ARGS__
#else
    #define sc_log_ap(fmt, ...) fmt "\n", __VA_ARGS__
#endif


/**
 * Printf-style format
 * e.g
 *    sc_log_error("Errno : %d, reason : %s", errno, strerror(errno));
 */
#define sc_log_debug(...) (sc_log_log(SC_LOG_DEBUG, sc_log_ap(__VA_ARGS__, "")))
#define sc_log_info(...)  (sc_log_log(SC_LOG_INFO, sc_log_ap(__VA_ARGS__, "")))
#define sc_log_warn(...)  (sc_log_log(SC_LOG_WARN, sc_log_ap(__VA_ARGS__, "")))
#define sc_log_error(...) (sc_log_log(SC_LOG_ERROR, sc_log_ap(__VA_ARGS__, "")))

#endif

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

#ifndef SC_COND_H
#define SC_COND_H

#include <stdbool.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#else
    #include <pthread.h>
#endif

struct sc_cond
{
    bool done;
    void* data;
    char err[64];

#if defined(_WIN32) || defined(_WIN64)
    CONDITION_VARIABLE cond;
    CRITICAL_SECTION mtx;
#else
    pthread_cond_t cond;
    pthread_mutex_t mtx;
#endif
};

/**
 * @param cond cond
 * @return     '0' on success, '-1' on error, use 'sc_cond_err' to get error msg
 */
int sc_cond_init(struct sc_cond* cond);

/**
 * @param cond cond
 * @return     '0' on success, '-1' on error, use 'sc_cond_err' to get error msg
 */
int sc_cond_term(struct sc_cond* cond);

/**
 * @param cond cond
 * @param data data to pass to thread which will call 'sc_cond_wait'.
 */
void sc_cond_signal(struct sc_cond* cond, void* data);

/**
 * @param cond cond
 * @return     'user data'.'data' argument on previous sc_cond_signal() call
 */
void* sc_cond_wait(struct sc_cond* cond);

/**
 * @param cond cond
 * @return     latest error message if any.
 */
const char* sc_cond_err(struct sc_cond *cond);

#endif

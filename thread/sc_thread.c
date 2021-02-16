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

#include "sc_thread.h"

#include <string.h>

void sc_thread_init(struct sc_thread *thread)
{
    thread->id = 0;
}

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable : 4996)

#include <process.h>

static void sc_thread_errstr(struct sc_thread *thread)
{
    int rc;
    DWORD err = GetLastError();
    LPSTR errstr = 0;

    rc = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL, err, 0, (LPSTR) &errstr, 0, NULL);
    if (rc != 0) {
        strncpy(thread->err, errstr, sizeof(thread->err) - 1);
        LocalFree(errstr);
    }
}

unsigned int __stdcall sc_thread_fn(void *arg)
{
    struct sc_thread *thread = arg;

    thread->ret = thread->fn(thread->arg);
    return 0;
}

int sc_thread_start(struct sc_thread *thread, void *(*fn)(void *), void *arg)
{
    int rc = 0;

    thread->fn = fn;
    thread->arg = arg;

    thread->id =
            (HANDLE) _beginthreadex(NULL, 0, sc_thread_fn, thread, 0, NULL);
    if (thread->id == 0) {
        sc_thread_errstr(thread);
        rc = -1;
    }

    return rc;
}

int sc_thread_join(struct sc_thread *thread, void **ret)
{
    int rc = 0;
    DWORD rv;
    BOOL brc;

    if (thread->id == 0) {
        strncpy(thread->err, "Already stopped.", sizeof(thread->err));
        return -1;
    }

    rv = WaitForSingleObject(thread->id, INFINITE);
    if (rv == WAIT_FAILED) {
        sc_thread_errstr(thread);
        rc = -1;
    }

    brc = CloseHandle(thread->id);
    if (!brc) {
        sc_thread_errstr(thread);
        rc = -1;
    }

    thread->id = 0;
    if (ret != NULL) {
        *ret = thread->ret;
    }

    return rc;
}
#else

int sc_thread_start(struct sc_thread *thread, void *(*fn)(void *), void *arg)
{
    int rc;
    pthread_attr_t attr;

    rc = pthread_attr_init(&attr);
    if (rc != 0) {
        strncpy(thread->err, strerror(rc), sizeof(thread->err) - 1);
        return -1;
    }

    // This may only fail with EINVAL.
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    rc = pthread_create(&thread->id, &attr, fn, arg);
    if (rc != 0) {
        strncpy(thread->err, strerror(rc), sizeof(thread->err) - 1);
    }

    // This may only fail with EINVAL.
    pthread_attr_destroy(&attr);

    return rc;
}

int sc_thread_join(struct sc_thread *thread, void **ret)
{
    int rc;
    void *val;

    if (thread->id == 0) {
        return -1;
    }

    rc = pthread_join(thread->id, &val);
    if (rc != 0) {
        strncpy(thread->err, strerror(rc), sizeof(thread->err) - 1);
    }

    thread->id = 0;

    if (ret != NULL) {
        *ret = val;
    }

    return rc;
}

#endif

int sc_thread_term(struct sc_thread *thread)
{
    return sc_thread_join(thread, NULL);
}

const char* sc_thread_err(struct sc_thread * thread)
{
    return thread->err;
}

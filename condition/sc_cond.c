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

#include "sc_cond.h"

#include <assert.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable : 4996)

static void sc_cond_errstr(struct sc_cond *cond)
{
    int rc;
    DWORD err = GetLastError();
    LPSTR errstr = 0;

    rc = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL, err, 0, (LPSTR) &errstr, 0, NULL);
    if (rc != 0) {
        strncpy(cond->err, errstr, sizeof(cond->err) - 1);
        LocalFree(errstr);
    }
}

int sc_cond_init(struct sc_cond *cond)
{
    *cond = (struct sc_cond){0};

    InitializeCriticalSection(&cond->mtx);
    InitializeConditionVariable(&cond->cond);

    return 0;
}

int sc_cond_term(struct sc_cond *cond)
{
    DeleteCriticalSection(&cond->mtx);

    return 0;
}

void sc_cond_finish(struct sc_cond *cond, void *var)
{
    EnterCriticalSection(&cond->mtx);

    cond->data = var;
    cond->done = true;

    WakeConditionVariable(&cond->cond);
    LeaveCriticalSection(&cond->mtx);
}

void* sc_cond_sync(struct sc_cond *cond)
{
    BOOL rc;
    void* data;

    EnterCriticalSection(&cond->mtx);

    while (cond->done == false) {
        // This should not fail as we pass INFINITE.
        rc = SleepConditionVariableCS(&cond->cond, &cond->mtx, INFINITE);
        assert(rc != 0);
    }

    data = cond->data;
    cond->data = NULL;
    cond->done = false;

    LeaveCriticalSection(&cond->mtx);

    return data;
}

#else


int sc_cond_init(struct sc_cond *cond)
{
    int rc;

    *cond = (struct sc_cond){0};

    pthread_mutexattr_t attr;
    pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

    cond->mtx = mut;

    rc = pthread_mutexattr_init(&attr);
    if (rc != 0) {
        goto error;
    }

    // This won't fail as long as we pass correct params.
    rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
    assert(rc == 0);

    rc = pthread_mutex_init(&cond->mtx, &attr);
    if (rc != 0) {
        goto cleanup_attr;
    }

    rc = pthread_cond_init(&cond->cond, NULL);
    if (rc != 0) {
        goto cleanup_mutex;
    }

    pthread_mutexattr_destroy(&attr);

    return 0;

cleanup_mutex:
    pthread_mutex_destroy(&cond->mtx);
cleanup_attr:
    pthread_mutexattr_destroy(&attr);
error:
    strncpy(cond->err, strerror(rc), sizeof(cond->err) - 1);
    return -1;
}

int sc_cond_term(struct sc_cond *cond)
{
    int rv, rc = 0;

    rv = pthread_mutex_destroy(&cond->mtx);
    if (rv != 0) {
        rc = -1;
        strncpy(cond->err, strerror(rv), sizeof(cond->err) - 1);
    }

    rv = pthread_cond_destroy(&cond->cond);
    if (rv != 0) {
        rc = -1;
        strncpy(cond->err, strerror(rv), sizeof(cond->err) - 1);
    }

    return rc;
}

void sc_cond_finish(struct sc_cond *cond, void *var)
{
    int rc;

    pthread_mutex_lock(&cond->mtx);

    cond->data = var;
    cond->done = true;

    // This won't fail as long as we pass correct params.
    rc = pthread_cond_signal(&cond->cond);
    assert(rc == 0);

    pthread_mutex_unlock(&cond->mtx);
}

void *sc_cond_sync(struct sc_cond *cond)
{
    int rc;
    void *data;

    pthread_mutex_lock(&cond->mtx);

    while (cond->done == false) {
        rc = pthread_cond_wait(&cond->cond, &cond->mtx);
        assert(rc == 0);
    }

    data = cond->data;
    cond->data = NULL;
    cond->done = false;

    pthread_mutex_unlock(&cond->mtx);

    return data;
}

#endif

const char *sc_cond_err(struct sc_cond *cond)
{
    return cond->err;
}

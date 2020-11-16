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

int sc_cond_init(struct sc_cond *cond)
{
    cond->data = NULL;
    cond->done = false;

    InitializeCriticalSection(&cond->mtx);
    InitializeConditionVariable(&cond->cond);

    return 0;
}

int sc_cond_term(struct sc_cond *cond)
{
    DeleteCriticalSection(&cond->mtx);

    return 0;
}

int sc_cond_finish(struct sc_cond *cond, void *var)
{
    EnterCriticalSection(&cond->mtx);

    cond->data = var;
    cond->done = true;

    WakeConditionVariable(&cond->cond);
    LeaveCriticalSection(&cond->mtx);

    return 0;
}

int sc_cond_sync(struct sc_cond *cond, void **data)
{
    int rc = 0;
    BOOL rv;

    EnterCriticalSection(&cond->mtx);

    while (cond->done == false) {
        rv = SleepConditionVariableCS(&cond->cond, &cond->mtx, INFINITE);
        if (rv == 0) {
            sc_cond_on_error("SleepConditionVariableCS: errcode(%d) ",
                             (int) GetLastError());
            rc = -1;
            goto out;
        }
    }

    if (data != NULL) {
        *data = cond->data;
    }

    cond->data = NULL;
    cond->done = false;

out:
    LeaveCriticalSection(&cond->mtx);

    return rc;
}

#else


int sc_cond_init(struct sc_cond *cond)
{
    int rc;

    cond->data = NULL;
    cond->done = false;

    pthread_mutexattr_t attr;
    pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

    cond->mtx = mut;

    // May fail on OOM
    rc = pthread_mutexattr_init(&attr);
    if (rc != 0) {
        sc_cond_on_error("pthread_mutexattr_init : errno(%d) \n", rc);
        return rc;
    }

    // This won't fail as long as we pass correct params.
    rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
    assert(rc == 0);

    // May fail on OOM
    rc = pthread_mutex_init(&cond->mtx, &attr);
    if (rc != 0) {
        sc_cond_on_error("pthread_mutex_init : errno(%d) \n", rc);
        return -1;
    }

    // This won't fail as long as we pass correct param.
    pthread_mutexattr_destroy(&attr);

    return pthread_cond_init(&cond->cond, NULL);
}

int sc_cond_term(struct sc_cond *cond)
{
    int rc;

    rc = pthread_mutex_destroy(&cond->mtx);
    rc |= pthread_cond_destroy(&cond->cond);

    return rc;
}

int sc_cond_finish(struct sc_cond *cond, void *var)
{
    int rc, rv;

    rv = pthread_mutex_lock(&cond->mtx);
    assert(rv == 0);

    cond->data = var;
    cond->done = true;

    rc = pthread_cond_signal(&cond->cond);
    if (rc != 0) {
        sc_cond_on_error("pthread_cond_signal : errno(%d) \n", rc);
    }
    rv = pthread_mutex_unlock(&cond->mtx);
    assert(rv == 0);

    return rc;
}

int sc_cond_sync(struct sc_cond *cond, void **data)
{
    int rc, rv;

    rv = pthread_mutex_lock(&cond->mtx);
    assert(rv == 0);

    while (cond->done == false) {
        rc = pthread_cond_wait(&cond->cond, &cond->mtx);
        if (rc != 0) {
            sc_cond_on_error("pthread_mutex_init : errno(%d) \n", rc);
            goto out;
        }
    }

    if (data != NULL) {
        *data = cond->data;
    }

    cond->data = NULL;
    cond->done = false;

out:
    rv = pthread_mutex_unlock(&cond->mtx);
    assert(rv == 0);

    return rc;
}

#endif

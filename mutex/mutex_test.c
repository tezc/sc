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

#include "sc_mutex.h"

#include <assert.h>
#include <stdbool.h>

#ifdef SC_HAVE_WRAP
bool mock_attrinit = false;
extern int __real_pthread_mutexattr_init(pthread_mutexattr_t *attr);
int __wrap_pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
    if (!mock_attrinit) {
        return __real_pthread_mutexattr_init(attr);
    }

    return -1;
}

bool mock_mutexdestroy = false;
extern int __real_pthread_mutex_destroy(pthread_mutex_t *m);
int __wrap_pthread_mutex_destroy(pthread_mutex_t *m)
{
    int rc;

    rc = __real_pthread_mutex_destroy(m);

    return mock_mutexdestroy ? -1 : rc;
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

#endif

int main(int argc, char *argv[])
{
    struct sc_mutex mutex;
#ifdef SC_HAVE_WRAP
    mock_attrinit = true;
    assert(sc_mutex_init(&mutex) != 0);
    mock_attrinit = false;
    assert(sc_mutex_init(&mutex) == 0);
    mock_mutexdestroy = true;
    assert(sc_mutex_term(&mutex) == -1);
    mock_mutexdestroy = false;
    mock_mutexinit = true;
    assert(sc_mutex_init(&mutex) != 0);
    mock_mutexinit = false;
    assert(sc_mutex_term(&mutex) == 0);

#endif
    assert(sc_mutex_init(&mutex) == 0);
    sc_mutex_lock(&mutex);
    sc_mutex_unlock(&mutex);
    assert(sc_mutex_term(&mutex) == 0);

    return 0;
}

#include "sc_cond.h"
#include <string.h>
#include <assert.h>

#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>

struct sc_thread
{
    HANDLE id;
    void* (*fn)(void*);
    void* arg;
    void* ret;
};

#else

#include <pthread.h>
#include <unistd.h>

struct sc_thread
{
    pthread_t id;
};

#endif

void sc_thread_init(struct sc_thread* thread);
int sc_thread_term(struct sc_thread* thread);
int sc_thread_start(struct sc_thread* thread, void* (*fn)(void*), void* arg);
int sc_thread_stop(struct sc_thread* thread, void** ret);

void sc_thread_init(struct sc_thread* thread)
{
    thread->id = 0;
}

#if defined(_WIN32) || defined(_WIN64)
#include <process.h>

unsigned int __stdcall sc_thread_fn(void* arg)
{
    struct sc_thread* thread = arg;

    thread->ret = thread->fn(thread->arg);
    return 0;
}

int sc_thread_start(struct sc_thread* thread, void* (*fn)(void*), void* arg)
{
    int rc;

    thread->fn = fn;
    thread->arg = arg;

    thread->id = (HANDLE)_beginthreadex(NULL, 0, sc_thread_fn, thread, 0, NULL);
    rc = thread->id == 0 ? -1 : 0;

    return rc;
}

int sc_thread_stop(struct sc_thread* thread, void** ret)
{
    int rc = 0;
    DWORD rv;
    BOOL brc;

    if (thread->id == 0) {
        return -1;
    }

    rv = WaitForSingleObject(thread->id, INFINITE);
    if (rv == WAIT_FAILED) {
        rc = -1;
    }

    brc = CloseHandle(thread->id);
    if (!brc) {
        rc = -1;
    }

    thread->id = 0;
    if (ret != NULL) {
        *ret = thread->ret;
    }

    return rc;
}
#else

int sc_thread_start(struct sc_thread* thread, void* (*fn)(void*), void* arg)
{
    int rc;
    pthread_attr_t hndl;

    rc = pthread_attr_init(&hndl);
    if (rc != 0) {
        return -1;
    }

    // This may only fail with EINVAL.
    pthread_attr_setdetachstate(&hndl, PTHREAD_CREATE_JOINABLE);

    rc = pthread_create(&thread->id, &hndl, fn, arg);

    // This may only fail with EINVAL.
    pthread_attr_destroy(&hndl);

    return rc;
}

int sc_thread_stop(struct sc_thread* thread, void** ret)
{
    int rc;
    void* val;

    if (thread->id == 0) {
        return -1;
    }

    rc = pthread_join(thread->id, &val);
    thread->id = 0;

    if (ret != NULL) {
        *ret = val;
    }

    return rc;
}

#endif

int sc_thread_term(struct sc_thread* thread)
{
    return sc_thread_stop(thread, NULL);
}

void* thread1_fn(void* arg)
{
    char* data;
    struct sc_cond* cond = arg;

    assert(sc_cond_sync(cond, (void**)&data) == 0);
    assert(strcmp(data, "finish") == 0);

    return NULL;
}

void* thread2_fn(void* arg)
{
    struct sc_cond* cond = arg;
    assert(sc_cond_finish(cond, "finish") == 0);
    return NULL;
}

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

bool mock_condwait = false;
extern int __real_pthread_cond_wait (pthread_cond_t *__restrict __cond,
                                     pthread_mutex_t *__restrict __mutex);
int __wrap_pthread_cond_wait (pthread_cond_t *__restrict __cond,
                              pthread_mutex_t *__restrict __mutex)
{
    if (!mock_condwait) {
        return __real_pthread_cond_wait(__cond, __mutex);
    }

    return -1;
}

void fail_test()
{
    struct sc_cond cond;
    char* var;

    mock_attrinit = true;
    assert(sc_cond_init(&cond) == -1);
    mock_attrinit = false;
    assert(sc_cond_init(&cond) == 0);
    assert(sc_cond_term(&cond) == 0);
    mock_mutexinit = true;
    assert(sc_cond_init(&cond) == -1);
    mock_mutexinit = false;
    assert(sc_cond_init(&cond) == 0);
    assert(sc_cond_term(&cond) == 0);

    assert(sc_cond_init(&cond) == 0);
    mock_condwait = true;
    assert(sc_cond_sync(&cond, (void**) &var) == -1);
    mock_condwait = false;
    assert(sc_cond_term(&cond) == 0);
}

#else
void fail_test()
{

}
#endif

void test1()
{
    struct sc_cond cond;
    struct sc_thread thread1;
    struct sc_thread thread2;

    assert(sc_cond_init(&cond) == 0);

    sc_thread_init(&thread1);
    sc_thread_init(&thread2);

    assert(sc_thread_start(&thread1, thread1_fn, &cond) == 0);
    assert(sc_thread_start(&thread2, thread2_fn, &cond) == 0);

    assert(sc_thread_term(&thread1) == 0);
    assert(sc_thread_term(&thread2) == 0);

    assert(sc_cond_term(&cond) == 0);
}


int main()
{
    test1();
    fail_test();

    return 0;
}

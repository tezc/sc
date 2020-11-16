#include "sc_thread.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

void* fn(void* arg)
{
    printf("%s \n", (char*) arg);
    return arg;
}

void test1()
{
    int rc;
    void* ret;
    struct sc_thread thread;

    sc_thread_init(&thread);
    rc = sc_thread_start(&thread, fn, "first");
    assert(rc == 0);

    rc = sc_thread_stop(&thread, &ret);
    assert(rc == 0);
    assert(strcmp((char*)ret, "first") == 0);

    rc = sc_thread_term(&thread);
    assert(rc == -1);

    sc_thread_init(&thread);
    rc = sc_thread_start(&thread, fn, "first");
    assert(rc == 0);
    rc = sc_thread_term(&thread);
    assert(rc == 0);
}

#ifdef SC_HAVE_WRAP
bool fail_pthread_attr_init = false;
int __real_pthread_attr_init (pthread_attr_t *__attr);
int __wrap_pthread_attr_init (pthread_attr_t *__attr)
{
    if (fail_pthread_attr_init) {
        return -1;
    }

    return __real_pthread_attr_init(__attr);
}
void fail_test()
{
    struct sc_thread thread;

    sc_thread_init(&thread);
    fail_pthread_attr_init = true;
    assert(sc_thread_start(&thread, fn, "first") != 0);
    fail_pthread_attr_init = false;
    assert(sc_thread_start(&thread, fn, "first") == 0);
    assert(sc_thread_term(&thread) == 0);
}

#else
void fail_test()
{

}
#endif

int main(int argc, char *argv[])
{
    test1();
    fail_test();
    return 0;
}

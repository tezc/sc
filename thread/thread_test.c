#include "sc_thread.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

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
    assert(rc == 0);
}

int main(int argc, char *argv[])
{
    test1();
    return 0;
}

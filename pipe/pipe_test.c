#include "sc_pipe.h"

#include <assert.h>
#include <string.h>
#include <stdbool.h>

#ifdef SC_HAVE_WRAP
#include <unistd.h>

bool fail_close = false;
int __real_close(int fd);
int __wrap_close(int fd)
{
    if (fail_close) {
        return -1;
    }

    return __real_close(fd);
}

bool fail_pipe = false;
int __real_pipe(int __pipedes[2]);
int __wrap_pipe(int __pipedes[2])
{
    if (fail_pipe) {
        return -1;
    }

    return __real_pipe(__pipedes);
}

void fail_test()
{
    struct sc_pipe pipe;
    fail_pipe = true;
    assert(sc_pipe_init(&pipe, 0) != 0);
    fail_pipe = false;
    assert(sc_pipe_init(&pipe, 0) == 0);
    fail_close = true;
    assert(sc_pipe_term(&pipe) == -1);
    fail_close = false;
    assert(sc_pipe_term(&pipe) == 0);
}

#else
void fail_test()
{

}

#endif

void test1(void)
{
    char buf[5];
    struct sc_pipe pipe;

    sc_pipe_init(&pipe, 0);
    sc_pipe_write(&pipe, "test", 5);
    sc_pipe_read(&pipe, buf, 5);
    sc_pipe_term(&pipe);

    assert(strcmp("test", buf) == 0);
}

int main(int argc, char* argv[])
{
#if defined(_WIN32) || defined(_WIN64)
    WSADATA data;

    int rc = WSAStartup(MAKEWORD(2, 2), &data);
    assert(rc == 0);
    assert(LOBYTE(data.wVersion) == 2 &&
        HIBYTE(data.wVersion) == 2);
#endif
    test1();
    fail_test();

#if defined(_WIN32) || defined(_WIN64)
    rc = WSACleanup();
    assert(rc == 0);
#endif

    return 0;
}

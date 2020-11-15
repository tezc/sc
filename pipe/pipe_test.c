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

#include "sc_pipe.h"

#include <assert.h>
#include <string.h>
#include <stdbool.h>

#ifdef SC_HAVE_WRAP
#include <unistd.h>

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

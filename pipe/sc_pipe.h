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
#ifndef SC_PIPE_H
#define SC_PIPE_H

#include <stdint.h>
#include <stddef.h>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")


struct sc_pipe
{
    int type;
    SOCKET r;
    SOCKET w;
};

#else
struct sc_pipe
{
    int type;
    int fds[2];
};
#endif

int sc_pipe_init(struct sc_pipe *pipe, int type);
int sc_pipe_term(struct sc_pipe *pipe);
int sc_pipe_write(struct sc_pipe *pipe, void *data, int len);
int sc_pipe_read(struct sc_pipe *pipe, void *data, int len);

/**
* If you want to log or abort on errors like mutex init,
* put your error function here. It will be called with printf like error msg.
*
* my_on_error(const char* fmt, ...);
*/
#define sc_pipe_on_error(...)

#endif

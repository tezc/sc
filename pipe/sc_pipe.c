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

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "sc_pipe.h"


#if defined(_WIN32) || defined(_WIN64)

int sc_pipe_init(struct sc_pipe* p)
{
    SOCKET listener;
    int rc;
    struct sockaddr_in addr;
    size_t addrlen;
    int one;
    BOOL nodelay;
    u_long nonblock;

    p->w = INVALID_SOCKET;
    p->r = INVALID_SOCKET;

    /*  Create listening socket. */
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == SOCKET_ERROR) {
        goto wsafail;
    }

    one = 1;
    rc = setsockopt(listener, SOL_SOCKET, SO_EXCLUSIVEADDRUSE,
        (char*)&one, sizeof(one));
    if (rc == SOCKET_ERROR) {
        goto wsafail;
    }

    /*  Bind the listening socket to the local port. */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;

    rc = bind(listener, (const struct sockaddr*)&addr, sizeof(addr));
    if (rc == SOCKET_ERROR) {
        goto wsafail;
    }

    addrlen = sizeof(addr);
    rc = getsockname(listener, (struct sockaddr*)&addr, &addrlen);
    if (rc == SOCKET_ERROR) {
        goto wsafail;
    }

    rc = listen(listener, 1);
    if (rc == SOCKET_ERROR) {
        goto wsafail;
    }

    p->w = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == SOCKET_ERROR) {
        goto wsafail;
    }

    /*  Set TCP_NODELAY on the writer socket to make efd as fast as possible.
        There's only one byte going to be written, so batching would not make
        sense anyway. */
    nodelay = 1;
    rc = setsockopt(p->w, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay,
        sizeof(nodelay));
    if (rc == SOCKET_ERROR) {
        goto wsafail;
    }

    rc = connect(p->w, (struct sockaddr*)&addr, sizeof(addr));
    if (rc == SOCKET_ERROR) {
        goto wsafail;
    }


    addrlen = sizeof(addr);
    p->r = accept(listener, (struct sockaddr*)&addr, &addrlen);
    if (p->r == INVALID_SOCKET) {
        goto wsafail;
    }

    (void)closesocket(listener);

    return 0;

wsafail:
    return -1;
}

int sc_pipe_term(struct sc_pipe* p)
{
    int rc;
    SOCKET s;

    rc = closesocket(p->r);
    rc |= closesocket(p->w);

    if (rc != 0) {
        sc_pipe_abort();
    }

    return rc;
}

int sc_pipe_write(struct sc_pipe* p, void* data, int len)
{
    int rc;

    rc = send(p->w, data, len, 0);
    if (rc == SOCKET_ERROR || rc != len) {
        sc_pipe_abort();
    }

    return rc;
}

int sc_pipe_read(struct sc_pipe* p, void* data, int len)
{
    int rc;

    rc = recv(p->r, (char*)data, len, 0);
    if (rc == SOCKET_ERROR || rc != len) {
        sc_pipe_abort();
    }

    return rc;
}

#else

#include <unistd.h>

int sc_pipe_init(struct sc_pipe *p, int  type)
{
    int rc;

    rc = pipe(p->fds);
    if (rc == -1) {
        sc_pipe_abort();
        return -1;
    }

    p->type = type;

    return 0;
}

int sc_pipe_term(struct sc_pipe *nfd)
{
    int rc;

    rc = close(nfd->fds[0]);
    rc |= close(nfd->fds[1]);

    return rc;
}

int sc_pipe_write(struct sc_pipe *nfd, void *data, int len)
{
    ssize_t n;

    n = write(nfd->fds[1], data, len);
    if (n != len) {
        sc_pipe_abort();
    }

    return n;
}

int sc_pipe_read(struct sc_pipe *nfd, void *data, int len)
{
    ssize_t n;

    n = read(nfd->fds[0], data, len);
    if (n == -1) {
        sc_pipe_abort();
    }

    return n;
}

#endif

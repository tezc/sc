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

#ifndef SC_POLL_H
#define SC_POLL_H

#include <stdlib.h>

#if  defined(__linux__)
#include <sys/epoll.h>

struct sc_poll
{
    int fds;
    size_t fd_count;
    size_t fd_cap;
    struct epoll_event *events;
};

#elif defined(__APPLE__) || defined(__FREE_BSD__) 
#include <sys/event.h>

struct sc_poll
{
    int fds;
    size_t fd_count;
    size_t fd_cap;
    struct kevent *events;
};

#else


#endif

#define sc_poll_malloc  malloc
#define sc_poll_realloc realloc
#define sc_poll_free    free
#define sc_poll_on_error(...)

#define SC_POLL_READ 1u
#define SC_POLL_WRITE 2u

int sc_poll_init(struct sc_poll *poll);
int sc_poll_term(struct sc_poll *poll);

int sc_poll_add_fd(struct sc_poll *poll, int fd, int events, void *data);
int sc_poll_mod_fd(struct sc_poll *poll, int fd, int events, void *data);
int sc_poll_del_fd(struct sc_poll *poll, int fd);

void *sc_poll_data(struct sc_poll *poll, int i);
uint32_t sc_poll_event(struct sc_poll *poll, int i);
int sc_poll_wait(struct sc_poll *poll, int timeout);

#endif

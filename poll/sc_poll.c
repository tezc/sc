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

#include "sc_poll.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__linux__)

int sc_poll_init(struct sc_poll *poll)
{
    int fds;

    *poll = (struct sc_poll){0};

    fds = epoll_create1(0);
    if (fds == -1) {
        sc_poll_on_error("epoll_create1(): %s ", strerror(errno));
        return -1;
    }

    poll->fds = fds;

    return 0;
}

int sc_poll_term(struct sc_poll *poll)
{
    sc_poll_free(poll->events);
    return close(poll->fds);
}

int sc_poll_add_fd(struct sc_poll *poll, int fd, int events, void *data)
{
    int rc;
    size_t size, cap;
    void *p;
    struct epoll_event ep_ev = {.data.ptr = data,
                                .events = EPOLLERR | EPOLLHUP | EPOLLRDHUP};

    if (poll->fd_count == poll->fd_cap) {
        cap = poll->fd_cap == 0 ? 16 : poll->fd_cap * 2;
        size = sizeof(struct epoll_event) * cap;

        p = sc_poll_realloc(poll->events, size);
        if (p == NULL) {
            sc_poll_on_error("Out of memory. size : %s ", size);
            return -1;
        }

        poll->events = p;
        poll->fd_cap = cap;
    }

    if (events & SC_POLL_WRITE) {
        ep_ev.events |= EPOLLOUT;
    }

    if (events & SC_POLL_READ) {
        ep_ev.events |= EPOLLIN;
    }

    rc = epoll_ctl(poll->fds, EPOLL_CTL_ADD, fd, &ep_ev);
    if (rc != 0) {
        sc_poll_on_error("epoll_ctl : %s ", strerror(errno));
        return -1;
    }

    poll->fd_count++;

    return 0;
}

int sc_poll_mod_fd(struct sc_poll *poll, int fd, int events, void *data)
{
    int rc;
    struct epoll_event ep_ev = {.data.ptr = data,
                                .events = EPOLLERR | EPOLLHUP | EPOLLRDHUP};

    if ((events & (SC_POLL_READ | SC_POLL_WRITE)) == 0) {
        return 0;
    }

    if (events & SC_POLL_WRITE) {
        ep_ev.events |= EPOLLOUT;
    }

    if (events & SC_POLL_READ) {
        ep_ev.events |= EPOLLIN;
    }

    rc = epoll_ctl(poll->fds, EPOLL_CTL_MOD, fd, &ep_ev);
    if (rc != 0) {
        sc_poll_on_error("epoll_ctl : %s ", strerror(errno));
    }

    return rc;
}

int sc_poll_del_fd(struct sc_poll *poll, int fd, int events)
{
    int rc;

    if ((events & (SC_POLL_READ | SC_POLL_WRITE)) == 0) {
        return 0;
    }

    rc = epoll_ctl(poll->fds, EPOLL_CTL_DEL, fd, NULL);
    if (rc != 0) {
        sc_poll_on_error("epoll_ctl : %s ", strerror(errno));
        return -1;
    }

    poll->fd_count--;

    return 0;
}

void *sc_poll_data(struct sc_poll *poll, int i)
{
    return poll->events[i].data.ptr;
}

uint32_t sc_poll_event(struct sc_poll *poll, int i)
{
    uint32_t events = 0;
    uint32_t epoll_events = poll->events[i].events;

    if (epoll_events & EPOLLIN) {
        events |= SC_POLL_READ;
    }

    if (epoll_events & EPOLLOUT) {
        events |= SC_POLL_WRITE;
    }

    epoll_events &= EPOLLHUP | EPOLLRDHUP | EPOLLERR;
    if (epoll_events != 0) {
        events = (SC_POLL_READ | SC_POLL_WRITE);
    }

    return events;
}

int sc_poll_wait(struct sc_poll *poll, int timeout)
{
    int n;

    do {
        n = epoll_wait(poll->fds, &poll->events[0], poll->fd_cap, timeout);
    } while (n < 0 && errno == EINTR);

    if (n == -1) {
        sc_poll_on_error("epoll_wait : %s ", strerror(errno));
    }

    return n;
}

#elif defined(__APPLE__) || defined(__FreeBSD__)
int sc_poll_init(struct sc_poll *poll)
{
    int fds;

    *poll = (struct sc_poll){0};

    fds = kqueue();
    if (fds == -1) {
        sc_poll_on_error("kqueue(): %s ", strerror(errno));
        return -1;
    }

    poll->fds = fds;

    return 0;
}

int sc_poll_term(struct sc_poll *poll)
{
    sc_poll_free(poll->events);
    return close(poll->fds);
}

int sc_poll_add_fd(struct sc_poll *poll, int fd, int events, void *data)
{
    int rc, count = 0;
    size_t cap, size;
    void* p;
    struct kevent ev[2];

    if (poll->fd_count == poll->fd_cap) {
        cap = poll->fd_cap == 0 ? 16 : poll->fd_cap * 2;
        size = sizeof(struct kevent) * cap;

        p = sc_poll_realloc(poll->events, size);
        if (p == NULL) {
            sc_poll_on_error("Out of memory. size : %s ", size);
            return -1;
        }

        poll->events = p;
        poll->fd_cap = cap;
    }

    if (events & SC_POLL_WRITE) {
        EV_SET(&ev[count++], fd, EVFILT_WRITE, EV_ADD, 0, 0, data);
    }

    if (events & SC_POLL_READ) {
        EV_SET(&ev[count++], fd, EVFILT_READ, EV_ADD, 0, 0, data);
    }

    rc = kevent(poll->fds, ev, count, NULL, 0, NULL);
    if (rc != 0) {
        sc_poll_on_error("kevent : %s ", strerror(errno));
        return -1;
    }

    poll->fd_count++;

    return 0;
}

int sc_poll_mod_fd(struct sc_poll *poll, int fd, int events, void *data)
{
    int rc, count = 0;
    struct kevent ev[4];

    EV_SET(&ev[count++], fd, EVFILT_READ, EV_DELETE, 0, 0, 0);
    EV_SET(&ev[count++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, 0);

    if (events & SC_POLL_READ) {
        EV_SET(&ev[count++], fd, EVFILT_READ, EV_ADD, 0, 0, data);
    }

    if (events & SC_POLL_WRITE) {
        EV_SET(&ev[count++], fd, EVFILT_WRITE, EV_ADD, 0, 0, data);
    }

    rc = kevent(poll->fds, ev, count, NULL, 0, NULL);
    if (rc != 0) {
        sc_poll_on_error("kevent : %s ", strerror(errno));
        return -1;
    }

    return 0;
}

int sc_poll_del_fd(struct sc_poll *poll, int fd, int events)
{
    int rc, count = 0;
    struct kevent ev[2];
    int i;

    if ((events & (SC_POLL_READ | SC_POLL_WRITE)) == 0) {
        return 0;
    }

    if (events & SC_POLL_READ) {
        EV_SET(&ev[count++], fd, EVFILT_READ, EV_DELETE, 0, 0, 0);
    }

    if (events & SC_POLL_WRITE) {
        EV_SET(&ev[count++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
    }

    rc = kevent(poll->fds, ev, count, NULL, 0, NULL);
    if (rc != 0) {
        sc_poll_on_error("kevent : %s ", strerror(errno));
        return -1;
    }

    poll->fd_count--;

    return 0;
}

void *sc_poll_data(struct sc_poll *poll, int i)
{
    return poll->events[i].udata;
}

uint32_t sc_poll_event(struct sc_poll *poll, int i)
{
    uint32_t events = 0;

    if (poll->events[i].flags & EV_EOF) {
        events = (SC_POLL_READ | SC_POLL_WRITE);
    } else if (poll->events[i].filter == EVFILT_READ) {
        events |= SC_POLL_READ;
    } else if (poll->events[i].filter == EVFILT_WRITE) {
        events |= SC_POLL_WRITE;
    }

    return events;
}

int sc_poll_wait(struct sc_poll *poll, int timeout)
{
    int n;
    struct timespec ts;

    do {
        ts.tv_sec = timeout / 1000;
        ts.tv_nsec = (timeout % 1000) * 1000000;

        n = kevent(poll->fds, NULL, 0, &poll->events[0], poll->fd_cap,
                   timeout >= 0 ? &ts : NULL);
    } while (n < 0 && errno == EINTR);

    if (n == -1) {
        sc_poll_on_error("kevent : %s ", strerror(errno));
    }

    return n;
}

#endif

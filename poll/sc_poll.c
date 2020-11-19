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

#if  defined(__linux__)

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

int sc_poll_del_fd(struct sc_poll *poll, int fd)
{
    int rc;

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
    char err[128];

    do {
        n = epoll_wait(poll->fds, &poll->events[0], poll->fd_cap, timeout);
    } while (n < 0 && errno == EINTR);

    if (n == -1) {
        sc_poll_on_error("epoll_wait : %s ", strerror(errno));
    }

    return n;
}

#elif defined(__APPLE__) || defined(__FREE_BSD__)
void sc_poll_init(struct sc_poll *poll)
{
    int fds;

    fds = kqueue();
    if (fds == -1) {
        sc_proc_abort();
    }

    poll->fds = fds;
    poll->fd_count = 0;
    poll->fd_cap = 16;
    poll->events = sc_mem_alloc(poll->fd_cap * sizeof(struct kevent));

    sc_thread_update_timestamp();
}

void sc_poll_term(struct sc_poll *poll)
{
    sc_mem_free(poll->events);
    sc_fd_close(poll->fds);
}

void sc_poll_register_fd(struct sc_poll *poll, struct sc_fd *nfd,
                         enum sc_fd_op events)
{
    int rc;
    struct kevent ev;

    nfd->op = events;

    if (events & sc_FD_WRITE) {
        EV_SET(&ev, nfd->fd, EVFILT_WRITE, EV_ADD, 0, 0, nfd);
        rc = kevent(poll->fds, &ev, 1, NULL, 0, NULL);
        if (rc != 0) {
            sc_proc_abort();
        }
    }

    if (events & sc_FD_READ) {
        EV_SET(&ev, nfd->fd, EVFILT_READ, EV_ADD, 0, 0, nfd);
        rc = kevent(poll->fds, &ev, 1, NULL, 0, NULL);
        if (rc != 0) {
            sc_proc_abort();
        }
    }

    if (poll->fd_count == poll->fd_cap) {
        poll->fd_cap *= 2;
        poll->events = sc_mem_realloc(poll->events,
                                      poll->fd_cap * sizeof(struct kevent));
    }

    poll->fd_count++;
}

void sc_poll_unregister_fd(struct sc_poll *poll, struct sc_fd *nfd)
{
    int rc;
    struct kevent ev;
    int i;

    if (nfd->op & sc_FD_READ) {
        EV_SET(&ev, nfd->fd, EVFILT_READ, EV_DELETE, 0, 0, 0);
        rc = kevent(poll->fds, &ev, 1, NULL, 0, NULL);
        if (rc != 0) {
            sc_proc_abort();
        }
    }

    if (nfd->op & sc_FD_WRITE) {
        EV_SET(&ev, nfd->fd, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
        rc = kevent(poll->fds, &ev, 1, NULL, 0, NULL);
        if (rc != 0) {
            sc_proc_abort();
        }
    }

    nfd->op = sc_FD_UNREGISTERED;
    poll->fd_count--;
}

void sc_poll_add_event(struct sc_poll *poll, struct sc_fd *nfd,
                       enum sc_fd_op event)
{
    int rc;
    struct kevent ev;

    if ((nfd->op & event)) {
        return;
    }

    nfd->op |= event;

    if (event & sc_FD_WRITE) {
        EV_SET(&ev, nfd->fd, EVFILT_WRITE, EV_ADD, 0, 0, nfd);
        rc = kevent(poll->fds, &ev, 1, NULL, 0, NULL);
        if (rc != 0) {
            sc_proc_abort();
        }
    }

    if (event & sc_FD_READ) {
        EV_SET(&ev, nfd->fd, EVFILT_READ, EV_ADD, 0, 0, nfd);
        rc = kevent(poll->fds, &ev, 1, NULL, 0, NULL);
        if (rc != 0) {
            sc_proc_abort();
        }
    }

    if (poll->fd_count == poll->fd_cap) {
        poll->fd_cap *= 2;
        poll->events = sc_mem_realloc(poll->events,
                                      poll->fd_cap * sizeof(struct kevent));
    }

    poll->fd_count++;
}

void sc_poll_del_event(struct sc_poll *poll, struct sc_fd *nfd,
                       enum sc_fd_op event)
{
    int rc;
    struct kevent ev;

    if (!(nfd->op & event)) {
        return;
    }

    nfd->op &= ~event;

    if (event & sc_FD_READ) {
        EV_SET(&ev, nfd->fd, EVFILT_READ, EV_DELETE, 0, 0, 0);
        rc = kevent(poll->fds, &ev, 1, NULL, 0, NULL);
        if (rc != 0) {
            sc_proc_abort();
        }
    }

    if (event & sc_FD_WRITE) {
        EV_SET(&ev, nfd->fd, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
        rc = kevent(poll->fds, &ev, 1, NULL, 0, NULL);
        if (rc != 0) {
            sc_proc_abort();
        }
    }

    if (poll->fd_count == poll->fd_cap) {
        poll->fd_cap *= 2;
        poll->events = sc_mem_realloc(poll->events,
                                      poll->fd_cap * sizeof(struct kevent));
    }

    poll->fd_count++;
}

void *sc_poll_data(struct sc_poll *poll, int i)
{
    return poll->events[i].udata;
}

uint32_t sc_poll_event(struct sc_poll *poll, int i)
{
    uint32_t events = 0;
    uint32_t kqueue_flags = poll->events[i].flags;

    if (kqueue_flags & EVFILT_READ) {
        events |= sc_FD_READ;
    }

    if (kqueue_flags & EVFILT_WRITE) {
        events |= sc_FD_WRITE;
    }

    if (kqueue_flags & EV_EOF) {
        events = (FD_READ | sc_FD_WRITE);
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
        sc_proc_abort();
    }

    return n;
}

#endif

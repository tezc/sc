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
#ifndef SC_SOCK_H
#define SC_SOCK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64)
#include <Ws2tcpip.h>
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

typedef SOCKET sc_sock_int;

#else
#include <sys/socket.h>
typedef int sc_sock_int;
#endif

#define SC_SOCK_BUF_SIZE 8192


enum sc_sock_rc
{
    SC_SOCK_WANT_READ = -4,
    SC_SOCK_WANT_WRITE = -2,
    SC_SOCK_ERROR = -1,
    SC_SOCK_OK = 0
};

enum sc_sock_ev
{
    SC_SOCK_NONE = 0u,
    SC_SOCK_READ = 1u,
    SC_SOCK_WRITE = 2u,
};

enum sc_sock_family
{
    SC_SOCK_INET = AF_INET,
    SC_SOCK_INET6 = AF_INET6,
    SC_SOCK_UNIX = AF_UNIX
};

struct sc_sock_fd
{
    sc_sock_int fd;
    enum sc_sock_ev op;
    int type;
    int index;
};

struct sc_sock
{
    struct sc_sock_fd fdt;
    bool blocking;
    int family;
    char err[128];
};

void sc_sock_init(struct sc_sock* sock, int type, bool blocking, int family);
int sc_sock_term(struct sc_sock* sock);

int sc_sock_listen(struct sc_sock* sock, const char* host, const char* port);
int sc_sock_accept(struct sc_sock* sock, struct sc_sock* in);

int sc_sock_connect(struct sc_sock* sock, const char* dest_addr,
                    const char* dest_port, const char* source_addr,
                    const char* source_port);

int sc_sock_finish_connect(struct sc_sock* sock);

int sc_sock_send(struct sc_sock* sock, char* buf, int len);
int sc_sock_recv(struct sc_sock* sock, char* buf, int len);

const char* sc_sock_error(struct sc_sock* sock);
void sc_sock_print(struct sc_sock* sock, char* buf, int len);


struct sc_sock_pipe
{
    struct sc_sock_fd fdt;
    sc_sock_int fds[2];
};

int sc_sock_pipe_init(struct sc_sock_pipe* pipe, int type);
int sc_sock_pipe_term(struct sc_sock_pipe* pipe);
int sc_sock_pipe_write(struct sc_sock_pipe* pipe, void* data, int len);
int sc_sock_pipe_read(struct sc_sock_pipe* pipe, void* data, int len);

#define sc_sock_on_error(...)

#if defined(__linux__)
#include <sys/epoll.h>


struct sc_sock_poll
{
    int fds;
    size_t count;
    size_t cap;
    struct epoll_event* events;
};

#elif defined(__FreeBSD__) || defined(__APPLE__)
#include <sys/event.h>

struct sc_sock_poll
{
    int fds;
    size_t count;
    size_t cap;
    struct kevent* events;
};
#else
#if !defined(_WIN32)
#include <sys/poll.h>
#endif

struct sc_sock_poll
{
    size_t count;
    size_t cap;
    void** data;
    struct pollfd* events;
};

#endif

int sc_sock_poll_init(struct sc_sock_poll* poll);
int sc_sock_poll_term(struct sc_sock_poll* poll);

int sc_sock_poll_add(struct sc_sock_poll* poll, struct sc_sock_fd* fdt,
                     enum sc_sock_ev events, void* data);
int sc_sock_poll_del(struct sc_sock_poll* poll, struct sc_sock_fd* fdt,
                     enum sc_sock_ev events, void* data);

void* sc_sock_poll_data(struct sc_sock_poll* poll, size_t i);
uint32_t sc_sock_poll_event(struct sc_sock_poll* poll, size_t i);
int sc_sock_poll_wait(struct sc_sock_poll* poll, int timeout);

#endif

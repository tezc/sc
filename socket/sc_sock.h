/*
 * BSD-3-Clause
 *
 * Copyright 2021 Ozan Tezcan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef SC_SOCK_H
#define SC_SOCK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SC_SOCK_VERSION "2.0.0"

#ifdef SC_HAVE_CONFIG_H
#include "config.h"
#else
#define sc_sock_malloc malloc
#define sc_sock_realloc realloc
#define sc_sock_free free
#endif

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

struct sc_sock_fd {
	sc_sock_int fd;
	enum sc_sock_ev op;
	int type; // user data
	int index;
};

struct sc_sock {
	struct sc_sock_fd fdt;
	bool blocking;
	int family;
	char err[128];
};

/**
 * Call once when your application starts.
 * @return 0 on success, negative on failure.
 */
int sc_sock_startup();

/**
 * Call once before your application terminates
 * @return 0 on success, negative on failure.
 */
int sc_sock_cleanup();

/**
 * Initialize sock
 *
 * @param s        sock
 * @param type     user data
 * @param blocking is socket blocking
 * @param family   one of SC_SOCK_INET, SC_SOCK_INET6, SC_SOCK_UNIX
 */
void sc_sock_init(struct sc_sock *s, int type, bool blocking, int family);

/**
 * Destroy sock
 *
 * @param s sock
 * @return  0' on success, negative number on failure.
 *          call sc_sock_error() for error string.
 */
int sc_sock_term(struct sc_sock *s);

/**
 * @param s    sock
 * @param host host
 * @param port port
 * @return    '0' on success, negative number on failure.
 *             call sc_sock_error() for error string.
 */
int sc_sock_listen(struct sc_sock *s, const char *host, const char *port);

/**
 * @param s    sock
 * @param in   sock struct pointer the incoming connection
 * @return    '0' on success, negative number on failure.
 *             call sc_sock_error() for error string.
 */
int sc_sock_accept(struct sc_sock *s, struct sc_sock *in);

/**
 * @param s            sock
 * @param dst_addr    destination addr
 * @param dst_port    destination port
 * @param src_addr  source addr (outgoing addr)
 * @param src_port  source port (outgoing port)
 * @return            '0' on success
 *                    negative value if it is non-blocking, errno will be EAGAIN
 *                    negative value on error, call sc_sock_error() error text.
 */
int sc_sock_connect(struct sc_sock *s, const char *dst_addr,
		    const char *dst_port, const char *src_addr,
		    const char *src_port);

/**
 * Set socket blocking or nonblocking. Normally, you don't call this directly.
 * sc_sock_init() takes 'blocking' parameter, so sockets will be set according
 * to it.
 *
 * @param s        sock
 * @param blocking blocking
 * @return         '0' on success, negative number on failure.
 *                 call sc_sock_error() for error string.
 */
int sc_sock_set_blocking(struct sc_sock *s, bool blocking);

/**
 * @param sock sock
 * @param ms   timeout milliseconds
 * @return     '0' on success, negative number on failure.
 *             call sc_sock_error() for error string.
 */
int sc_sock_set_rcvtimeo(struct sc_sock *s, int ms);

/**
 * @param s    sock
 * @param ms   timeout milliseconds
 * @return     '0' on success, negative number on failure.
 *             call sc_sock_error() for error string.
 */
int sc_sock_set_sndtimeo(struct sc_sock *s, int ms);

/**
 * Finish connect for nonblocking connections. This function must be called
 * after sc_sock_poll() indicates socket is writable.
 *
 * @param s sock
 * @return  '0' on success, negative number on failure.
 *          call sc_sock_error() for error string.
 */
int sc_sock_finish_connect(struct sc_sock *s);

/**
 * @param s     sock
 * @param buf   buf
 * @param len   len
 * @param flags normally should be zero, otherwise flags are passed to send().
 * @return      - on success, returns sent byte count.
 *              - negative value if it fails with errno = EAGAIN.
 *              - negative value on error
 */
int sc_sock_send(struct sc_sock *s, char *buf, int len, int flags);

/**
 * @param s     sock
 * @param buf   buf
 * @param len   len
 * @param flags normally should be zero, otherwise flags are passed to recv().
 * @return      - on success, returns bytes received.
 *              - negative value if it fails with errno = EAGAIN.
 *              - negative value on error
 */
int sc_sock_recv(struct sc_sock *s, char *buf, int len, int flags);

/**
 * @param s sock
 * @return  last error string
 */
const char *sc_sock_error(struct sc_sock *s);

/**
 * @param s    sock
 * @param buf  buf
 * @param len  buf len
 * @return     local host:port string of the socket.
 */
const char *sc_sock_local_str(struct sc_sock *s, char *buf, size_t len);

/**
 * @param s    sock
 * @param buf  buf
 * @param len  buf len
 * @return     remote host:port string of the socket.
 */
const char *sc_sock_remote_str(struct sc_sock *s, char *buf, size_t len);

/**
 * Print socket in format "Local(127.0.0.1:8080), Remote(180.20.20.3:9000)"
 *
 * @param s    sock
 * @param buf  buf
 * @param len  buf len
 */
void sc_sock_print(struct sc_sock *s, char *buf, size_t len);

/**
 * Linux only. Helper function make your application a daemon with systemd.
 * e.g
 * sc_sock_notify_systemd("READY=1\n");           // Tell systemd app started
 * sc_sock_notify_systemd("STATUS=doing work\n"); // Tell systemd app doing sth
 * sc_sock_notify_systemd("STOPPING=1\n")       ; // Tell systemd app will stop
 *
 * @param msg msg with systemd protocol format
 * @return    '0' on success, negative on error, errno will be set.
 */
int sc_sock_notify_systemd(const char *msg);

struct sc_sock_pipe {
	struct sc_sock_fd fdt;
	sc_sock_int fds[2];
	char err[128];
};

/**
 * Create pipe
 *
 * @param p    pipe
 * @param type user data into struct sc_sock_fdt
 * @return '0' on success, negative number on failure,
 *         call sc_sock_pipe_err() to get error string
 */
int sc_sock_pipe_init(struct sc_sock_pipe *p, int type);

/**
 * Destroy pipe
 *
 * @param p pipe
 * @return '0' on success, negative number on failure,
 *         call sc_sock_pipe_err() to get error string
 */
int sc_sock_pipe_term(struct sc_sock_pipe *p);

/**
 * Write data to pipe
 *
 * @param p    pipe
 * @param data data
 * @param len  data len
 * @return     written data len, normally pipe is blocking, return value should
 *             be equal to 'len'
 */
int sc_sock_pipe_write(struct sc_sock_pipe *p, void *data, unsigned int len);

/**
 * Read data from pipe
 *
 * @param p    pipe
 * @param data destination
 * @param len  read size
 * @return     read data len, normally pipe is blocking, return value should
 *             be equal to 'len'
 */
int sc_sock_pipe_read(struct sc_sock_pipe *p, void *data, unsigned int len);

/**
 * Get error string
 * @param p pipe
 * @return  last error string
 */
const char *sc_sock_pipe_err(struct sc_sock_pipe *p);

#if defined(__linux__)

#include <sys/epoll.h>

struct sc_sock_poll {
	int fds;
	int count;
	int cap;
	struct epoll_event *events;
	char err[128];
};

#elif defined(__FreeBSD__) || defined(__APPLE__)
#include <sys/event.h>

struct sc_sock_poll {
	int fds;
	int count;
	int cap;
	struct kevent *events;
	char err[128];
};
#else

#if !defined(_WIN32)
#include <poll.h>
#endif

struct sc_sock_poll {
	int count;
	int cap;
	void **data;
	struct pollfd *events;
	char err[128];
};

#endif

/**
 * Create poll
 *
 * @param p poll
 * @return  '0' on success, negative number on failure,
 *          call sc_sock_poll_err() to get error string
 */
int sc_sock_poll_init(struct sc_sock_poll *p);

/**
 * Destroy poll
 *
 * @param p poll
 * @return  '0' on success, negative number on failure,
 *          call sc_sock_poll_err() to get error string
 */
int sc_sock_poll_term(struct sc_sock_poll *p);

/**
 * Add fd to to poller.
 *
 * @param p       poll
 * @param fdt     fdt
 * @param events  SC_SOCK_READ, SC_SOCK_WRITE or SC_SOCK_READ | SC_SOCK_WRITE
 * @param data    user data
 * @return        '0' on success, negative number on failure,
 *                call sc_sock_poll_err() to get error string
 */
int sc_sock_poll_add(struct sc_sock_poll *p, struct sc_sock_fd *fdt,
		     enum sc_sock_ev events, void *data);

/**
 *
 * @param p      poll
 * @param fdt    fdt
 * @param events SC_SOCK_READ, SC_SOCK_WRITE or SC_SOCK_READ | SC_SOCK_WRITE
 * @param data   user data
 * @return       '0' on success, negative number on failure,
 *               call sc_sock_poll_err() to get error string
 */
int sc_sock_poll_del(struct sc_sock_poll *p, struct sc_sock_fd *fdt,
		     enum sc_sock_ev events, void *data);

/**
 * e.g
 *  int n = sc_sock_poll_wait(poll, 100);
 *  for (int i = 0; i < n; i++) {
 *      void *user_data = sc_sock_poll_data(poll, i);
 *      uint32_t events = sc_sock_poll_event(poll, i);
 *
 *      if (events & SC_SOCK_READ)  {
 *          // Handle read event
 *      }
 *
 *      if (events & SC_SOCK_WRITE)  {
 *          // Handle write event
 *      }
 *  }
 *
 * @param poll    poll
 * @param timeout timeout
 * @return
 */
int sc_sock_poll_wait(struct sc_sock_poll *p, int timeout);

/**
 *
 * @param p poll
 * @param i event index
 * @return  user data of fd at index 'i'
 */
void *sc_sock_poll_data(struct sc_sock_poll *p, int i);

/**
 *
 * @param p poll
 * @param i event index
 * @return  events of fd at index 'i', events might be :
 *          - SC_SOCK_READ
 *          - SC_SOCK_WRITE
 *          - SC_SOCK_READ | SC_SOCK_WRITE
 *
 *          Closed fd will set SC_SOCK_READ | SC_SOCK_WRITE together. So,
 *          any attempt to read or write will indicate socket is closed.
 */
uint32_t sc_sock_poll_event(struct sc_sock_poll *p, int i);

/**
 * Get error string
 *
 * @param p poll
 * @return  last error string
 */
const char *sc_sock_poll_err(struct sc_sock_poll *p);

#endif

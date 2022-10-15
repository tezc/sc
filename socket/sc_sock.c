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

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "sc_sock.h"

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef SC_SIZE_MAX
#define SC_SIZE_MAX INT32_MAX
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <afunix.h>
#include <assert.h>
#include <ws2tcpip.h>

#pragma warning(disable : 4996)
#define sc_close(n) closesocket(n)
#define sc_unlink(n) DeleteFileA(n)
#define SC_ERR SOCKET_ERROR
#define SC_INVALID INVALID_SOCKET
#define SC_EAGAIN WSAEWOULDBLOCK
#define SC_EINPROGRESS WSAEINPROGRESS
#define SC_EINTR WSAEINTR

typedef int socklen_t;

static int sc_sock_err()
{
	return WSAGetLastError();
}

static void sc_sock_errstr(struct sc_sock *s, int gai_err)
{
	int rc;
	DWORD err = WSAGetLastError();
	LPSTR str = 0;

	rc = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				    FORMAT_MESSAGE_FROM_SYSTEM,
			    NULL, err, 0, (LPSTR) &str, 0, NULL);
	if (rc != 0) {
		strncpy(s->fdt.err, str, sizeof(s->fdt.err) - 1);
		LocalFree(str);
	}
}

int sc_sock_set_blocking(struct sc_sock *s, bool blocking)
{
	u_long mode = blocking ? 0 : 1;
	int rc = ioctlsocket(s->fdt.fd, FIONBIO, &mode);

	return rc == 0 ? 0 : -1;
}

int sc_sock_startup()
{
	int rc;
	WSADATA data;

	rc = WSAStartup(MAKEWORD(2, 2), &data);
	if (rc != 0 ||
	    (LOBYTE(data.wVersion) != 2 || HIBYTE(data.wVersion) != 2)) {
		return -1;
	}

	return 0;
}

int sc_sock_cleanup()
{
	int rc;

	rc = WSACleanup();
	return rc != 0 ? -1 : 0;
}

int sc_sock_notify_systemd(const char *msg)
{
	return -1;
}

#else
#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>

#define sc_close(n) close(n)
#define sc_unlink(n) unlink(n)
#define SC_ERR (-1)
#define SC_INVALID (-1)
#define SC_EAGAIN EAGAIN
#define SC_EINPROGRESS EINPROGRESS
#define SC_EINTR EINTR

int sc_sock_notify_systemd(const char *msg)
{
	assert(msg != NULL);

	int fd, rc;
	const char *s;

	struct sockaddr_un addr = {
		.sun_family = AF_UNIX,
	};

	struct iovec iovec = {
		.iov_base = (char *) msg,
		.iov_len = strlen(msg),
	};

	struct msghdr msghdr = {
		.msg_name = &addr,
		.msg_iov = &iovec,
		.msg_iovlen = 1,
	};

	s = getenv("NOTIFY_SOCKET");
	if (!s) {
		errno = EINVAL;
		return -1;
	}

	if ((s[0] != '@' && s[0] != '/') || s[1] == '\0') {
		errno = EINVAL;
		return -1;
	}

	fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (fd < 0) {
		return -1;
	}

	strncpy(addr.sun_path, s, sizeof(addr.sun_path) - 1);
	if (addr.sun_path[0] == '@') {
		addr.sun_path[0] = '\0';
	}

	msghdr.msg_namelen = offsetof(struct sockaddr_un, sun_path) + strlen(s);
	if (msghdr.msg_namelen > sizeof(struct sockaddr_un)) {
		msghdr.msg_namelen = sizeof(struct sockaddr_un);
	}

	rc = (int) sendmsg(fd, &msghdr, MSG_NOSIGNAL);

	close(fd);

	return rc < 0 ? -1 : 0;
}

int sc_sock_startup(void)
{
	return 0;
}

int sc_sock_cleanup(void)
{
	return 0;
}

static int sc_sock_err(void)
{
	return errno;
}

static void sc_sock_errstr(struct sc_sock *s, int gai_err)
{
	const char *str;

	str = gai_err ? gai_strerror(gai_err) : strerror(errno);
	strncpy(s->fdt.err, str, sizeof(s->fdt.err) - 1);
}

int sc_sock_set_blocking(struct sc_sock *s, bool blocking)
{
	int flags;

	flags = fcntl(s->fdt.fd, F_GETFL, 0);
	if (flags == -1) {
		return -1;
	}

	flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
	return (fcntl(s->fdt.fd, F_SETFL, flags) == 0) ? 0 : -1;
}

#endif

void sc_sock_init(struct sc_sock *s, int type, bool blocking, int family)
{
	s->fdt.fd = -1;
	s->fdt.type = type;
	s->fdt.op = SC_SOCK_NONE;
#if defined(_WIN32) || defined(_WIN64)
	s->fdt.poll_data = NULL;
#endif

	s->blocking = blocking;
	s->family = family;

	memset(s->fdt.err, 0, sizeof(s->fdt.err));
}

static int sc_sock_close(struct sc_sock *s)
{
	int rc = 0;

	if (s->fdt.fd != -1) {
		rc = sc_close(s->fdt.fd);
		s->fdt.fd = -1;
	}

	return (rc == 0) ? 0 : -1;
}

int sc_sock_term(struct sc_sock *s)
{
	int rc;

	rc = sc_sock_close(s);
	if (rc != 0) {
		sc_sock_errstr(s, 0);
	}

	return rc;
}

int sc_sock_set_rcvtimeo(struct sc_sock *s, int ms)
{
	int rc;
	void *p;

	struct timeval tv = {
		.tv_usec = ms % 1000,
		.tv_sec = ms / 1000,
	};

	p = (void *) &tv;

	rc = setsockopt(s->fdt.fd, SOL_SOCKET, SO_RCVTIMEO, p, sizeof(tv));
	if (rc != 0) {
		sc_sock_errstr(s, 0);
	}

	return rc;
}

int sc_sock_set_sndtimeo(struct sc_sock *s, int ms)
{
	int rc;
	void *p;

	struct timeval tv = {
		.tv_usec = ms % 1000,
		.tv_sec = ms / 1000,
	};

	p = (void *) &tv;

	rc = setsockopt(s->fdt.fd, SOL_SOCKET, SO_SNDTIMEO, p, sizeof(tv));
	if (rc != 0) {
		sc_sock_errstr(s, 0);
	}

	return rc;
}

static int sc_sock_bind_unix(struct sc_sock *s, const char *host)
{
	int rc;
	struct sockaddr_un addr = {
		.sun_family = AF_UNIX,
	};

	strncpy(addr.sun_path, host, sizeof(addr.sun_path) - 1);
	sc_unlink(host);

	rc = bind(s->fdt.fd, (struct sockaddr *) &addr, sizeof(addr));

	return rc == 0 ? 0 : -1;
}

static int sc_sock_bind(struct sc_sock *s, const char *host, const char *port)
{
	int rc, rv = 0;
	struct addrinfo *servinfo = NULL;
	struct addrinfo hints = {
		.ai_family = s->family,
		.ai_socktype = SOCK_STREAM,
	};

	*s->fdt.err = '\0';

	if (s->family == AF_UNIX) {
		sc_sock_int fd = socket(AF_UNIX, SOCK_STREAM, 0);
		if (fd == SC_INVALID) {
			goto error_unix;
		}

		s->fdt.fd = fd;

		rc = sc_sock_bind_unix(s, host);
		if (rc != 0) {
			goto error_unix;
		}

		return 0;

error_unix:
		sc_sock_errstr(s, 0);
		sc_sock_close(s);

		return -1;
	}

	rc = getaddrinfo(host, port, &hints, &servinfo);
	if (rc != 0) {
		sc_sock_errstr(s, rc);
		return -1;
	}

	for (struct addrinfo *p = servinfo; p != NULL; p = p->ai_next) {
		const int tsz = sizeof(int);
		sc_sock_int fd;
		void *tmp;

		fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (fd == SC_INVALID) {
			continue;
		}

		s->fdt.fd = fd;

		if (s->family == AF_INET6) {
			rc = setsockopt(s->fdt.fd, IPPROTO_IPV6, IPV6_V6ONLY,
					(void *) &(int){1}, tsz);
			if (rc != 0) {
				goto error;
			}
		}

		rc = sc_sock_set_blocking(s, s->blocking);
		if (rc != 0) {
			goto error;
		}

		tmp = (void *) &(int){1};
		rc = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, tmp, sizeof(int));
		if (rc != 0) {
			goto error;
		}

		rc = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, tmp, sizeof(int));
		if (rc != 0) {
			goto error;
		}

		rc = bind(s->fdt.fd, p->ai_addr, (socklen_t) p->ai_addrlen);
		if (rc == -1) {
			goto error;
		}

		goto out;
	}

error:
	sc_sock_errstr(s, 0);
	sc_sock_close(s);
	rv = -1;
out:
	freeaddrinfo(servinfo);

	return rv;
}

int sc_sock_finish_connect(struct sc_sock *s)
{
	int ret, rc;
	socklen_t len = sizeof(ret);

	rc = getsockopt(s->fdt.fd, SOL_SOCKET, SO_ERROR, (void *) &ret, &len);
	if (rc == 0 && ret != 0) {
		errno = ret;
		rc = -1;
	}
	if (rc != 0) {
		sc_sock_errstr(s, 0);
		return -1;
	}

	return 0;
}

static int sc_sock_connect_unix(struct sc_sock *s, const char *addr)
{
	const size_t len = strlen(addr);

	int rc;
	sc_sock_int fd;
	struct sockaddr_un un = {
		.sun_family = AF_UNIX,
	};

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd == SC_INVALID) {
		goto err;
	}

	s->fdt.fd = fd;

	if (len >= sizeof(un.sun_path)) {
		errno = EINVAL;
		goto err;
	}

	strcpy(un.sun_path, addr);

	rc = connect(s->fdt.fd, (struct sockaddr *) &un, sizeof(un));
	if (rc != 0) {
		goto err;
	}

	return 0;

err:
	sc_sock_errstr(s, 0);
	sc_sock_close(s);

	return -1;
}

static int sc_sock_bind_src(struct sc_sock *s, const char *addr,
			    const char *port)
{
	int rc;
	struct addrinfo *p = NULL, *i;
	struct addrinfo inf = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM,
	};

	rc = getaddrinfo(addr, port, &inf, &p);
	if (rc != 0) {
		goto gai_err;
	}

	for (i = p; i != NULL; i = i->ai_next) {
		rc = bind(s->fdt.fd, i->ai_addr, (socklen_t) i->ai_addrlen);
		if (rc != -1) {
			break;
		}
	}

	freeaddrinfo(p);

	if (rc == -1) {
		goto err;
	}

	return 0;

gai_err:
	sc_sock_errstr(s, rc);
	return -1;
err:
	sc_sock_errstr(s, 0);
	return -1;
}

int sc_sock_connect(struct sc_sock *s, const char *dst_addr,
		    const char *dst_port, const char *src_addr,
		    const char *src_port)
{
	int family = s->family;
	int rc, rv = 0;
	sc_sock_int fd;
	void *tmp;
	struct addrinfo *sinfo = NULL, *p;

	struct addrinfo inf = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM,
	};

	if (family == AF_UNIX) {
		return sc_sock_connect_unix(s, dst_addr);
	}

	rc = getaddrinfo(dst_addr, dst_port, &inf, &sinfo);
	if (rc != 0) {
		sc_sock_errstr(s, rc);
		return -1;
	}

	for (int i = 0; i < 2; i++) {
		for (p = sinfo; p != NULL; p = p->ai_next) {
			// Try same family addresses in the first iteration.                        
			if ((i == 0) ^ (p->ai_family == family)) {
				continue;
			}

			fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
			if (fd == SC_INVALID) {
				continue;
			}

			s->family = p->ai_family;
			s->fdt.fd = fd;

			rc = sc_sock_set_blocking(s, s->blocking);
			if (rc != 0) {
				goto error;
			}

			tmp = (void *) &(int){1};
			rc = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, tmp, sizeof(int));
			if (rc != 0) {
				goto error;
			}

			rc = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, tmp, sizeof(int));
			if (rc != 0) {
				goto error;
			}

			if (src_addr || src_port) {
				rc = sc_sock_bind_src(s, src_addr, src_port);
				if (rc != 0) {
					goto bind_error;
				}
			}

			rc = connect(fd, p->ai_addr, (socklen_t) p->ai_addrlen);
			if (rc != 0) {
				if (!s->blocking && (sc_sock_err() == SC_EINPROGRESS ||
							 sc_sock_err() == SC_EAGAIN)) {
					errno = EAGAIN;
					rv = -1;
					goto end;
				}

				sc_sock_close(s);
				continue;
			}

			goto end;
		}
	}

error:
	sc_sock_errstr(s, 0);
bind_error:
	rv = -1;
	sc_sock_close(s);
end:
	freeaddrinfo(sinfo);

	return rv;
}

int sc_sock_send(struct sc_sock *s, char *buf, int len, int flags)
{
	int n, err;

	if (len <= 0) {
		return len;
	}

retry:
	n = (int) send(s->fdt.fd, buf, (size_t) len, flags);
	if (n == SC_ERR) {
		err = sc_sock_err();
		if (err == SC_EINTR) {
			goto retry;
		}

		if (err == SC_EAGAIN) {
#if defined(_WIN32) || defined(_WIN64)
			// Stop masking WRITE event.
			struct sc_sock_poll_data *pd = s->fdt.poll_data;
            if (pd != NULL && (pd->edge_mask & SC_SOCK_WRITE)) {
				InterlockedAnd(&pd->edge_mask, ~SC_SOCK_WRITE);
			}
#endif
			errno = EAGAIN;
			return -1;
		}

		sc_sock_errstr(s, 0);
		n = -1;
	}

	return n;
}

int sc_sock_recv(struct sc_sock *s, char *buf, int len, int flags)
{
	int n, err;

	if (len <= 0) {
		return len;
	}

retry:
	n = (int) recv(s->fdt.fd, buf, (size_t) len, flags);
	if (n == 0) {
		errno = EOF;
		return -1;
	} else if (n == SC_ERR) {
		err = sc_sock_err();
		if (err == SC_EINTR) {
			goto retry;
		}

		if (err == SC_EAGAIN) {
#if defined(_WIN32) || defined(_WIN64)
			// Stop masking READ event.
			struct sc_sock_poll_data *pd = s->fdt.poll_data;
			if (pd != NULL && (pd->edge_mask & SC_SOCK_READ)) {
				InterlockedAnd(&pd->edge_mask, ~SC_SOCK_READ);
			}
#endif
			errno = EAGAIN;
			return -1;
		}

		sc_sock_errstr(s, 0);
		n = -1;
	}

	return n;
}

int sc_sock_accept(struct sc_sock *s, struct sc_sock *in)
{
	const void *tmp = (void *) &(int){1};

	int rc;
	sc_sock_int fd;

	fd = accept(s->fdt.fd, NULL, NULL);
	if (fd == SC_INVALID) {
		sc_sock_errstr(s, 0);
		return -1;
	}

	in->fdt.fd = fd;
	in->fdt.op = SC_SOCK_NONE;
	in->family = s->family;

	if (in->family != AF_UNIX) {
		rc = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, tmp, sizeof(int));
		if (rc != 0) {
			goto error;
		}
	}

	rc = sc_sock_set_blocking(in, s->blocking);
	if (rc != 0) {
		goto error;
	}

	return 0;

error:
	sc_sock_errstr(s, 0);
	sc_sock_close(in);

	return -1;
}

int sc_sock_listen(struct sc_sock *s, const char *host, const char *port)
{
	int rc;

	rc = sc_sock_bind(s, host, port);
	if (rc != 0) {
		return rc;
	}

	rc = listen(s->fdt.fd, 4096);
	if (rc != 0) {
		goto err;
	}

	return 0;
err:
	sc_sock_errstr(s, 0);
	sc_sock_close(s);
	return -1;
}

const char *sc_sock_error(struct sc_sock *s)
{
	s->fdt.err[sizeof(s->fdt.err) - 1] = '\0';
	return s->fdt.err;
}

static const char *sc_sock_addr(struct sc_sock *sock, int af, void *cp,
				char *buf, socklen_t len)
{
	const char *dest;

	dest = inet_ntop(af, cp, buf, len);
	if (dest == NULL) {
		sc_sock_errstr(sock, 0);
		*buf = '\0';
	}

	return dest;
}

static const char *sc_sock_print_storage(struct sc_sock *sock,
					 struct sockaddr_storage *storage,
					 char *buf, size_t len)
{
	const char *dst;
	struct sockaddr_in *addr;
	struct sockaddr_in6 *addr6;
	struct sockaddr_un *addr_un;
	char tmp[INET6_ADDRSTRLEN];

	*buf = '\0';

	switch (storage->ss_family) {
	case AF_INET:
		addr = (struct sockaddr_in *) storage;
		dst = sc_sock_addr(sock, AF_INET, &addr->sin_addr, tmp,
				   sizeof(tmp));
		snprintf(buf, len, "%s:%d", dst, ntohs(addr->sin_port));
		break;

	case AF_INET6:
		addr6 = (struct sockaddr_in6 *) storage;
		dst = sc_sock_addr(sock, AF_INET6, &addr6->sin6_addr, tmp,
				   sizeof(tmp));
		snprintf(buf, len, "%s:%d", dst, ntohs(addr6->sin6_port));
		break;

	case AF_UNIX:
		addr_un = (struct sockaddr_un *) storage;
		snprintf(buf, len, "%s", addr_un->sun_path);
		break;
	}

	return buf;
}

const char *sc_sock_local_str(struct sc_sock *sock, char *buf, size_t len)
{
	int rc;
	struct sockaddr_storage st;
	socklen_t storage_len = sizeof(st);

	rc = getsockname(sock->fdt.fd, (struct sockaddr *) &st, &storage_len);
	if (rc != 0) {
		sc_sock_errstr(sock, 0);
		*buf = '\0';
		return NULL;
	}

	return sc_sock_print_storage(sock, &st, buf, len);
}

const char *sc_sock_remote_str(struct sc_sock *sock, char *buf, size_t len)
{
	int rc;
	struct sockaddr_storage st;
	socklen_t storage_len = sizeof(st);

	rc = getpeername(sock->fdt.fd, (struct sockaddr *) &st, &storage_len);
	if (rc != 0) {
		sc_sock_errstr(sock, 0);
		*buf = '\0';
		return NULL;
	}

	return sc_sock_print_storage(sock, &st, buf, len);
}

void sc_sock_print(struct sc_sock *sock, char *buf, size_t len)
{
	char l[128];
	char r[128];

	sc_sock_local_str(sock, l, sizeof(l));
	sc_sock_remote_str(sock, r, sizeof(r));

	snprintf(buf, len, "Local(%s), Remote(%s) ", l, r);
}

const char *sc_sock_pipe_err(struct sc_sock_pipe *pipe)
{
	pipe->fdt.err[sizeof(pipe->fdt.err) - 1] = '\0';
	return pipe->fdt.err;
}

static void sc_sock_pipe_set_err(struct sc_sock_pipe *pipe, const char *fmt,
				 ...)
{
	va_list args;

	va_start(args, fmt);
	vsnprintf(pipe->fdt.err, sizeof(pipe->fdt.err), fmt, args);
	va_end(args);

	pipe->fdt.err[sizeof(pipe->fdt.err) - 1] = '\0';
}

#if defined(_WIN32) || defined(_WIN64)

int sc_sock_pipe_init(struct sc_sock_pipe *p, int type)
{
	SOCKET listener;
	int rc;
	struct sockaddr_in addr;
	int addrlen = sizeof(addr);
	int val = 1;
	BOOL nodelay = 1;

	*p = (struct sc_sock_pipe){
		.fds = {SC_INVALID, SC_INVALID},
	};

	p->fdt.type = type;
	p->fdt.op = SC_SOCK_NONE;
	p->fdt.poll_data = NULL;

	/*  Create listening socket. */
	listener = socket(AF_INET, SOCK_STREAM, 0);
	if (listener == SOCKET_ERROR) {
		goto wsafail;
	}

	rc = setsockopt(listener, SOL_SOCKET, SO_EXCLUSIVEADDRUSE,
			(char *) &val, sizeof(val));
	if (rc == SOCKET_ERROR) {
		goto wsafail;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = 0;

	rc = bind(listener, (const struct sockaddr *) &addr, sizeof(addr));
	if (rc == SOCKET_ERROR) {
		goto wsafail;
	}

	rc = getsockname(listener, (struct sockaddr *) &addr, &addrlen);
	if (rc == SOCKET_ERROR) {
		goto wsafail;
	}

	rc = listen(listener, 1);
	if (rc == SOCKET_ERROR) {
		goto wsafail;
	}

	p->fds[1] = socket(AF_INET, SOCK_STREAM, 0);
	if (p->fds[1] == SOCKET_ERROR) {
		goto wsafail;
	}

	rc = setsockopt(p->fds[1], IPPROTO_TCP, TCP_NODELAY, (char *) &nodelay,
			sizeof(nodelay));
	if (rc == SOCKET_ERROR) {
		goto wsafail;
	}

	rc = connect(p->fds[1], (struct sockaddr *) &addr, sizeof(addr));
	if (rc == SOCKET_ERROR) {
		goto wsafail;
	}

	p->fds[0] = accept(listener, (struct sockaddr *) &addr, &addrlen);
	if (p->fds[0] == INVALID_SOCKET) {
		goto wsafail;
	}

	p->fdt.fd = p->fds[0];
	closesocket(listener);

	return 0;

wsafail:
	sc_sock_pipe_set_err(p, "sc_sock_pipe_init() : %d ", WSAGetLastError());
	return -1;
}

int sc_sock_pipe_term(struct sc_sock_pipe *p)
{
	int rc = 0, rv;

	if (p->fds[0] == SC_INVALID) {
		return 0;
	}

	rv = closesocket(p->fds[0]);
	if (rv != 0) {
		rc = -1;
		sc_sock_pipe_set_err(p, "closesocket() : err(%d) ",
				     WSAGetLastError());
	}

	rv = closesocket(p->fds[1]);
	if (rv != 0) {
		rc = -1;
		sc_sock_pipe_set_err(p, "closesocket() : err(%d) ",
				     WSAGetLastError());
	}

	p->fds[0] = SC_INVALID;
	p->fds[1] = SC_INVALID;

	return rc;
}

int sc_sock_pipe_write(struct sc_sock_pipe *p, void *data, unsigned int len)
{
	int rc;

	rc = send(p->fds[1], data, len, 0);
	if (rc == SOCKET_ERROR) {
		sc_sock_pipe_set_err(p, "pipe send() : err(%d) ",
				     WSAGetLastError());
	}

	return rc;
}

int sc_sock_pipe_read(struct sc_sock_pipe *p, void *data, unsigned int len)
{
	int rc;

	rc = recv(p->fds[0], (char *) data, len, 0);
	if (rc == SOCKET_ERROR) {
		sc_sock_pipe_set_err(p, "pipe recv() : err(%d) ",
				     WSAGetLastError());
	}

	return rc;
}

#else

int sc_sock_pipe_init(struct sc_sock_pipe *p, int type)
{
	int rc;

	*p = (struct sc_sock_pipe){
		.fds = {SC_INVALID, SC_INVALID},
	};

	rc = pipe(p->fds);
	if (rc != 0) {
		sc_sock_pipe_set_err(p, "pipe() : %s ", strerror(errno));
		return -1;
	}

	p->fdt.type = type;
	p->fdt.op = SC_SOCK_NONE;
	p->fdt.fd = p->fds[0];

	return 0;
}

int sc_sock_pipe_term(struct sc_sock_pipe *p)
{
	int rc = 0, rv;

	if (p->fds[0] == SC_INVALID) {
		return 0;
	}

	rv = close(p->fds[0]);
	if (rv != 0) {
		rc = -1;
		sc_sock_pipe_set_err(p, "pipe close() : %s ", strerror(errno));
	}

	rv = close(p->fds[1]);
	if (rv != 0) {
		rc = -1;
		sc_sock_pipe_set_err(p, "pipe close() : %s ", strerror(errno));
	}

	p->fds[0] = SC_INVALID;
	p->fds[1] = SC_INVALID;

	return rc;
}

int sc_sock_pipe_write(struct sc_sock_pipe *p, void *data, unsigned int len)
{
	ssize_t n;
	char *b = data;

retry:
	n = write(p->fds[1], b, len);
	if (n == -1 && errno == EINTR) {
		goto retry;
	}

	return (int) n;
}

int sc_sock_pipe_read(struct sc_sock_pipe *p, void *data, unsigned int len)
{
	ssize_t n;
	char *b = data;

retry:
	n = read(p->fds[0], b, len);
	if (n == -1 && errno == EINTR) {
		goto retry;
	}

	return (int) n;
}

#endif

const char *sc_sock_poll_err(struct sc_sock_poll *p)
{
	return p->err;
}

static void sc_sock_poll_set_err(struct sc_sock_poll *p, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vsnprintf(p->err, sizeof(p->err), fmt, args);
	va_end(args);

	p->err[sizeof(p->err) - 1] = '\0';
}

#if defined(__linux__)

int sc_sock_poll_init(struct sc_sock_poll *p)
{
	int fds;

	*p = (struct sc_sock_poll){0};

	p->events = sc_sock_malloc(sizeof(*p->events) * SC_SOCK_POLL_MAX_EVENTS);
	if (p->events == NULL) {
		errno = ENOMEM;
		goto error;
	}

	fds = epoll_create1(0);
	if (fds == -1) {
		goto error;
	}
	p->fds = fds;

	return 0;
error:
	sc_sock_poll_set_err(p, strerror(errno));
	sc_sock_free(p->events);

	p->events = NULL;
	p->fds = -1;

	return -1;
}

int sc_sock_poll_term(struct sc_sock_poll *p)
{
	int rc;

	if (!p->events) {
		return 0;
	}

	sc_sock_free(p->events);

	rc = close(p->fds);
	if (rc != 0) {
		sc_sock_poll_set_err(p, strerror(errno));
	}

	p->events = NULL;
	p->fds = SC_INVALID;

	return rc;
}

int sc_sock_poll_add(struct sc_sock_poll *p, struct sc_sock_fd *fdt,
		     enum sc_sock_ev events, void *data)
{
	int rc, op = EPOLL_CTL_MOD;
	enum sc_sock_ev mask = fdt->op | events;

	struct epoll_event ep_ev = {
		.data.ptr = data,
		.events = EPOLLERR | EPOLLHUP | EPOLLRDHUP,
	};

	if (fdt->op == mask) {
		return 0;
	}

	if (fdt->op == SC_SOCK_NONE) {
		op = EPOLL_CTL_ADD;
	}

	if (mask & SC_SOCK_READ) {
		ep_ev.events |= EPOLLIN;
	}

	if (mask & SC_SOCK_WRITE) {
		ep_ev.events |= EPOLLOUT;
	}

	if (mask & SC_SOCK_EDGE) {
		ep_ev.events |= EPOLLET;
	}

	rc = epoll_ctl(p->fds, op, fdt->fd, &ep_ev);
	if (rc != 0) {
		sc_sock_poll_set_err(p, "epoll_ctl : %s ", strerror(errno));
		return -1;
	}

	fdt->op = mask;

	return 0;
}

int sc_sock_poll_del(struct sc_sock_poll *p, struct sc_sock_fd *fdt,
		     enum sc_sock_ev events, void *data)
{
	int rc, op;
	struct epoll_event ep_ev = {
		.data.ptr = data,
		.events = EPOLLERR | EPOLLHUP | EPOLLRDHUP,
	};

	if ((fdt->op & events) == 0) {
		return 0;
	}

	fdt->op &= ~events;

	if (fdt->op == SC_SOCK_EDGE) {
		fdt->op = SC_SOCK_NONE;
	}

	op = fdt->op == SC_SOCK_NONE ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;

	if (fdt->op & SC_SOCK_READ) {
		ep_ev.events |= EPOLLIN;
	}

	if (fdt->op & SC_SOCK_WRITE) {
		ep_ev.events |= EPOLLOUT;
	}

	if (fdt->op & SC_SOCK_EDGE) {
		ep_ev.events |= EPOLLET;
	}

	rc = epoll_ctl(p->fds, op, fdt->fd, &ep_ev);
	if (rc != 0) {
		sc_sock_poll_set_err(p, "epoll_ctl : %s ", strerror(errno));
		return -1;
	}

	return 0;
}

void *sc_sock_poll_data(struct sc_sock_poll *p, int i)
{
	return p->events[i].data.ptr;
}

uint32_t sc_sock_poll_event(struct sc_sock_poll *p, int i)
{
	uint32_t ev = 0;
	uint32_t epoll_ev = p->events[i].events;

	if (epoll_ev & EPOLLIN) {
		ev |= SC_SOCK_READ;
	}

	if (epoll_ev & EPOLLOUT) {
		ev |= SC_SOCK_WRITE;
	}

	epoll_ev &= EPOLLHUP | EPOLLRDHUP | EPOLLERR;
	if (epoll_ev != 0) {
		ev = (SC_SOCK_READ | SC_SOCK_WRITE);
	}

	return ev;
}

int sc_sock_poll_wait(struct sc_sock_poll *p, int timeout)
{
	int n;

	do {
		n = epoll_wait(p->fds, p->events, SC_SOCK_POLL_MAX_EVENTS, timeout);
	} while (n < 0 && errno == EINTR);

	if (n == -1) {
		sc_sock_poll_set_err(p, "epoll_wait : %s ", strerror(errno));
	}

	return n;
}

#elif defined(__APPLE__) || defined(__FreeBSD__)

int sc_sock_poll_init(struct sc_sock_poll *p)
{
	int fds;

	*p = (struct sc_sock_poll){0};

	p->events = sc_sock_malloc(sizeof(*p->events) * SC_SOCK_POLL_MAX_EVENTS);
	if (p->events == NULL) {
		errno = ENOMEM;
		goto err;
	}

	fds = kqueue();
	if (fds == -1) {
		goto err;
	}
	p->fds = fds;

	return 0;
err:
	sc_sock_poll_set_err(p, strerror(errno));
	sc_sock_free(p->events);
	p->events = NULL;
	p->fds = -1;

	return -1;
}

int sc_sock_poll_term(struct sc_sock_poll *p)
{
	int rc;

	if (!p->events) {
		return 0;
	}

	sc_sock_free(p->events);

	rc = close(p->fds);
	if (rc != 0) {
		sc_sock_poll_set_err(p, strerror(errno));
	}

	p->events = NULL;
	p->fds = SC_INVALID;

	return rc;
}

int sc_sock_poll_add(struct sc_sock_poll *p, struct sc_sock_fd *fdt,
		     enum sc_sock_ev events, void *data)
{
	int rc, count = 0;
	struct kevent ev[2];
	enum sc_sock_ev mask = fdt->op | events;

	if (fdt->op == mask) {
		return 0;
	}

	unsigned short act = EV_ADD;

	if (mask & SC_SOCK_EDGE) {
		act |= EV_CLEAR;
	}

	if (mask & SC_SOCK_WRITE) {
		EV_SET(&ev[count++], fdt->fd, EVFILT_WRITE, act, 0, 0, data);
	}

	if (mask & SC_SOCK_READ) {
		EV_SET(&ev[count++], fdt->fd, EVFILT_READ, act, 0, 0, data);
	}

	rc = kevent(p->fds, ev, count, NULL, 0, NULL);
	if (rc != 0) {
		sc_sock_poll_set_err(p, "kevent : %s ", strerror(errno));
		return -1;
	}

	fdt->op = mask;

	return 0;
}

int sc_sock_poll_del(struct sc_sock_poll *p, struct sc_sock_fd *fdt,
		     enum sc_sock_ev events, void *data)
{
	int rc, count = 0;
	struct kevent ev[2];
	enum sc_sock_ev mask = fdt->op & events;

	if (mask == 0) {
		return 0;
	}

	if (mask & SC_SOCK_READ) {
		EV_SET(&ev[count++], fdt->fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	} else if ((mask & SC_SOCK_EDGE) != 0 && (fdt->op & SC_SOCK_READ) != 0) {
		EV_SET(&ev[count++], fdt->fd, EVFILT_READ, EV_ADD, 0, 0, data);
	}

	if (mask & SC_SOCK_WRITE) {
		EV_SET(&ev[count++], fdt->fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
	} else if ((mask & SC_SOCK_EDGE) != 0 && (fdt->op & SC_SOCK_WRITE) != 0) {
		EV_SET(&ev[count++], fdt->fd, EVFILT_WRITE, EV_ADD, 0, 0, data);
	}

	rc = kevent(p->fds, ev, count, NULL, 0, NULL);
	if (rc != 0) {
		sc_sock_poll_set_err(p, "kevent : %s ", strerror(errno));
		return -1;
	}

	fdt->op &= ~events;

	if (fdt->op == SC_SOCK_EDGE) {
		fdt->op = SC_SOCK_NONE;
	}

	return 0;
}

void *sc_sock_poll_data(struct sc_sock_poll *p, int i)
{
	return p->events[i].udata;
}

uint32_t sc_sock_poll_event(struct sc_sock_poll *p, int i)
{
	uint32_t events = 0;

	if (p->events[i].flags & EV_EOF) {
		events = (SC_SOCK_READ | SC_SOCK_WRITE);
	} else if (p->events[i].filter == EVFILT_READ) {
		events |= SC_SOCK_READ;
	} else if (p->events[i].filter == EVFILT_WRITE) {
		events |= SC_SOCK_WRITE;
	}

	return events;
}

int sc_sock_poll_wait(struct sc_sock_poll *p, int timeout)
{
	int n;
	struct timespec ts;

	do {
		ts.tv_sec = timeout / 1000;
		ts.tv_nsec = (timeout % 1000) * 1000000;

		n = kevent(p->fds, NULL, 0, p->events, SC_SOCK_POLL_MAX_EVENTS,
			   timeout >= 0 ? &ts : NULL);
	} while (n < 0 && errno == EINTR);

	if (n == -1) {
		sc_sock_poll_set_err(p, "kevent : %s ", strerror(errno));
	}

	return n;
}

#else // WINDOWS

int sc_sock_poll_init(struct sc_sock_poll *p)
{
	*p = (struct sc_sock_poll){0};

	p->results = sc_sock_malloc(sizeof(*p->results) * SC_SOCK_POLL_MAX_EVENTS);
	if (p->results == NULL) {
		goto err;
	}

	p->events = sc_sock_malloc(sizeof(*p->events) * 16);
	if (p->events == NULL) {
		goto err;
	}

	p->data[0] = sc_sock_malloc(sizeof(struct sc_sock_poll_data) * 16);
	if (p->data[0] == NULL) {
		goto err;
	}

	p->cap = 16;

	for (int i = 0; i < p->cap; i++) {
		p->events[i].fd = SC_INVALID;
	}

	if (sc_sock_pipe_init(&p->signal_pipe, 0) != 0) {
		goto err;
	}

	InitializeCriticalSectionAndSpinCount(&p->lock, 4000);
	sc_sock_poll_add(p, &p->signal_pipe.fdt, SC_SOCK_READ, NULL);

	return 0;
err:
	sc_sock_free(p->results);
	sc_sock_free(p->events);
	sc_sock_free(p->data[0]);
	p->events = NULL;
	p->data[0] = NULL;

	sc_sock_poll_set_err(p, "Out of memory.");

	return -1;
}

int sc_sock_poll_term(struct sc_sock_poll *p)
{
	if (p->events == NULL) {
		return 0;
	}

	sc_sock_poll_del(p, &p->signal_pipe.fdt, SC_SOCK_READ, NULL);
	sc_sock_pipe_term(&p->signal_pipe);
	DeleteCriticalSection(&p->lock);

	for (int i = 0; i < 16 && p->data[i] != NULL; i++) {
		sc_sock_free(p->data[i]);
		p->data[i] = NULL;
	}

	sc_sock_free(p->results);
	sc_sock_free(p->events);
	sc_sock_free(p->ops);

	p->events = NULL;
	p->cap = 0;
	p->count = 0;

	return 0;
}

static int sc_sock_poll_expand(struct sc_sock_poll *p)
{
	int cap, rc = 0;
	struct pollfd *ev = NULL;

	if (p->count == p->cap) {
		if (p->cap >= SC_SIZE_MAX / 2) {
			goto err;
		}

		cap = p->cap * 2;
		ev = sc_sock_realloc(p->events, cap * sizeof(*ev));
		if (ev == NULL) {
			goto err;
		}

		// We do not use realloc for p->data as with p->events because we need a stable
		// pointer for sc_sock_poll_data->edge_mask, thus instead we append a next chunk
		// as large as the current capacity, this way we double the total capacity.
		// The resulting data structure will have the following chunk sizes: [16, 16, 32, 64, 128, 256... ]
		// Important properties of this data structure are that:
		//  - every new chunk is 2x larger than the previous one (except for the first chunk)
		//  - all the chunk sizes are power of 2
		// which allow to easily calculate coordinates in this two-dimensional data structure
		// from an absolute index (from zero to capacity).
		bool data_expanded = false;
		for (int i = 1; i < 16; i++) {
			if (p->data[i] == NULL) {
				p->data[i] = sc_sock_malloc(sizeof(struct sc_sock_poll_data) * p->cap);
				if (p->data[i] == NULL) {
					goto err;
				}
				data_expanded = true;
				break;
			}
		}
		if (!data_expanded) {
			goto err;
		}

		p->events = ev;

		for (int i = p->cap; i < cap; i++) {
			p->events[i].fd = SC_INVALID;
		}

		p->cap = cap;
	}

	return rc;

err:
	sc_sock_free(ev);
	sc_sock_poll_set_err(p, "Out of memory.");
	return -1;
}

static int sc_sock_poll_signal(struct sc_sock_poll *p, struct sc_sock_fd *fdt,
     		     enum sc_sock_ev events, void *data, bool add) {
	int index = p->ops_count;
	int cap = p->ops_cap;
	struct sc_sock_poll_op *ops = p->ops;

	if (index == cap) {
		if (cap >= SC_SIZE_MAX / 2) {
    		ops = NULL;
    	} else if (ops == NULL) {
			cap = 16;
			ops = sc_sock_malloc(cap * sizeof(*ops));
		} else {
			cap = cap * 3 / 2;
			ops = sc_sock_realloc(ops, cap * sizeof(*ops));
		}
		if (ops != NULL) {
			p->ops_cap = cap;
            p->ops = ops;
		} else {
			// TODO set OOM error
			LeaveCriticalSection(&p->lock);
			return -1;
        }
	}

	p->ops[index] = (struct sc_sock_poll_op){
		.add = add,
		.events = events,
		.fdt = fdt,
		.data = data,
	};
	p->ops_count++;

	LeaveCriticalSection(&p->lock);

	// Signal outside of the lock to reduce contention.
	// Signal only for the first submitted operation because it is not
	// any useful to do that for any subsequent one.
	if (index == 0 && sc_sock_pipe_write(&p->signal_pipe, "s", 1) != 1) {
		// TODO set error from pipe to socket
		return -1;
	}
	return 0;
}

// Define GCC intrinsic for MSVC to be compatible.
#ifdef _MSC_VER
// Number of leading zeros in unsigned int value.
#define __builtin_clz(x) ((int)__lzcnt(x))
#endif

static inline int sc_sock_number_of_meaningful_bits(unsigned int i) {
	return sizeof(unsigned int) * 8 - __builtin_clz(i);
}

static struct sc_sock_poll_data *sc_sock_poll_data_inner(struct sc_sock_poll *p, int i)
{
	if (i < 16) {
		return &p->data[0][i];
	}

	int n = sc_sock_number_of_meaningful_bits(i);

	// Equivalent of `n + 1 - sc_sock_number_of_meaningful_bits(first_chunk_size)`,
	// currently the smallest chunk size is 16, thus it is `n + 1 - 5`
	int x = n - 4;

	// Clear the most significant bit in i.
	int y = i & ~(1 << (n - 1));

	return &p->data[x][y];
}

int sc_sock_poll_add(struct sc_sock_poll *p, struct sc_sock_fd *fdt,
		     enum sc_sock_ev events, void *data)
{
	int rc;
	int index = -1;
	enum sc_sock_ev mask = fdt->op | events;
	struct sc_sock_poll_data *pd = NULL;

	if (fdt->op == mask) {
		return 0;
	}

	EnterCriticalSection(&p->lock);

	// If polling is in progress we can not modify p->events, instead submit
	// the operation for asynchronous processing later by the poller thread.
	if (p->polling) {
		// sc_sock_poll_signal() calls LeaveCriticalSection()
		return sc_sock_poll_signal(p, fdt, events, data, true);
	}

	if (fdt->op == SC_SOCK_NONE) {
		rc = sc_sock_poll_expand(p);
		if (rc != 0) {
			sc_sock_poll_set_err(p, "Out of memory."); // TODO set err to sock

			LeaveCriticalSection(&p->lock);
			return -1;
		}

		p->count++;

		for (int i = 0; i < p->cap; i++) {
			if (p->events[i].fd == SC_INVALID) {
				index = i;
				break;
			}
		}

		assert(index != -1);

		pd = sc_sock_poll_data_inner(p, index);
		pd->index = index;
		InterlockedExchange(&pd->edge_mask, SC_SOCK_NONE);
		fdt->poll_data = pd;

		p->events[index].fd = fdt->fd;
	} else {
		pd = fdt->poll_data;
		index = pd->index;
	}

	assert(index != -1);
	assert(pd != NULL);

	fdt->op = mask;

	p->events[index].events = 0;
	p->events[index].revents = 0;

	if (mask & SC_SOCK_READ) {
		p->events[index].events |= POLLIN;
	}

	if (mask & SC_SOCK_WRITE) {
		p->events[index].events |= POLLOUT;
	}

	if (mask & SC_SOCK_EDGE) {
		InterlockedOr(&pd->edge_mask, SC_SOCK_EDGE);
	}

	pd->data = data;

	LeaveCriticalSection(&p->lock);
	return 0;
}

int sc_sock_poll_del(struct sc_sock_poll *p, struct sc_sock_fd *fdt,
		     enum sc_sock_ev events, void *data)
{
	if ((fdt->op & events) == 0) {
		return 0;
	}

	EnterCriticalSection(&p->lock);

	// If polling is in progress we can not modify p->events, instead submit
	// the operation for asynchronous processing later by the poller thread.
	if (p->polling) {
		// sc_sock_poll_signal() calls LeaveCriticalSection()
		return sc_sock_poll_signal(p, fdt, events, data, false);
	}

	fdt->op &= ~events;

	if (fdt->op == SC_SOCK_EDGE) {
		fdt->op = SC_SOCK_NONE;
	}

	struct sc_sock_poll_data *pd = fdt->poll_data;

	if (fdt->op == SC_SOCK_NONE) {
		fdt->poll_data = NULL;
		p->events[pd->index].fd = SC_INVALID;
		p->count--;
		InterlockedExchange(&pd->edge_mask, SC_SOCK_NONE);
	} else {
		p->events[pd->index].events = 0;

		if (fdt->op & SC_SOCK_READ) {
			p->events[pd->index].events |= POLLIN;
		}

		if (fdt->op & SC_SOCK_WRITE) {
			p->events[pd->index].events |= POLLOUT;
		}

		if ((fdt->op & SC_SOCK_EDGE) == 0) {
			InterlockedExchange(&pd->edge_mask, SC_SOCK_NONE);
		}

		pd->data = data;
	}

	LeaveCriticalSection(&p->lock);
	return 0;
}

void *sc_sock_poll_data(struct sc_sock_poll *p, int i)
{
	return p->results[i].data;
}

uint32_t sc_sock_poll_event(struct sc_sock_poll *p, int i)
{
	return p->results[i].events;
}

static uint32_t sc_sock_poll_event_inner(struct sc_sock_poll *p, int i)
{
	if (p->events[i].fd == SC_INVALID) {
		return SC_SOCK_NONE;
	}

	uint32_t evs = 0;
	uint32_t poll_evs = p->events[i].revents;

	if (poll_evs == 0) {
		return SC_SOCK_NONE;
	}

	if (poll_evs & POLLIN) {
		evs |= SC_SOCK_READ;
	}

	if (poll_evs & POLLOUT) {
		evs |= SC_SOCK_WRITE;
	}

	// Start masking fired events in Edge-Triggered mode.
	struct sc_sock_poll_data *pd = sc_sock_poll_data_inner(p, i);
retry:
	LONG mask = pd->edge_mask;

	if (mask & SC_SOCK_EDGE) {
		// Since the kernel calls and edge_mask updates are separate events,
		// we can have two possible race conditions:
		// 1. "stop masking" happens after "start masking" will result in
		//     an extra event fired, which is fine.
		// 2. "stop masking" happens before "start masking" will mean that
		//     the current event will not be masked here, which is also fine.
		// It means the scenario when we miss events and hang should be impossible.
		if (mask != InterlockedCompareExchange(&pd->edge_mask, mask | evs, mask)) {
			goto retry;
		}
		evs &= ~mask;
	}

	poll_evs &= POLLHUP | POLLERR;
	if (poll_evs != 0) {
		evs = (SC_SOCK_READ | SC_SOCK_WRITE);
	}

	return evs;
}

static int sc_sock_poll_fill_results(struct sc_sock_poll *p) {
	int found = 0;
	int remaining = p->results_remaining;

	for (int i = p->results_offset; i < p->cap && remaining > 0; i++) {
		enum sc_sock_ev events = sc_sock_poll_event_inner(p, i);

		if (events != SC_SOCK_NONE) {
			remaining--;

			p->results[found] = (struct sc_sock_poll_result){
				.events = events,
				.data = sc_sock_poll_data_inner(p, i)->data,
			};

			found++;

			if (found == SC_SOCK_POLL_MAX_EVENTS) {
				p->results_offset = i + 1;
				break;
			}
		}
	}
	p->results_remaining = remaining;
	return found;
}

int sc_sock_poll_wait(struct sc_sock_poll *p, int timeout)
{
	int n, rc;

	timeout = (timeout == -1) ? 16 : timeout;

	EnterCriticalSection(&p->lock);
	if (p->polling) {
		rc = -1;
		sc_sock_poll_set_err(p, "poll : polling is already in progress in parallel thread");
	} else if (p->results_remaining > 0) {
		// Fill the remaining results that did not fit into p->results in the previous iteration.
		rc = sc_sock_poll_fill_results(p);
	} else {
		// When p->polling is set to true, no add/del operations can happen,
		// instead these operations will be submitted to the sc_sock_poll->ops list
		// and processed by the poller thread after the polling is finished.
		p->polling = true;
	}
	LeaveCriticalSection(&p->lock);

	if (!p->polling) {
		return rc;
	}

	do {
		// When p->polling is set to true p->events and p->cap are stable
		// and can be read outside of the lock.
		n = WSAPoll(p->events, (ULONG) p->cap, timeout);
	} while (n < 0 && errno == EINTR);

	// Drain signal_pipe only when we can do that without blocking.
	if (n > 0 && sc_sock_poll_event_inner(p, 0) == SC_SOCK_READ) {
		n--;
		// To reduce contention we drain signal_pipe outside of the lock,
		// it is OK to do that before the processing of p->ops: this way we do
		// not drain any extra signals.
		//
		// At this point we may have any number of bytes in signal_pipe,
		// we just pick reasonably large buffer size for draining and
		// assume that sc_sock_pipe_read() supports partial reads (less than
		// the provided buffer size).
		char drain_buf[16];
		sc_sock_pipe_read(&p->signal_pipe, drain_buf, sizeof(drain_buf));
	}

	if (n == 0) {
		rc = 0;
	} else if (n == SOCKET_ERROR) {
		rc = -1;
		sc_sock_poll_set_err(p, "poll : %s ", strerror(errno));
	}

	EnterCriticalSection(&p->lock);
	// Have to reset this flag before processing p->ops because it will be checked
	// inside of sc_sock_poll_add() and sc_sock_poll_del().
	assert(p->polling);
	p->polling = false;

	// Process signalled socket operations in the same order they were added.
	for (int i = 0; i < p->ops_count; i++) {
		struct sc_sock_poll_op *o = &p->ops[i];
		if (o->add) {
			sc_sock_poll_add(p, o->fdt, o->events, o->data);
		} else {
			sc_sock_poll_del(p, o->fdt, o->events, o->data);
		}
	}
	p->ops_count = 0;

	if (n > 0) {
		// We have a separate buffer for results and fill it under the lock
		// because otherwise we would need to acquire the lock on every
		// sc_sock_poll_event() and sc_sock_poll_data() call which would be suboptimal.
		p->results_offset = 1;
		p->results_remaining = n;
		rc = sc_sock_poll_fill_results(p);
	}

	LeaveCriticalSection(&p->lock);

	return rc;
}

#endif

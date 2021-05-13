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
#include <Ws2tcpip.h>

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
		strncpy(s->err, str, sizeof(s->err) - 1);
		LocalFree(str);
	}
}

int sc_sock_set_blocking(struct sc_sock *s, bool blocking)
{
	int mode = blocking ? 0 : 1;
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

	rc = sendmsg(fd, &msghdr, MSG_NOSIGNAL);

	close(fd);

	return rc < 0 ? -1 : 0;
}

int sc_sock_startup()
{
	return 0;
}

int sc_sock_cleanup()
{
	return 0;
}

static int sc_sock_err()
{
	return errno;
}

static void sc_sock_errstr(struct sc_sock *s, int gai_err)
{
	const char *str;

	str = gai_err ? gai_strerror(gai_err) : strerror(errno);
	strncpy(s->err, str, sizeof(s->err) - 1);
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
	s->fdt.index = -1;
	s->blocking = blocking;
	s->family = family;

	memset(s->err, 0, sizeof(s->err));
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

	*s->err = '\0';

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
	if (rc != 0 || ret != 0) {
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
	int rc, rv = 0;
	sc_sock_int fd;
	void *tmp;
	struct addrinfo *sinfo = NULL, *p;

	struct addrinfo inf = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM,
	};

	if (s->family == AF_UNIX) {
		return sc_sock_connect_unix(s, dst_addr);
	}

	rc = getaddrinfo(dst_addr, dst_port, &inf, &sinfo);
	if (rc != 0) {
		sc_sock_errstr(s, rc);
		return -1;
	}

	for (p = sinfo; p != NULL; p = p->ai_next) {
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

		rc = connect(s->fdt.fd, p->ai_addr, (socklen_t) p->ai_addrlen);
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
	s->err[sizeof(s->err) - 1] = '\0';
	return s->err;
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
	return pipe->err;
}

static void sc_sock_pipe_set_err(struct sc_sock_pipe *pipe, const char *fmt,
				 ...)
{
	va_list args;

	va_start(args, fmt);
	vsnprintf(pipe->err, sizeof(pipe->err), fmt, args);
	va_end(args);

	pipe->err[sizeof(pipe->err) - 1] = '\0';
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
	p->fdt.index = -1;
	p->fds[0] = INVALID_SOCKET;
	p->fds[1] = INVALID_SOCKET;

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
	if (rc == SOCKET_ERROR || (unsigned int) rc != len) {
		sc_sock_pipe_set_err(p, "pipe send() : err(%d) ",
				     WSAGetLastError());
	}

	return rc;
}

int sc_sock_pipe_read(struct sc_sock_pipe *p, void *data, unsigned int len)
{
	int rc;

	rc = recv(p->fds[0], (char *) data, len, 0);
	if (rc == SOCKET_ERROR || (unsigned int) rc != len) {
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

	return n;
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

	return n;
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

	p->events = sc_sock_malloc(sizeof(*p->events) * 16);
	if (p->events == NULL) {
		errno = ENOMEM;
		goto error;
	}

	fds = epoll_create1(0);
	if (fds == -1) {
		goto error;
	}

	p->cap = 16;
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
	p->cap = 0;
	p->count = 0;

	return rc;
}

static int sc_sock_poll_expand(struct sc_sock_poll *p)
{
	int cap, rc = 0;
	void *ev;

	if (p->count == p->cap) {
		if (p->cap >= SC_SIZE_MAX / 2) {
			goto error;
		}

		cap = p->cap * 2;
		ev = sc_sock_realloc(p->events, cap * sizeof(*p->events));
		if (ev == NULL) {
			goto error;
		}

		p->cap = cap;
		p->events = ev;
	}

	return rc;

error:
	sc_sock_poll_set_err(p, "Out of memory.");
	return -1;
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

	if ((fdt->op & events) == events) {
		return 0;
	}

	if (fdt->op == SC_SOCK_NONE) {
		rc = sc_sock_poll_expand(p);
		if (rc != 0) {
			return -1;
		}

		op = EPOLL_CTL_ADD;
	}

	if (mask & SC_SOCK_READ) {
		ep_ev.events |= EPOLLIN;
	}

	if (mask & SC_SOCK_WRITE) {
		ep_ev.events |= EPOLLOUT;
	}

	rc = epoll_ctl(p->fds, op, fdt->fd, &ep_ev);
	if (rc != 0) {
		sc_sock_poll_set_err(p, "epoll_ctl : %s ", strerror(errno));
		return -1;
	}

	p->count += fdt->op == SC_SOCK_NONE;
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
	op = fdt->op == SC_SOCK_NONE ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;

	if (fdt->op & SC_SOCK_READ) {
		ep_ev.events |= EPOLLIN;
	}

	if (fdt->op & SC_SOCK_WRITE) {
		ep_ev.events |= EPOLLOUT;
	}

	rc = epoll_ctl(p->fds, op, fdt->fd, &ep_ev);
	if (rc != 0) {
		sc_sock_poll_set_err(p, "epoll_ctl : %s ", strerror(errno));
		return -1;
	}

	if (fdt->op == SC_SOCK_NONE) {
		p->count--;
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
		n = epoll_wait(p->fds, &p->events[0], p->cap, timeout);
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

	p->events = sc_sock_malloc(sizeof(*p->events) * 16);
	if (p->events == NULL) {
		errno = ENOMEM;
		goto err;
	}

	fds = kqueue();
	if (fds == -1) {
		goto err;
	}

	p->cap = 16;
	p->fds = fds;

	return 0;
err:
	sc_sock_poll_set_err(p, strerror(errno));
	sc_sock_free(p->events);
	p->events = NULL;
	p->fds = -1;

	return -1;
}

static int sc_sock_poll_expand(struct sc_sock_poll *p)
{
	int rc = 0, cap;
	void *ev;

	if (p->count == p->cap) {
		if (p->cap >= SC_SIZE_MAX / 2) {
			goto err;
		}

		cap = p->cap * 2;
		ev = sc_sock_realloc(p->events, cap * sizeof(*p->events));
		if (ev == NULL) {
			goto err;
		}

		p->cap = cap;
		p->events = ev;
	}

	return rc;

err:
	sc_sock_poll_set_err(p, "Out of memory.");
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
	p->cap = 0;
	p->count = 0;

	return rc;
}

int sc_sock_poll_add(struct sc_sock_poll *p, struct sc_sock_fd *fdt,
		     enum sc_sock_ev events, void *data)
{
	int rc, count = 0;
	struct kevent ev[2];
	int mask = fdt->op | events;

	if ((fdt->op & events) == events) {
		return 0;
	}

	if (fdt->op == SC_SOCK_NONE) {
		rc = sc_sock_poll_expand(p);
		if (rc != 0) {
			return -1;
		}
	}

	if (mask & SC_SOCK_WRITE) {
		EV_SET(&ev[count++], fdt->fd, EVFILT_WRITE, EV_ADD, 0, 0, data);
	}

	if (mask & SC_SOCK_READ) {
		EV_SET(&ev[count++], fdt->fd, EVFILT_READ, EV_ADD, 0, 0, data);
	}

	rc = kevent(p->fds, ev, count, NULL, 0, NULL);
	if (rc != 0) {
		sc_sock_poll_set_err(p, "kevent : %s ", strerror(errno));
		return -1;
	}

	p->count += fdt->op == SC_SOCK_NONE;
	fdt->op = mask;

	return 0;
}

int sc_sock_poll_del(struct sc_sock_poll *p, struct sc_sock_fd *fdt,
		     enum sc_sock_ev events, void *data)
{
	(void) data;

	int rc, count = 0;
	struct kevent ev[2];
	int mask = fdt->op & events;

	if (mask == 0) {
		return 0;
	}

	if (mask & SC_SOCK_READ) {
		EV_SET(&ev[count++], fdt->fd, EVFILT_READ, EV_DELETE, 0, 0, 0);
	}

	if (mask & SC_SOCK_WRITE) {
		EV_SET(&ev[count++], fdt->fd, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
	}

	rc = kevent(p->fds, ev, count, NULL, 0, NULL);
	if (rc != 0) {
		sc_sock_poll_set_err(p, "kevent : %s ", strerror(errno));
		return -1;
	}

	fdt->op &= ~events;
	p->count -= fdt->op == SC_SOCK_NONE;

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

		n = kevent(p->fds, NULL, 0, &p->events[0], p->cap,
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

	p->events = sc_sock_malloc(sizeof(*p->events) * 16);
	if (p->events == NULL) {
		goto err;
	}

	p->data = sc_sock_malloc(sizeof(void *) * 16);
	if (p->data == NULL) {
		goto err;
	}

	p->cap = 16;

	for (int i = 0; i < p->cap; i++) {
		p->events[i].fd = SC_INVALID;
	}

	return 0;
err:
	sc_sock_free(p->events);
	p->events = NULL;
	p->data = NULL;

	sc_sock_poll_set_err(p, "Out of memory.");

	return -1;
}

int sc_sock_poll_term(struct sc_sock_poll *p)
{
	if (p->events == NULL) {
		return 0;
	}

	sc_sock_free(p->events);
	sc_sock_free(p->data);

	p->events = NULL;
	p->cap = 0;
	p->count = 0;

	return 0;
}

static int sc_sock_poll_expand(struct sc_sock_poll *p)
{
	int cap, rc = 0;
	void **data = NULL;
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

		data = sc_sock_realloc(p->data, cap * sizeof(*data));
		if (data == NULL) {
			goto err;
		}

		p->events = ev;
		p->data = data;

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

int sc_sock_poll_add(struct sc_sock_poll *p, struct sc_sock_fd *fdt,
		     enum sc_sock_ev events, void *data)
{
	int rc;
	int index = fdt->index;

	if ((fdt->op & events) == events) {
		return 0;
	}

	if (fdt->op == SC_SOCK_NONE) {
		rc = sc_sock_poll_expand(p);
		if (rc != 0) {
			sc_sock_poll_set_err(p, "Out of memory.");
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

		p->events[index].fd = fdt->fd;
		fdt->index = index;
	}

	assert(index != -1);

	fdt->op |= events;

	p->events[fdt->index].events = 0;
	p->events[fdt->index].revents = 0;

	if (events & SC_SOCK_READ) {
		p->events[fdt->index].events |= POLLIN;
	}

	if (events & SC_SOCK_WRITE) {
		p->events[fdt->index].events |= POLLOUT;
	}

	p->data[fdt->index] = data;

	return 0;
}

int sc_sock_poll_del(struct sc_sock_poll *p, struct sc_sock_fd *fdt,
		     enum sc_sock_ev events, void *data)
{
	if ((fdt->op & events) == 0) {
		return 0;
	}

	fdt->op &= ~events;
	if (fdt->op == SC_SOCK_NONE) {
		p->events[fdt->index].fd = SC_INVALID;
		p->count--;
		fdt->index = -1;
	} else {
		p->events[fdt->index].events = 0;

		if (fdt->op & SC_SOCK_READ) {
			p->events[fdt->index].events |= POLLIN;
		}

		if (fdt->op & SC_SOCK_WRITE) {
			p->events[fdt->index].events |= POLLOUT;
		}

		p->data[fdt->index] = data;
	}

	return 0;
}

void *sc_sock_poll_data(struct sc_sock_poll *p, int i)
{
	return p->data[i];
}

uint32_t sc_sock_poll_event(struct sc_sock_poll *p, int i)
{
	uint32_t evs = 0;
	uint32_t poll_evs = p->events[i].revents;

	if (poll_evs & POLLIN) {
		evs |= SC_SOCK_READ;
	}

	if (poll_evs & POLLOUT) {
		evs |= SC_SOCK_WRITE;
	}

	poll_evs &= POLLHUP | POLLERR;
	if (poll_evs != 0) {
		evs = (SC_SOCK_READ | SC_SOCK_WRITE);
	}

	return evs;
}

int sc_sock_poll_wait(struct sc_sock_poll *p, int timeout)
{
	int n, rc = p->cap;

	timeout = (timeout == -1) ? 16 : timeout;

	do {
		n = WSAPoll(p->events, (ULONG) p->cap, timeout);
	} while (n < 0 && errno == EINTR);

	if (n == SC_INVALID) {
		rc = -1;
		sc_sock_poll_set_err(p, "poll : %s ", strerror(errno));
	}

	return rc;
}

#endif

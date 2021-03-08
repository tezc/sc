/*
 * MIT License
 *
 * Copyright (c) 2021 Ozan Tezcan
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
    #include <Ws2tcpip.h>
    #include <afunix.h>
    #include <assert.h>

    #pragma warning(disable : 4996)
    #define sc_close(n)    closesocket(n)
    #define sc_unlink(n)   DeleteFileA(n)
    #define SC_ERR         SOCKET_ERROR
    #define SC_INVALID     INVALID_SOCKET
    #define SC_EAGAIN      WSAEWOULDBLOCK
    #define SC_EINPROGRESS WSAEINPROGRESS
    #define SC_EINTR       WSAEINTR

typedef int socklen_t;

static int sc_sock_err()
{
    return WSAGetLastError();
}

static void sc_sock_errstr(struct sc_sock *sock, int gai_err)
{
    int rc;
    DWORD err = WSAGetLastError();
    LPSTR errstr = 0;

    rc = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL, err, 0, (LPSTR) &errstr, 0, NULL);
    if (rc != 0) {
        strncpy(sock->err, errstr, sizeof(sock->err) - 1);
        LocalFree(errstr);
    }
}

int sc_sock_set_blocking(struct sc_sock *sock, bool blocking)
{
    int mode = blocking ? 0 : 1;
    int rc = ioctlsocket(sock->fdt.fd, FIONBIO, &mode);

    return rc == 0 ? 0 : -1;
}

int sc_sock_startup()
{
    int rc;
    WSADATA data;

    rc = WSAStartup(MAKEWORD(2, 2), &data);
    if (rc != 0 || (LOBYTE(data.wVersion) != 2 || HIBYTE(data.wVersion) != 2)) {
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

    #define sc_close(n)    close(n)
    #define sc_unlink(n)   unlink(n)
    #define SC_ERR         (-1)
    #define SC_INVALID     (-1)
    #define SC_EAGAIN      EAGAIN
    #define SC_EINPROGRESS EINPROGRESS
    #define SC_EINTR       EINTR

int sc_sock_notify_systemd(const char *msg)
{
    assert(msg != NULL);

    int fd, rc;
    const char *s;
    struct sockaddr_un addr = {.sun_family = AF_UNIX};
    struct iovec iovec = {.iov_base = (char *) msg, .iov_len = strlen(msg)};
    struct msghdr msghdr = {.msg_name = &addr,
            .msg_iov = &iovec,
            .msg_iovlen = 1};

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

static void sc_sock_errstr(struct sc_sock *sock, int gai_err)
{
    const char *str = gai_err ? gai_strerror(gai_err) : strerror(errno);

    strncpy(sock->err, str, sizeof(sock->err) - 1);
}

int sc_sock_set_blocking(struct sc_sock *sock, bool blocking)
{
    int flags = fcntl(sock->fdt.fd, F_GETFL, 0);
    if (flags == -1) {
        return -1;
    }

    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return (fcntl(sock->fdt.fd, F_SETFL, flags) == 0) ? 0 : -1;
}

#endif


void sc_sock_init(struct sc_sock *sock, int type, bool blocking, int family)
{
    sock->fdt.fd = -1;
    sock->fdt.type = type;
    sock->fdt.op = SC_SOCK_NONE;
    sock->fdt.index = -1;
    sock->blocking = blocking;
    sock->family = family;

    memset(sock->err, 0, sizeof(sock->err));
}

static int sc_sock_close(struct sc_sock *sock)
{
    int rc = 0;

    if (sock->fdt.fd != -1) {
        rc = sc_close(sock->fdt.fd);
        sock->fdt.fd = -1;
    }

    return (rc == 0) ? 0 : -1;
}

int sc_sock_term(struct sc_sock *sock)
{
    int rc;

    rc = sc_sock_close(sock);
    if (rc != 0) {
        sc_sock_errstr(sock, 0);
    }

    return rc;
}

int sc_sock_set_rcvtimeo(struct sc_sock *sock, int ms)
{
    int rc;
    void *p;

    struct timeval tv = {
            .tv_usec = ms % 1000,
            .tv_sec = ms / 1000,
    };

    p = (void *) &tv;

    rc = setsockopt(sock->fdt.fd, SOL_SOCKET, SO_RCVTIMEO, p, sizeof(tv));
    if (rc != 0) {
        sc_sock_errstr(sock, 0);
    }

    return rc == 0 ? SC_SOCK_OK : SC_SOCK_ERROR;
}

int sc_sock_set_sndtimeo(struct sc_sock *sock, int ms)
{
    int rc;
    void *p;

    struct timeval tv = {
            .tv_usec = ms % 1000,
            .tv_sec = ms / 1000,
    };

    p = (void *) &tv;

    rc = setsockopt(sock->fdt.fd, SOL_SOCKET, SO_SNDTIMEO, p, sizeof(tv));
    if (rc != 0) {
        sc_sock_errstr(sock, 0);
    }

    return rc == 0 ? SC_SOCK_OK : SC_SOCK_ERROR;
}

static int sc_sock_bind_unix(struct sc_sock *sock, const char *host)
{
    int rc;
    struct sockaddr_un addr = {.sun_family = AF_UNIX};

    strncpy(addr.sun_path, host, sizeof(addr.sun_path) - 1);
    sc_unlink(host);

    rc = bind(sock->fdt.fd, (struct sockaddr *) &addr, sizeof(addr));

    return rc == 0 ? 0 : -1;
}

static int sc_sock_bind(struct sc_sock *sock, const char *host, const char *prt)
{
    const int bf = SC_SOCK_BUF_SIZE;
    const socklen_t sz = sizeof(bf);

    int rc, rv = 0;
    struct addrinfo *servinfo = NULL;
    struct addrinfo hints = {.ai_family = sock->family,
                             .ai_socktype = SOCK_STREAM};

    *sock->err = '\0';

    if (sock->family == AF_UNIX) {
        sc_sock_int fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd == SC_INVALID) {
            goto error_unix;
        }

        sock->fdt.fd = fd;

        rc = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (void *) &bf, sz);
        if (rc != 0) {
            goto error_unix;
        }

        rc = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void *) &bf, sz);
        if (rc != 0) {
            goto error_unix;
        }

        rc = sc_sock_bind_unix(sock, host);
        if (rc != 0) {
            goto error_unix;
        }

        return 0;

error_unix:
        sc_sock_errstr(sock, 0);
        sc_sock_close(sock);

        return -1;
    }

    rc = getaddrinfo(host, prt, &hints, &servinfo);
    if (rc != 0) {
        sc_sock_errstr(sock, rc);
        return -1;
    }

    for (struct addrinfo *p = servinfo; p != NULL; p = p->ai_next) {
        void *tmp;
        const int tsz = sizeof(int);

        sc_sock_int fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd == SC_INVALID) {
            continue;
        }

        sock->fdt.fd = fd;

        if (sock->family == AF_INET6) {
            tmp = (void *) &(int){1};
            rc = setsockopt(sock->fdt.fd, IPPROTO_IPV6, IPV6_V6ONLY, tmp, tsz);
            if (rc != 0) {
                goto error;
            }
        }

        rc = sc_sock_set_blocking(sock, sock->blocking);
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

        rc = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (void *) &bf, sz);
        if (rc != 0) {
            goto error;
        }

        rc = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void *) &bf, sz);
        if (rc != 0) {
            goto error;
        }

        rc = bind(sock->fdt.fd, p->ai_addr, (socklen_t) p->ai_addrlen);
        if (rc == -1) {
            goto error;
        }

        goto out;
    }

error:
    sc_sock_errstr(sock, 0);
    sc_sock_close(sock);
    rv = -1;
out:
    freeaddrinfo(servinfo);

    return rv;
}

int sc_sock_finish_connect(struct sc_sock *sock)
{
    int result, rc;
    socklen_t len = sizeof(result);

    rc = getsockopt(sock->fdt.fd, SOL_SOCKET, SO_ERROR, (void *) &result, &len);
    if (rc != 0 || result != 0) {
        sc_sock_errstr(sock, 0);
        return SC_SOCK_ERROR;
    }

    return SC_SOCK_OK;
}

static int sc_sock_connect_unix(struct sc_sock *sock, const char *addr)
{
    int rc;
    const size_t len = strlen(addr);
    struct sockaddr_un addr_un = {.sun_family = AF_UNIX};

    if (len >= sizeof(addr_un.sun_path)) {
        return -1;
    }

    strcpy(addr_un.sun_path, addr);

    rc = connect(sock->fdt.fd, (struct sockaddr *) &addr_un, sizeof(addr_un));
    if (rc != 0) {
        sc_sock_errstr(sock, 0);
    }

    return rc;
}

int sc_sock_connect(struct sc_sock *sock, const char *dest_addr,
                    const char *dest_port, const char *source_addr,
                    const char *source_port)
{
    const int bf = SC_SOCK_BUF_SIZE;
    const socklen_t sz = sizeof(bf);

    int rc, rv = SC_SOCK_OK;
    struct addrinfo inf = {.ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM};
    struct addrinfo *servinfo = NULL, *bindinfo = NULL, *p, *s;

    if (sock->family == AF_UNIX) {
        sc_sock_int fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd == SC_INVALID) {
            goto error_unix;
        }

        sock->fdt.fd = fd;

        rc = setsockopt(sock->fdt.fd, SOL_SOCKET, SO_RCVBUF, (void *) &bf, sz);
        if (rc != 0) {
            goto error_unix;
        }

        rc = setsockopt(sock->fdt.fd, SOL_SOCKET, SO_SNDBUF, (void *) &bf, sz);
        if (rc != 0) {
            goto error_unix;
        }

        rc = sc_sock_connect_unix(sock, dest_addr);
        if (rc != 0) {
            goto error_unix;
        }

        return 0;

error_unix:
        sc_sock_errstr(sock, 0);
        sc_sock_close(sock);

        return -1;
    }

    rc = getaddrinfo(dest_addr, dest_port, &inf, &servinfo);
    if (rc != 0) {
        sc_sock_errstr(sock, rc);
        return -1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        void *tmp;
        sc_sock_int fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd == SC_INVALID) {
            continue;
        }

        sock->family = p->ai_family;
        sock->fdt.fd = fd;

        rc = sc_sock_set_blocking(sock, sock->blocking);
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

        rc = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (void *) &bf, sizeof(int));
        if (rc != 0) {
            goto error;
        }

        rc = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void *) &bf, sizeof(int));
        if (rc != 0) {
            goto error;
        }

        if (source_addr || source_port) {
            rc = getaddrinfo(source_addr, source_port, &inf, &bindinfo);
            if (rc != 0) {
                sc_sock_errstr(sock, rc);
                goto error_gai;
            }

            for (s = bindinfo; s != NULL; s = s->ai_next) {
                rc = bind(sock->fdt.fd, s->ai_addr, (socklen_t) s->ai_addrlen);
                if (rc != -1) {
                    break;
                }
            }

            freeaddrinfo(bindinfo);

            if (rc == -1) {
                goto error;
            }
        }

        rc = connect(sock->fdt.fd, p->ai_addr, (socklen_t) p->ai_addrlen);
        if (rc != 0) {
            if (!sock->blocking && (sc_sock_err() == SC_EINPROGRESS ||
                                    sc_sock_err() == SC_EAGAIN)) {
                rv = SC_SOCK_WANT_WRITE;
                goto end;
            }

            sc_sock_close(sock);
            continue;
        }

        goto end;
    }

error:
    sc_sock_errstr(sock, 0);
error_gai:
    sc_sock_close(sock);
    rv = -1;
end:
    freeaddrinfo(servinfo);

    return rv;
}

int sc_sock_send(struct sc_sock *sock, char *buf, int len, int flags)
{
    int n;

    if (len <= 0) {
        return len;
    }

retry:
    n = (int) send(sock->fdt.fd, buf, (size_t) len, flags);
    if (n == SC_ERR) {
        int err = sc_sock_err();
        if (err == SC_EINTR) {
            goto retry;
        }

        if (err == SC_EAGAIN) {
            return SC_SOCK_WANT_WRITE;
        }

        sc_sock_errstr(sock, 0);
        n = SC_SOCK_ERROR;
    }

    return n;
}

int sc_sock_recv(struct sc_sock *sock, char *buf, int len, int flags)
{
    int n;

    if (len <= 0) {
        return len;
    }

retry:
    n = (int) recv(sock->fdt.fd, buf, (size_t) len, flags);
    if (n == 0) {
        return SC_SOCK_ERROR;
    } else if (n == SC_ERR) {
        int err = sc_sock_err();
        if (err == SC_EINTR) {
            goto retry;
        }

        if (err == SC_EAGAIN) {
            return SC_SOCK_WANT_READ;
        }

        sc_sock_errstr(sock, 0);
        n = SC_SOCK_ERROR;
    }

    return n;
}

int sc_sock_accept(struct sc_sock *sock, struct sc_sock *in)
{
    const int bf = SC_SOCK_BUF_SIZE;
    const socklen_t sz = sizeof(bf);

    int rc;
    sc_sock_int fd;
    void *tmp;

    fd = accept(sock->fdt.fd, NULL, NULL);
    if (fd == SC_INVALID) {
        sc_sock_errstr(sock, 0);
        return SC_SOCK_ERROR;
    }

    in->fdt.fd = fd;
    in->fdt.op = SC_SOCK_NONE;
    in->family = sock->family;

    if (in->family != AF_UNIX) {
        tmp = (void *) &(int){1};
        rc = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, tmp, sizeof(int));
        if (rc != 0) {
            goto error;
        }
    }

    rc = sc_sock_set_blocking(in, sock->blocking);
    if (rc != 0) {
        goto error;
    }

    rc = setsockopt(in->fdt.fd, SOL_SOCKET, SO_RCVBUF, (void *) &bf, sz);
    if (rc != 0) {
        goto error;
    }

    rc = setsockopt(in->fdt.fd, SOL_SOCKET, SO_SNDBUF, (void *) &bf, sz);
    if (rc != 0) {
        goto error;
    }

    return SC_SOCK_OK;

error:
    sc_sock_errstr(sock, 0);
    sc_sock_close(in);

    return SC_SOCK_ERROR;
}

int sc_sock_listen(struct sc_sock *sock, const char *host, const char *port)
{
    int rc;

    rc = sc_sock_bind(sock, host, port);
    if (rc != 0) {
        return rc;
    }

    rc = listen(sock->fdt.fd, 4096);
    if (rc != 0) {
        sc_sock_errstr(sock, 0);
        sc_sock_close(sock);
    }

    return rc == 0 ? 0 : -1;
}

const char *sc_sock_error(struct sc_sock *sock)
{
    sock->err[sizeof(sock->err) - 1] = '\0';
    return sock->err;
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
        dst = sc_sock_addr(sock, AF_INET, &addr->sin_addr, tmp, sizeof(tmp));
        snprintf(buf, len, "%s:%d", dst, ntohs(addr->sin_port));
        break;

    case AF_INET6:
        addr6 = (struct sockaddr_in6 *) storage;
        dst = sc_sock_addr(sock, AF_INET6, &addr6->sin6_addr, tmp, sizeof(tmp));
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

    rc = setsockopt(listener, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *) &val,
                    sizeof(val));
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

    rv = closesocket(p->fds[0]);
    if (rv != 0) {
        rc = -1;
        sc_sock_pipe_set_err(p, "closesocket() : err(%d) ", WSAGetLastError());
    }

    rv = closesocket(p->fds[1]);
    if (rv != 0) {
        rc = -1;
        sc_sock_pipe_set_err(p, "closesocket() : err(%d) ", WSAGetLastError());
    }

    return rc;
}

int sc_sock_pipe_write(struct sc_sock_pipe *p, void *data, unsigned int len)
{
    int rc;

    rc = send(p->fds[1], data, len, 0);
    if (rc == SOCKET_ERROR || (unsigned int) rc != len) {
        sc_sock_pipe_set_err(p, "pipe send() : err(%d) ", WSAGetLastError());
    }

    return rc;
}

int sc_sock_pipe_read(struct sc_sock_pipe *p, void *data, unsigned int len)
{
    int rc;

    rc = recv(p->fds[0], (char *) data, len, 0);
    if (rc == SOCKET_ERROR || (unsigned int) rc != len) {
        sc_sock_pipe_set_err(p, "pipe recv() : err(%d) ", WSAGetLastError());
    }

    return rc;
}

#else

int sc_sock_pipe_init(struct sc_sock_pipe *p, int type)
{
    int rc;

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
        sc_sock_poll_set_err(p, "Out of memory.");
        goto error;
    }

    fds = epoll_create1(0);
    if (fds == -1) {
        sc_sock_poll_set_err(p, "epoll_create1(): %s ", strerror(errno));
        goto error;
    }

    p->cap = 16;
    p->fds = fds;

    return 0;
error:
    sc_sock_free(p->events);
    p->events = NULL;
    p->fds = -1;

    return -1;
}

int sc_sock_poll_term(struct sc_sock_poll *p)
{
    sc_sock_free(p->events);
    return close(p->fds);
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

    struct epoll_event ep_ev = {.data.ptr = data,
                                .events = EPOLLERR | EPOLLHUP | EPOLLRDHUP};

    if ((fdt->op & events) == events) {
        return SC_SOCK_OK;
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
    struct epoll_event ep_ev = {.data.ptr = data,
                                .events = EPOLLERR | EPOLLHUP | EPOLLRDHUP};

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
    uint32_t events = 0;
    uint32_t epoll_events = p->events[i].events;

    if (epoll_events & EPOLLIN) {
        events |= SC_SOCK_READ;
    }

    if (epoll_events & EPOLLOUT) {
        events |= SC_SOCK_WRITE;
    }

    epoll_events &= EPOLLHUP | EPOLLRDHUP | EPOLLERR;
    if (epoll_events != 0) {
        events = (SC_SOCK_READ | SC_SOCK_WRITE);
    }

    return events;
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
        sc_sock_poll_set_err(p, "Out of memory.");
        goto error;
    }

    fds = kqueue();
    if (fds == -1) {
        sc_sock_poll_set_err(p, "kqueue(): %s ", strerror(errno));
        return -1;
    }

    p->cap = 16;
    p->fds = fds;

    return 0;
error:
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

int sc_sock_poll_term(struct sc_sock_poll *p)
{
    sc_sock_free(p->events);
    return close(p->fds);
}

int sc_sock_poll_add(struct sc_sock_poll *p, struct sc_sock_fd *fdt,
                     enum sc_sock_ev events, void *data)
{
    int rc, count = 0;
    struct kevent ev[2];
    int mask = fdt->op | events;

    if ((fdt->op & events) == events) {
        return SC_SOCK_OK;
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
        sc_sock_poll_set_err(p, "Out of memory.");
        return -1;
    }

    p->data = sc_sock_malloc(sizeof(void *) * 16);
    if (p->data == NULL) {
        sc_sock_free(p->events);
        sc_sock_poll_set_err(p, "Out of memory.");
        return -1;
    }

    p->cap = 16;

    for (int i = 0; i < p->cap; i++) {
        p->events[i].fd = SC_INVALID;
    }

    return 0;
}

int sc_sock_poll_term(struct sc_sock_poll *p)
{
    sc_sock_free(p->events);
    sc_sock_free(p->data);

    return 0;
}

static int sc_sock_poll_expand(struct sc_sock_poll *p)
{
    int cap, rc = 0;
    void **data = NULL;
    struct pollfd *ev = NULL;

    if (p->count == p->cap) {
        if (p->cap >= SC_SIZE_MAX / 2) {
            goto error;
        }

        cap = p->cap * 2;
        ev = sc_sock_realloc(p->events, cap * sizeof(*ev));
        if (ev == NULL) {
            goto error;
        }

        data = sc_sock_realloc(p->data, cap * sizeof(*data));
        if (data == NULL) {
            goto error;
        }

        p->events = ev;
        p->data = data;

        for (int i = p->cap; i < cap; i++) {
            p->events[i].fd = SC_INVALID;
        }

        p->cap = cap;
    }

    return rc;

error:
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
        return SC_SOCK_OK;
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
    uint32_t events = 0;
    uint32_t epoll_events = p->events[i].revents;

    if (epoll_events & POLLIN) {
        events |= SC_SOCK_READ;
    }

    if (epoll_events & POLLOUT) {
        events |= SC_SOCK_WRITE;
    }

    epoll_events &= POLLHUP | POLLERR;
    if (epoll_events != 0) {
        events = (SC_SOCK_READ | SC_SOCK_WRITE);
    }

    return events;
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

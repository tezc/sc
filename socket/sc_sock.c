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

#include "sc_sock.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#if defined(_WIN32) || defined(_WIN64)
#include <Ws2tcpip.h>
#include <afunix.h>

#pragma warning(disable : 4996)
#define SC_UNLINK(n)   DeleteFileA(n)
#define SC_ERR         SOCKET_ERROR
#define SC_INVALID     INVALID_SOCKET
#define SC_EAGAIN      WSAEWOULDBLOCK
#define SC_EINPROGRESS WSAEINPROGRESS
#define SC_EINTR       WSAEINTR
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <unistd.h>

#define SC_UNLINK(n)   unlink(n)
#define SC_ERR         (-1)
#define SC_INVALID     (-1)
#define SC_EAGAIN      EAGAIN
#define SC_EINPROGRESS EINPROGRESS
#define SC_EINTR       EINTR
#endif

static int sc_sock_err()
{
#if defined(_WIN32) || defined(_WIN64)
    return WSAGetLastError();
#else
    return errno;
#endif
}

static void sc_sock_errstr(struct sc_sock* sock, int gai_err)
{
#if defined(_WIN32) || defined(_WIN64)
    int rc;
    DWORD err = GetLastError();
    LPSTR errstr = 0;

    rc = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, err, 0, (LPSTR)&errstr, 0, NULL);
    if (rc != 0) {
        strncpy(sock->err, errstr, sizeof(sock->err) - 1);
        LocalFree(errstr);
    }

#else
    const char* str = gai_err ? gai_strerror(gai_err) : strerror(errno);
    strncpy(sock->err, str, sizeof(sock->err) - 1);
#endif
}


struct sc_sock* sc_sock_create(int type, bool blocking, int family)
{
    struct sc_sock* socket;

    socket = malloc(sizeof(struct sc_sock));
    if (socket == NULL) {
        return NULL;
    }

    sc_sock_init(socket, type, blocking, family);

    return socket;
}

int sc_sock_destroy(struct sc_sock* sock)
{
    int rc;

    rc = sc_sock_term(sock);
    free(sock);

    return rc;
}

void sc_sock_init(struct sc_sock* sock, int type, bool blocking, int family)
{
    sock->type = type;
    sock->blocking = blocking;
    sock->family = family;
    sock->op = SC_SOCK_OP_UNREGISTERED;
    sock->fd = -1;
    memset(sock->err, 0, sizeof(sock->err));
}

static int sc_sock_close(struct sc_sock* sock)
{
    int rc = 0;

    if (sock->fd != -1) {
#if defined(_WIN32) || defined(_WIN64)
        rc = closesocket(sock->fd);
#else
        rc = close(sock->fd);
#endif
        sock->fd = -1;
    }

    return (rc == 0) ? 0 : -1;
}

int sc_sock_term(struct sc_sock* sock)
{
    int rc;

    rc = sc_sock_close(sock);
    if (rc != 0) {
        sc_sock_errstr(sock, 0);
    }

    return rc;
}

static int sc_sock_set_blocking(struct sc_sock* sock, bool blocking)
{
#ifdef _WIN32
    int mode = blocking ? 0 : 1;
    return (ioctlsocket(sock->fd, FIONBIO, &mode) == 0) ? 0 : -1;
#else
    int flags = fcntl(sock->fd, F_GETFL, 0);
    if (flags == -1) {
        return -1;
    }

    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return (fcntl(sock->fd, F_SETFL, flags) == 0) ? 0 : -1;
#endif
}

static int sc_sock_bind_unix(struct sc_sock* sock, const char* host)
{
    int rc;
    struct sockaddr_un addr = { .sun_family = AF_UNIX };

    strncpy(addr.sun_path, host, sizeof(addr.sun_path) - 1);
    SC_UNLINK(host);

    rc = bind(sock->fd, (struct sockaddr*)&addr, sizeof(addr));

    return rc == 0 ? 0 : -1;
}

static int sc_sock_bind(struct sc_sock* sock, const char* host, const char* prt)
{
    const int bf = SC_SOCK_BUF_SIZE;
    const int sz = sizeof(bf);

    int rc = 0, rv = 0;
    struct addrinfo* servinfo = NULL;
    struct addrinfo hints = { .ai_family = sock->family,
                             .ai_socktype = SOCK_STREAM };

    *sock->err = '\0';

    if (sock->family == AF_UNIX) {
        sc_sock_int fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd == SC_INVALID) {
            goto error_unix;
        }

        sock->fd = fd;

        rc = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (void*)&bf, sz);
        if (rc != 0) {
            goto error_unix;
        }

        rc = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void*)&bf, sz);
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

    for (struct addrinfo* p = servinfo; p != NULL; p = p->ai_next) {
        sc_sock_int fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd == SC_INVALID) {
            continue;
        }

        sock->fd = fd;

        if (sock->family == AF_INET6) {
            rc = setsockopt(sock->fd, IPPROTO_IPV6, IPV6_V6ONLY, &(int){1},
                sizeof(int));
            if (rc != 0) {
                goto error;
            }
        }

        rc = sc_sock_set_blocking(sock, sock->blocking);
        if (rc != 0) {
            goto error;
        }

        rc = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
        if (rc != 0) {
            goto error;
        }

        rc = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &(int){1}, sizeof(int));
        if (rc != 0) {
            goto error;
        }

        rc = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (void*)&bf, sz);
        if (rc != 0) {
            goto error;
        }

        rc = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void*)&bf, sz);
        if (rc != 0) {
            goto error;
        }

        rc = bind(sock->fd, p->ai_addr, p->ai_addrlen);
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

int sc_sock_finish_connect(struct sc_sock* sock)
{
    int result, rc;
    socklen_t len = sizeof(result);

    rc = getsockopt(sock->fd, SOL_SOCKET, SO_ERROR, (void*)&result, &len);
    if (rc != 0 || result != 0) {
        sc_sock_errstr(sock, 0);
        sc_sock_close(sock);
        rc = -1;
    }

    return rc;
}

static int sc_sock_connect_unix(struct sc_sock* sock, const char* addr)
{
    int rc, err;
    const size_t len = strlen(addr);
    struct sockaddr_un addr_un = { .sun_family = AF_UNIX };

    if (len >= sizeof(addr_un.sun_path)) {
        return -1;
    }

    strcpy(addr_un.sun_path, addr);

    rc = connect(sock->fd, (struct sockaddr*)&addr_un, sizeof(addr_un));
    if (rc != 0) {
        err = sc_sock_err();
        if (!sock->blocking && ((err == SC_EINTR || err == SC_EINPROGRESS))) {
            return 0;
        }
        sc_sock_errstr(sock, 0);
    }

    return rc;
}

int sc_sock_connect(struct sc_sock* sock, const char* addr, const char* port,
    const char* source_addr, const char* source_port)
{
    const int bf = SC_SOCK_BUF_SIZE;
    const int sz = sizeof(bf);

    int rc = 0, rv = 0;
    struct addrinfo inf = { .ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM };
    struct addrinfo* servinfo = NULL, * bindinfo = NULL, * p, * s;

    if (sock->family == AF_UNIX) {
        sc_sock_int fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd == SC_INVALID) {
            goto error_unix;
        }

        sock->fd = fd;

        rc = setsockopt(sock->fd, SOL_SOCKET, SO_RCVBUF, (void*)&bf, sz);
        if (rc != 0) {
            goto error_unix;
        }

        rc = setsockopt(sock->fd, SOL_SOCKET, SO_SNDBUF, (void*)&bf, sz);
        if (rc != 0) {
            goto error_unix;
        }

        rc = sc_sock_connect_unix(sock, addr);
        if (rc != 0) {
            goto error_unix;
        }

        return 0;

    error_unix:
        sc_sock_errstr(sock, 0);
        sc_sock_close(sock);

        return -1;
    }

    rc = getaddrinfo(addr, port, &inf, &servinfo);
    if (rc != 0) {
        sc_sock_errstr(sock, rc);
        return -1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        sc_sock_int fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd == SC_INVALID) {
            continue;
        }

        sock->family = p->ai_family;
        sock->fd = fd;

        rc = sc_sock_set_blocking(sock, sock->blocking);
        if (rc != 0) {
            goto error;
        }

        rc = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
        if (rc != 0) {
            goto error;
        }

        rc = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &(int){1}, sizeof(int));
        if (rc != 0) {
            goto error;
        }

        rc = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (void*)&bf, sizeof(int));
        if (rc != 0) {
            goto error;
        }

        rc = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void*)&bf, sizeof(int));
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
                rc = bind(sock->fd, s->ai_addr, s->ai_addrlen);
                if (rc != -1) {
                    break;
                }
            }

            freeaddrinfo(bindinfo);

            if (rc == -1) {
                goto error;
            }
        }

        rc = connect(sock->fd, p->ai_addr, p->ai_addrlen);
        if (rc != 0) {
            if (!sock->blocking && (sc_sock_err() == SC_EINPROGRESS ||
                sc_sock_err() == SC_EAGAIN)) {
                rv = 0;
                goto end;
            }

            sc_sock_close(sock);
            continue;
        }

        goto end;
    }

    if (p == NULL) {
        goto error;
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

int sc_sock_send(struct sc_sock* sock, char* buf, int len)
{
    int n;

    assert(len > 0);

    if (len <= 0) {
        return len;
    }

retry:
    n = send(sock->fd, buf, len, 0);
    if (n == SC_ERR) {
        int err = sc_sock_err();
        if (err == SC_EINTR) {
            goto retry;
        }

        if (!sock->blocking && err == SC_EAGAIN) {
            return 0;
        }

        sc_sock_errstr(sock, 0);
    }

    return n;
}

int sc_sock_recv(struct sc_sock* sock, char* buf, int len)
{
    int n;

    assert(len > 0);

    if (len <= 0) {
        return len;
    }

retry:
    n = recv(sock->fd, buf, len, 0);
    if (n == 0) {
        return -1;
    }
    else if (n == SC_ERR) {
        int err = sc_sock_err();
        if (err == SC_EINTR) {
            goto retry;
        }

        if (!sock->blocking && err == SC_EAGAIN) {
            return 0;
        }

        sc_sock_errstr(sock, 0);
    }

    return n;
}

int sc_sock_accept(struct sc_sock* sock, struct sc_sock* in)
{
    const int bf = SC_SOCK_BUF_SIZE;
    const int sz = sizeof(bf);

    int rc;
    sc_sock_int fd;

    fd = accept(sock->fd, NULL, NULL);
    if (fd == SC_INVALID) {
        sc_sock_errstr(sock, 0);
        return -1;
    }

    in->op = SC_SOCK_OP_UNREGISTERED;
    in->fd = fd;
    in->family = sock->family;

    if (in->family != AF_UNIX) {
        rc = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &(int){1}, sizeof(int));
        if (rc != 0) {
            goto error;
        }
    }

    rc = sc_sock_set_blocking(in, sock->blocking);
    if (rc != 0) {
        goto error;
    }

    rc = setsockopt(in->fd, SOL_SOCKET, SO_RCVBUF, (void*)&bf, sz);
    if (rc != 0) {
        goto error;
    }

    rc = setsockopt(in->fd, SOL_SOCKET, SO_SNDBUF, (void*)&bf, sz);
    if (rc != 0) {
        goto error;
    }

    return 0;

error:
    sc_sock_errstr(sock, 0);
    sc_sock_close(in);

    return -1;
}

int sc_sock_listen(struct sc_sock* sock, const char* host, const char* port)
{
    int rc;

    rc = sc_sock_bind(sock, host, port);
    if (rc != 0) {
        return rc;
    }

    rc = listen(sock->fd, 4096);
    if (rc != 0) {
        sc_sock_errstr(sock, 0);
    }

    return rc == 0 ? 0 : -1;
}

const char* sc_sock_error(struct sc_sock* sock)
{
    sock->err[sizeof(sock->err) - 1]  = '\0';
    return sock->err;
}

static const char* sc_sock_addr(struct sc_sock* sock, int af, void* cp,
    char* buf, int len)
{
    const char* dest;

    dest = inet_ntop(af, cp, buf, len);
    if (dest == NULL) {
        sc_sock_errstr(sock, 0);
        *buf = '\0';
    }

    return dest;
}

static const char* sc_sock_print_storage(struct sc_sock* sock,
    struct sockaddr_storage* storage,
    char* buf, int len)
{
    const char* dst;
    struct sockaddr_in* addr;
    struct sockaddr_in6* addr6;
    struct sockaddr_un* addr_un;
    char tmp[INET6_ADDRSTRLEN];

    *buf = '\0';

    switch (storage->ss_family) {
    case AF_INET:
        addr = (struct sockaddr_in*)storage;
        dst = sc_sock_addr(sock, AF_INET, &addr->sin_addr, tmp, sizeof(tmp));
        snprintf(buf, len, "%s:%d", dst, ntohs(addr->sin_port));
        break;

    case AF_INET6:
        addr6 = (struct sockaddr_in6*)storage;
        dst = sc_sock_addr(sock, AF_INET6, &addr6->sin6_addr, tmp, sizeof(tmp));
        snprintf(buf, len, "%s:%d", dst, ntohs(addr6->sin6_port));
        break;

    case AF_UNIX:
        addr_un = (struct sockaddr_un*)storage;
        snprintf(buf, len, "%s", addr_un->sun_path);
        break;

    default:
        snprintf(buf, len, "Unknown family : %d \n", storage->ss_family);
        break;
    }

    return buf;
}

static const char* sc_sock_local_str(struct sc_sock* sock, char* buf, int len)
{
    int rc;
    struct sockaddr_storage st;
    socklen_t storage_len = sizeof(st);

    rc = getsockname(sock->fd, (struct sockaddr*)&st, &storage_len);
    if (rc != 0) {
        sc_sock_errstr(sock, 0);
        *buf = '\0';
        return NULL;
    }

    return sc_sock_print_storage(sock, &st, buf, len);
}

static const char* sc_sock_remote_str(struct sc_sock* sock, char* buf, int len)
{
    int rc;
    struct sockaddr_storage st;
    socklen_t storage_len = sizeof(st);

    rc = getpeername(sock->fd, (struct sockaddr*)&st, &storage_len);
    if (rc != 0) {
        sc_sock_errstr(sock, 0);
        *buf = '\0';
        return NULL;
    }

    return sc_sock_print_storage(sock, &st, buf, len);
}

void sc_sock_print(struct sc_sock* sock, char* buf, int len)
{
    char l[128];
    char r[128];

    sc_sock_local_str(sock, l, sizeof(l));
    sc_sock_remote_str(sock, r, sizeof(r));

    snprintf(buf, len, "Local(%s), Remote(%s) ", l, r);
}

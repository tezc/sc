#define SY_HAVE_BACKTRACE
#include "sc_signal.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <Ws2tcpip.h>
    #include <windows.h>
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")

typedef SOCKET sc_sock_int;

#else
    #include <sys/socket.h>
    #include <unistd.h>
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

struct sc_sock_fd
{
    sc_sock_int fd;
    enum sc_sock_ev op;
    int type;
    int index;
};

struct sc_sock_pipe
{
    struct sc_sock_fd fdt;
    sc_sock_int fds[2];
};

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
    return -1;
}

int sc_sock_pipe_term(struct sc_sock_pipe *p)
{
    int rc = 0, rv;

    rv = closesocket(p->fds[0]);
    if (rv != 0) {
        rc = -1;
    }

    rv = closesocket(p->fds[1]);
    if (rv != 0) {
        rc = -1;
    }

    return rc;
}

int sc_sock_pipe_write(struct sc_sock_pipe *p, void *data, int len)
{
    int rc;

    rc = send(p->fds[1], data, len, 0);

    return rc;
}

int sc_sock_pipe_read(struct sc_sock_pipe *p, void *data, int len)
{
    int rc;

    rc = recv(p->fds[0], (char *) data, len, 0);

    return rc;
}

#else

int sc_sock_pipe_init(struct sc_sock_pipe *p, int type)
{
    int rc;

    rc = pipe(p->fds);
    if (rc != 0) {
        return -1;
    }

    p->fdt.type = type;
    p->fdt.op = 0;
    p->fdt.fd = p->fds[0];

    return 0;
}

int sc_sock_pipe_term(struct sc_sock_pipe *p)
{
    int rc = 0, rv;

    rv = close(p->fds[0]);
    if (rv != 0) {
        rc = -1;
    }

    rv = close(p->fds[1]);
    if (rv != 0) {
        rc = -1;
    }

    return rc;
}

int sc_sock_pipe_write(struct sc_sock_pipe *p, void *data, int len)
{
    ssize_t n;
    char *b = data;

retry:
    n = write(p->fds[1], b, len);
    if (n == -1 && errno == EINTR) {
        goto retry;
    }

    if (n > 0 && n != len) {
        len -= n;
        b += n;
        goto retry;
    }

    return n;
}

int sc_sock_pipe_read(struct sc_sock_pipe *p, void *data, int len)
{
    ssize_t n;
    char *b = data;

retry:
    n = read(p->fds[0], b, len);
    if (n == -1 && errno == EINTR) {
        goto retry;
    }

    if (n > 0 && n != len) {
        len -= n;
        b += n;
        goto retry;
    }

    return n;
}

#endif
void test1()
{
    int rc;
    char buf[32];
    struct sc_sock_pipe pipe;

    rc = sc_signal_init();
    assert(rc == 0);

    rc = sc_sock_pipe_init(&pipe, 0);
    assert(rc == 0);

    sc_signal_shutdown_fd = pipe.fds[1];
    raise(SIGINT);

    rc = sc_sock_pipe_read(&pipe, buf, 1);
    assert(rc == 1);

    rc = sc_sock_pipe_term(&pipe);
    assert(rc == 0);
}


int main()
{
#if defined(_WIN32) || defined(_WIN64)
    WSADATA data;

    int rc = WSAStartup(MAKEWORD(2, 2), &data);
    assert(rc == 0);
    assert(LOBYTE(data.wVersion) == 2 && HIBYTE(data.wVersion) == 2);
#endif

    test1();

#if defined(_WIN32) || defined(_WIN64)
    rc = WSACleanup();
    assert(rc == 0);
#endif
    return 0;
}

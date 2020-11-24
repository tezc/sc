
#include "sc_sock.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(_WIN64)
#include <synchapi.h>
#define sleep(n) (Sleep(n * 1000))
#endif

#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>

struct sc_thread
{
    HANDLE id;
    void* (*fn)(void*);
    void* arg;
    void* ret;
};

#else

#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

struct sc_thread
{
    pthread_t id;
};

#endif

void sc_thread_init(struct sc_thread* thread);
int sc_thread_term(struct sc_thread* thread);
int sc_thread_start(struct sc_thread* thread, void* (*fn)(void*), void* arg);
int sc_thread_stop(struct sc_thread* thread, void** ret);

void sc_thread_init(struct sc_thread* thread)
{
    thread->id = 0;
}

#if defined(_WIN32) || defined(_WIN64)
#include <process.h>

unsigned int __stdcall sc_thread_fn(void* arg)
{
    struct sc_thread* thread = arg;

    thread->ret = thread->fn(thread->arg);
    return 0;
}

int sc_thread_start(struct sc_thread* thread, void* (*fn)(void*), void* arg)
{
    int rc;

    thread->fn = fn;
    thread->arg = arg;

    thread->id =
        (HANDLE)_beginthreadex(NULL, 0, sc_thread_fn, thread, 0, NULL);
    rc = thread->id == 0 ? -1 : 0;

    return rc;
}

int sc_thread_stop(struct sc_thread* thread, void** ret)
{
    int rc = 0;
    DWORD rv;
    BOOL brc;

    if (thread->id == 0) {
        return -1;
    }

    rv = WaitForSingleObject(thread->id, INFINITE);
    if (rv == WAIT_FAILED) {
        rc = -1;
    }

    brc = CloseHandle(thread->id);
    if (!brc) {
        rc = -1;
    }

    thread->id = 0;
    if (ret != NULL) {
        *ret = thread->ret;
    }

    return rc;
}
#else

int sc_thread_start(struct sc_thread* thread, void* (*fn)(void*), void* arg)
{
    int rc;
    pthread_attr_t hndl;

    rc = pthread_attr_init(&hndl);
    if (rc != 0) {
        return -1;
    }

    // This may only fail with EINVAL.
    pthread_attr_setdetachstate(&hndl, PTHREAD_CREATE_JOINABLE);

    rc = pthread_create(&thread->id, &hndl, fn, arg);

    // This may only fail with EINVAL.
    pthread_attr_destroy(&hndl);

    return rc;
}

int sc_thread_stop(struct sc_thread* thread, void** ret)
{
    int rc;
    void* val;

    if (thread->id == 0) {
        return -1;
    }

    rc = pthread_join(thread->id, &val);
    thread->id = 0;

    if (ret != NULL) {
        *ret = val;
    }

    return rc;
}

#endif

int sc_thread_term(struct sc_thread* thread)
{
    return sc_thread_stop(thread, NULL);
}

void* server_ip4(void* arg)
{
    char buf[5];
    char tmp[128];
    struct sc_sock sock, accepted;

    sc_sock_init(&sock, 0, true, AF_INET);
    assert(sc_sock_listen(&sock, "127.0.0.1", "8004") == 0);
    sc_sock_print(&sock, tmp, sizeof(tmp));
    assert(strcmp(tmp, "Local(127.0.0.1:8004), Remote() ") == 0);

    assert(sc_sock_accept(&sock, &accepted) == 0);
    assert(sc_sock_recv(&accepted, buf, 5) == 5);
    assert(strcmp("test", buf) == 0);

    assert(sc_sock_term(&sock) == 0);
    assert(sc_sock_term(&accepted) == 0);

    return NULL;
}

void* client_ip4(void* arg)
{
    int rc;
    char tmp[128];
    struct sc_sock sock;

    sc_sock_init(&sock, 0, true, AF_INET);

    for (int i = 0; i < 5; i++) {
        rc = sc_sock_connect(&sock, "127.0.0.1", "8004", NULL, NULL);
        if (rc == 0) {
            break;
        }

        sleep(1);
    }

    assert(rc == 0);
    sc_sock_print(&sock, tmp, sizeof(tmp));
    assert(sc_sock_send(&sock, "test", 5) == 5);
    assert(sc_sock_term(&sock) == 0);

    return NULL;
}

void test_ip4()
{
    struct sc_thread thread1;
    struct sc_thread thread2;

    sc_thread_init(&thread1);
    sc_thread_init(&thread2);

    assert(sc_thread_start(&thread1, server_ip4, NULL) == 0);
    assert(sc_thread_start(&thread2, client_ip4, NULL) == 0);

    assert(sc_thread_term(&thread1) == 0);
    assert(sc_thread_term(&thread2) == 0);
}

void* server_ip6(void* arg)
{
    char buf[5];
    char tmp[128];
    struct sc_sock sock, accepted;

    sc_sock_init(&sock, 0, true, AF_INET6);
    assert(sc_sock_listen(&sock, "::1", "8006") == 0);
    sc_sock_print(&sock, tmp, sizeof(tmp));
    assert(strcmp(tmp, "Local(::1:8006), Remote() ") == 0);
    assert(sc_sock_accept(&sock, &accepted) == 0);
    assert(sc_sock_recv(&accepted, buf, 5) == 5);
    assert(strcmp("test", buf) == 0);

    assert(sc_sock_term(&sock) == 0);
    assert(sc_sock_term(&accepted) == 0);

    return NULL;
}

void* client_ip6(void* arg)
{
    int rc;
    struct sc_sock sock;

    sc_sock_init(&sock, 0, true, AF_INET6);

    for (int i = 0; i < 5; i++) {
        rc = sc_sock_connect(&sock, "::1", "8006", NULL, NULL);
        if (rc == 0) {
            break;
        }

        sleep(1);
    }
    assert(rc == 0);
    assert(sc_sock_send(&sock, "test", 5) == 5);

    assert(sc_sock_term(&sock) == 0);

    return NULL;
}

void test_ip6()
{
    struct sc_thread thread1;
    struct sc_thread thread2;

    sc_thread_init(&thread1);
    sc_thread_init(&thread2);

    assert(sc_thread_start(&thread1, server_ip6, NULL) == 0);
    assert(sc_thread_start(&thread2, client_ip6, NULL) == 0);

    assert(sc_thread_term(&thread1) == 0);
    assert(sc_thread_term(&thread2) == 0);
}

void* server_unix(void* arg)
{
    char buf[5];
    char tmp[128];
    struct sc_sock sock, accepted;

    sc_sock_init(&sock, 0, true, AF_UNIX);
    assert(sc_sock_listen(&sock, "x.sock", NULL) == 0);
    sc_sock_print(&sock, tmp, sizeof(tmp));
    assert(strcmp(tmp, "Local(x.sock), Remote() ") == 0);
    assert(sc_sock_accept(&sock, &accepted) == 0);
    assert(sc_sock_recv(&accepted, buf, 5) == 5);
    assert(strcmp("test", buf) == 0);

    assert(sc_sock_term(&sock) == 0);
    assert(sc_sock_term(&accepted) == 0);

    return NULL;
}

void* client_unix(void* arg)
{
    int rc;
    struct sc_sock sock;

    sleep(3);
    sc_sock_init(&sock, 0, true, AF_UNIX);
    for (int i = 0; i < 10; i++) {
        rc = sc_sock_connect(&sock, "x.sock", NULL, NULL, NULL);
        if (rc == 0) {
            break;
        }

        sleep(1);
    }

    assert(rc == 0);
    assert(sc_sock_send(&sock, "test", 5) == 5);
    assert(sc_sock_term(&sock) == 0);

    return NULL;
}


void test_unix()
{
    struct sc_thread thread1;
    struct sc_thread thread2;

    sc_thread_init(&thread1);
    sc_thread_init(&thread2);

    assert(sc_thread_start(&thread1, server_unix, NULL) == 0);
    assert(sc_thread_start(&thread2, client_unix, NULL) == 0);

    assert(sc_thread_term(&thread1) == 0);
    assert(sc_thread_term(&thread2) == 0);
}


void test1()
{
    char tmp[5];
    struct sc_sock sock;

    sc_sock_init(&sock, 0, false, AF_INET);
    assert(sc_sock_connect(&sock, "3127.0.0.1", "2131", NULL, NULL) != 0);
    assert(sc_sock_finish_connect(&sock) != 0);
    assert(sc_sock_connect(&sock, "3127.0.0.1", "2131", "127.90.1.1", "50") !=
           0);
    assert(sc_sock_finish_connect(&sock) != 0);
    assert(sc_sock_send(&sock, "test", 5) == -1);
    assert(sc_sock_recv(&sock, tmp, sizeof(tmp)) == -1);
    assert(sc_sock_connect(&sock, "131s::1", "2131", "::1", "50") != 0);
    assert(sc_sock_finish_connect(&sock) != 0);
    assert(sc_sock_connect(&sock, "d::1", "2131", "dsad", "50") != 0);
    assert(sc_sock_finish_connect(&sock) != 0);
    assert(sc_sock_connect(&sock, "dsadas", "2131", "::1", "50") != 0);

    sc_sock_term(&sock);
}

void test_pipe(void)
{
    char buf[5];
    struct sc_sock_pipe pipe;

    sc_sock_pipe_init(&pipe, 0);
    sc_sock_pipe_write(&pipe, "test", 5);
    sc_sock_pipe_read(&pipe, buf, 5);
    sc_sock_pipe_term(&pipe);

    assert(strcmp("test", buf) == 0);
}

#ifdef SC_HAVE_WRAP
#include <unistd.h>

bool fail_close = false;
int __real_close(int fd);
int __wrap_close(int fd)
{
    if (fail_close) {
        return -1;
    }

    return __real_close(fd);
}

bool fail_pipe = false;
int __real_pipe(int __pipedes[2]);
int __wrap_pipe(int __pipedes[2])
{
    if (fail_pipe) {
        return -1;
    }

    return __real_pipe(__pipedes);
}

void pipe_fail_test()
{
    struct sc_sock_pipe pipe;
    fail_pipe = true;
    assert(sc_sock_pipe_init(&pipe, 0) != 0);
    fail_pipe = false;
    assert(sc_sock_pipe_init(&pipe, 0) == 0);
    fail_close = true;
    assert(sc_sock_pipe_term(&pipe) == -1);
    fail_close = false;
    assert(sc_sock_pipe_term(&pipe) == 0);
}

#else
void pipe_fail_test()
{
}

#endif


void* server(void* arg)
{
    int write_triggered = 2;
    int rc, received = 0;
    struct sc_sock server;
    struct sc_sock accepted;
    struct sc_sock_poll poll;


    assert(sc_sock_poll_init(&poll) == 0);
    assert(sc_sock_poll_term(&poll) == 0);
    assert(sc_sock_poll_init(&poll) == 0);

    sc_sock_init(&server, 0, true, SC_SOCK_INET);
    rc = sc_sock_listen(&server, "127.0.0.1", "11000");
    assert(rc == 0);

    rc = sc_sock_poll_add(&poll, &server.fdt, SC_SOCK_READ, &server);
    assert(rc == 0);
    bool done = false;

    while (!done) {
        rc = sc_sock_poll_wait(&poll, -1);
        assert(rc != -1);

        for (int i = 0; i < rc; i++) {
            int ev = sc_sock_poll_event(&poll, i);
            struct sc_sock* sock = sc_sock_poll_data(&poll, i);

            if (ev == 0) {
                continue;
            }

            if (sock == &server) {
                if (ev & SC_SOCK_READ) {
                    rc = sc_sock_accept(&server, &accepted);

                    assert(rc == 0);
                    assert(sc_sock_poll_add(&poll, &accepted.fdt,
                                            SC_SOCK_WRITE,
                                            &accepted) == 0);
                }

                if (ev & SC_SOCK_WRITE) {
                    assert(false);
                }
            }
            else {
                if (ev & SC_SOCK_READ) {
                    char buf[8];
                    rc = sc_sock_recv(&accepted, buf, sizeof(buf));
                    if (rc == 8) {
                        assert(strcmp(buf, "dataxxx") == 0);
                        received++;
                    }
                    else if (rc == 0 || rc < 0) {
                        rc = sc_sock_poll_del(&poll, &accepted.fdt,
                                              SC_SOCK_READ |
                                              SC_SOCK_WRITE,
                                              &accepted);
                        assert(rc == 0);
                        rc = sc_sock_term(&accepted);
                        assert(rc == 0);
                        done = true;
                        break;
                    }
                }

                if (write_triggered > 0) {
                    write_triggered--;
                    if (write_triggered == 0) {
                        rc = sc_sock_poll_del(&poll, &accepted.fdt, SC_SOCK_WRITE, &accepted);
                        assert(rc == 0);
                    }
                    else {
                        rc = sc_sock_poll_add(&poll, &accepted.fdt, SC_SOCK_READ, &accepted);
                    }
                }
            }
        }
    }

    assert(write_triggered == 0);
    assert(sc_sock_term(&server) == 0);
    assert(sc_sock_poll_term(&poll) == 0);

    printf("Received :%d \n", received);
    assert(received == 10000);


    return NULL;
}

void* client(void* arg)
{
    int rc;
    struct sc_sock sock;

    for (int i = 0; i < 10; i++) {
        sc_sock_init(&sock, 0, true, SC_SOCK_INET);

        rc = sc_sock_connect(&sock, "127.0.0.1", "11000", NULL, NULL);
        if (rc != 0) {
            assert(sc_sock_term(&sock) == 0);
            sleep(1);
            continue;
        }

        for (int j = 0; j < 10000; j++) {
            rc = sc_sock_send(&sock, "dataxxx", 8);
            assert(rc == 8);
        }

        rc = sc_sock_term(&sock);
        assert(rc == 0);
        break;
    }

    return NULL;
}


void test_poll()
{
    struct sc_thread thread1;
    struct sc_thread thread2;

    sc_thread_init(&thread1);
    sc_thread_init(&thread2);

    assert(sc_thread_start(&thread1, server, NULL) == 0);
    assert(sc_thread_start(&thread2, client, NULL) == 0);

    assert(sc_thread_term(&thread1) == 0);
    assert(sc_thread_term(&thread2) == 0);
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
    test_ip4();
    test_ip6();
    test_unix();
    test_pipe();
    pipe_fail_test();
    test_poll();

#if defined(_WIN32) || defined(_WIN64)
    rc = WSACleanup();
    assert(rc == 0);
#endif
    return 0;
}


#include "sc_sock.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

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
    assert(sc_sock_listen(&sock, "127.0.0.1", "8000") == 0);
    sc_sock_print(&sock, tmp, sizeof(tmp));
    assert(strcmp(tmp, "Local(127.0.0.1:8000), Remote() ") == 0);

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
        rc = sc_sock_connect(&sock, "127.0.0.1", "8000", "127.0.0.1", "9090");
        if (rc == 0) {
            break;
        }

        sleep(1);
    }

    assert(rc == 0);
    sc_sock_print(&sock, tmp, sizeof(tmp));
    assert(strcmp(tmp, "Local(127.0.0.1:9090), Remote(127.0.0.1:8000) ") == 0);
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
    assert(sc_sock_listen(&sock, "::1", "8000") == 0);
    sc_sock_print(&sock, tmp, sizeof(tmp));
    assert(strcmp(tmp, "Local(::1:8000), Remote() ") == 0);
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
        rc = sc_sock_connect(&sock, "::1", "8000", NULL, NULL);
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

    sc_sock_init(&sock, 0, true, AF_UNIX);
    for (int i = 0; i < 5; i++) {
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
    struct sc_sock *sock;

    sock = sc_sock_create(0, false, AF_INET);
    assert(sc_sock_connect(sock, "3127.0.0.1", "2131", NULL, NULL) != 0);
    assert(sc_sock_finish_connect(sock) != 0);
    assert(sc_sock_connect(sock, "3127.0.0.1", "2131", "127.90.1.1", "50") != 0);
    assert(sc_sock_finish_connect(sock) != 0);
    assert(sc_sock_send(sock, "test", 5) == -1);
    assert(sc_sock_recv(sock, tmp, sizeof(tmp)) == -1);
    assert(sc_sock_connect(sock, "131s::1", "2131", "::1", "50") != 0);
    assert(sc_sock_finish_connect(sock) != 0);
    assert(sc_sock_connect(sock, "d::1", "2131", "dsad", "50") != 0);
    assert(sc_sock_finish_connect(sock) != 0);
    assert(sc_sock_connect(sock, "dsadas", "2131", "::1", "50") != 0);

    assert(sock != NULL);
    sc_sock_destroy(sock);
}



int main()
{
#if defined(_WIN32) || defined(_WIN64)
    WSADATA data;

    int rc = WSAStartup(MAKEWORD(2, 2), &data);
    assert(rc == 0);
    assert(LOBYTE(data.wVersion) == 2 &&
        HIBYTE(data.wVersion) == 2);
#endif

    test1();
    test_ip4();
    test_ip6();
    test_unix();
    return 0;
}

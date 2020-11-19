

#include "sc_poll.h"

#include <assert.h>
#include <memory.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#if defined(_WIN32) || defined(_WIN64)

    #include <windows.h>

struct sc_thread
{
    HANDLE id;
    void *(*fn)(void *);
    void *arg;
    void *ret;
};

#else

    #include <errno.h>
    #include <pthread.h>
    #include <unistd.h>

struct sc_thread
{
    pthread_t id;
};

#endif

void sc_thread_init(struct sc_thread *thread);
int sc_thread_term(struct sc_thread *thread);
int sc_thread_start(struct sc_thread *thread, void *(*fn)(void *), void *arg);
int sc_thread_stop(struct sc_thread *thread, void **ret);

void sc_thread_init(struct sc_thread *thread)
{
    thread->id = 0;
}

#if defined(_WIN32) || defined(_WIN64)
    #include <process.h>

unsigned int __stdcall sc_thread_fn(void *arg)
{
    struct sc_thread *thread = arg;

    thread->ret = thread->fn(thread->arg);
    return 0;
}

int sc_thread_start(struct sc_thread *thread, void *(*fn)(void *), void *arg)
{
    int rc;

    thread->fn = fn;
    thread->arg = arg;

    thread->id =
            (HANDLE) _beginthreadex(NULL, 0, sc_thread_fn, thread, 0, NULL);
    rc = thread->id == 0 ? -1 : 0;

    return rc;
}

int sc_thread_stop(struct sc_thread *thread, void **ret)
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

int sc_thread_start(struct sc_thread *thread, void *(*fn)(void *), void *arg)
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

int sc_thread_stop(struct sc_thread *thread, void **ret)
{
    int rc;
    void *val;

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

int sc_thread_term(struct sc_thread *thread)
{
    return sc_thread_stop(thread, NULL);
}

void *server(void *arg)
{
    int write_triggered = 2;
    int server_fd, rc, on = 1, received = 0;
    struct sockaddr_in addr = {0};
    struct sockaddr_in c_addr = {0};
    int socklen = sizeof(c_addr);
    struct sc_poll poll;


    assert(sc_poll_init(&poll) == 0);
    assert(sc_poll_term(&poll) == 0);
    assert(sc_poll_init(&poll) == 0);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(server_fd != -1);

    rc = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    assert(rc == 0);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8000);

    rc = bind(server_fd, (struct sockaddr *) &addr, sizeof(addr));
    assert(rc == 0);

    rc = listen(server_fd, 10);
    assert(rc == 0);

    assert(sc_poll_add_fd(&poll, server_fd, SC_POLL_READ,
                          (void *) (uintptr_t) server_fd) == 0);
    bool done = false;

    while (!done) {
        rc = sc_poll_wait(&poll, -1);
        assert(rc != -1);

        for (int i = 0; i < rc; i++) {
            int ev = sc_poll_event(&poll, i);
            int fd = (int) (uintptr_t) sc_poll_data(&poll, i);

            if (fd == server_fd) {
                if (ev & SC_POLL_READ) {
                    int in = accept(fd, (struct sockaddr *) &c_addr,
                                    (socklen_t *) &socklen);
                    assert(in != -1);
                    assert(sc_poll_add_fd(&poll, in, SC_POLL_WRITE,
                                          (void *) (uintptr_t) in) == 0);
                }

                if (ev & SC_POLL_WRITE) {
                    assert(false);
                }
            } else {
                if (ev & SC_POLL_READ) {
                    char buf[8];
                    int n = read(fd, buf, sizeof(buf));
                    if (n == 8) {
                        assert(strcmp(buf, "dataxxx") == 0);
                        received++;
                    } else if (n == 0 || n < 0) {
                        assert(sc_poll_del_fd(&poll, fd) == 0);
                        assert(close(fd) == 0);
                        done = true;
                        break;
                    }
                }

                if (write_triggered > 0 && (ev & SC_POLL_WRITE)) {
                    assert(fd != server_fd);
                    write_triggered--;
                    int flags = (write_triggered != 0) ?
                                SC_POLL_READ | SC_POLL_WRITE :
                                SC_POLL_READ;
                    assert(sc_poll_mod_fd(&poll, fd, flags,
                                          (void *) (uintptr_t) fd) == 0);
                }
            }
        }
    }

    assert(write_triggered == 0);
    assert(close(server_fd) == 0);
    assert(sc_poll_term(&poll) == 0);

    printf("Received :%d \n", received);
    assert(received == 10000);


    return NULL;
}

void *client(void *arg)
{
    for (int i = 0; i < 10; i++) {
        struct sockaddr_in srv_addr = {0};

        srv_addr.sin_family = AF_INET;
        srv_addr.sin_addr.s_addr = INADDR_ANY;
        srv_addr.sin_port = htons(8000);

        int client_fd = socket(AF_INET, SOCK_STREAM, 0);
        assert(client_fd != -1);


        int rc = connect(client_fd, (struct sockaddr *) &srv_addr,
                         sizeof(srv_addr));
        if (rc != 0) {
            sleep(1);
            assert(close(client_fd) == 0);
            continue;
        }

        for (int j = 0; j < 10000; j++) {
            rc = write(client_fd, "dataxxx", 8);
            assert(rc == 8);
        }

        assert(close(client_fd) == 0);
        break;
    }

    return NULL;
}


void test1()
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
    test1();

    return 0;
}

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "sc_sock.h"

#include <assert.h>
#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
#include <synchapi.h>
#define sleep(n) (Sleep(n * 1000))
#endif

#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>

struct sc_thread {
	HANDLE id;
	void *(*fn)(void *);
	void *arg;
	void *ret;
};

#else

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

struct sc_thread {
	pthread_t id;
};

#endif

void sc_thread_init(struct sc_thread *thread);
int sc_thread_term(struct sc_thread *thread);
int sc_thread_start(struct sc_thread *thread, void *(*fn)(void *), void *arg);
int sc_thread_join(struct sc_thread *thread, void **ret);

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

int sc_thread_join(struct sc_thread *thread, void **ret)
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

int sc_thread_join(struct sc_thread *thread, void **ret)
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
	return sc_thread_join(thread, NULL);
}

int sc_time_sleep(uint64_t millis)
{
#if defined(_WIN32) || defined(_WIN64)
	Sleep((DWORD) millis);
	return 0;
#else
	int rc;
	struct timespec t, rem;

	rem.tv_sec = (time_t) millis / 1000;
	rem.tv_nsec = (long) (millis % 1000) * 1000000;

	do {
		t = rem;
		rc = nanosleep(&t, &rem);
	} while (rc != 0 && errno == EINTR);

	return rc;
#endif
}

void *server_ip4(void *arg)
{
	(void) arg;

	char buf[5];
	char tmp[128];
	struct sc_sock sock, accepted;

	sc_sock_init(&sock, 0, true, SC_SOCK_INET);
	assert(sc_sock_listen(&sock, "127.0.0.1", "8004") == 0);
	sc_sock_print(&sock, tmp, sizeof(tmp));
	assert(strcmp(tmp, "Local(127.0.0.1:8004), Remote() ") == 0);

	assert(sc_sock_accept(&sock, &accepted) == 0);
	assert(sc_sock_recv(&accepted, buf, 5, 0) == 5);
	assert(strcmp("test", buf) == 0);

	assert(sc_sock_term(&sock) == 0);
	assert(sc_sock_term(&accepted) == 0);

	return NULL;
}

void *client_ip4(void *arg)
{
	(void) arg;

	int rc;
	char tmp[128];
	struct sc_sock sock;

	sc_sock_init(&sock, 0, true, SC_SOCK_INET);
	assert(sc_sock_set_sndtimeo(&sock, 10000) != 0);
	printf("%s \n", sc_sock_error(&sock));
	assert(sc_sock_set_rcvtimeo(&sock, 10000) != 0);
	printf("%s \n", sc_sock_error(&sock));

	for (int i = 0; i < 5; i++) {
		rc = sc_sock_connect(&sock, "127.0.0.1", "8004", NULL, NULL);
		if (rc == 0) {
			break;
		}

		sleep(1);
	}

	assert(rc == 0);
	sc_sock_print(&sock, tmp, sizeof(tmp));
	assert(sc_sock_set_sndtimeo(&sock, 10000) == 0);
	assert(sc_sock_set_rcvtimeo(&sock, 10000) == 0);
	assert(sc_sock_send(&sock, "test", 5, 0) == 5);
	assert(sc_sock_term(&sock) == 0);

	return NULL;
}

void test_ip4(void)
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

void *server_ip6(void *arg)
{
	(void) arg;
	char buf[5];
	char tmp[128];
	struct sc_sock sock, accepted;

	sc_sock_init(&sock, 0, true, SC_SOCK_INET6);
	assert(sc_sock_listen(&sock, "::1", "8006") == 0);
	sc_sock_print(&sock, tmp, sizeof(tmp));
	assert(strcmp(tmp, "Local(::1:8006), Remote() ") == 0);
	assert(sc_sock_accept(&sock, &accepted) == 0);
	assert(sc_sock_recv(&accepted, buf, 5, 0) == 5);
	assert(strcmp("test", buf) == 0);

	assert(sc_sock_term(&sock) == 0);
	assert(sc_sock_term(&accepted) == 0);

	return NULL;
}

void *client_ip6(void *arg)
{
	(void) arg;
	int rc;
	struct sc_sock sock;

	sc_sock_init(&sock, 0, true, SC_SOCK_INET6);

	for (int i = 0; i < 5; i++) {
		rc = sc_sock_connect(&sock, "::1", "8006", NULL, NULL);
		if (rc == 0) {
			break;
		}

		sleep(1);
	}
	assert(rc == 0);
	assert(sc_sock_send(&sock, "test", 5, 0) == 5);

	assert(sc_sock_term(&sock) == 0);

	return NULL;
}

void test_ip6(void)
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

void *server_unix(void *arg)
{
	(void) arg;
	char buf[5];
	char tmp[128];
	struct sc_sock sock, accepted;

	sc_sock_init(&sock, 0, true, SC_SOCK_UNIX);
	assert(sc_sock_listen(&sock, "x.sock", NULL) == 0);
	sc_sock_print(&sock, tmp, sizeof(tmp));
	assert(strcmp(tmp, "Local(x.sock), Remote() ") == 0);
	int rc = sc_sock_accept(&sock, &accepted);
	if (rc != 0) {
		printf("error(%d) : %s\n", errno, sock.err);
	}

	assert(rc == 0);
	assert(sc_sock_recv(&accepted, buf, 5, 0) == 5);
	assert(strcmp("test", buf) == 0);

	assert(sc_sock_term(&sock) == 0);
	assert(sc_sock_term(&accepted) == 0);

	return NULL;
}

void *client_unix(void *arg)
{
	(void) arg;
	int rc;
	struct sc_sock sock;

	sleep(3);
	sc_sock_init(&sock, 0, true, SC_SOCK_UNIX);
	for (int i = 0; i < 10; i++) {
		printf("Will connect to x.sock \n");
		rc = sc_sock_connect(&sock, "x.sock", NULL, NULL, NULL);
		if (rc == 0) {
			printf("Connected to x.sock \n");
			break;
		}

		printf("Failed to connect to x.sock \n");
		sleep(1);
	}

	assert(rc == 0);
	assert(sc_sock_send(&sock, "test", 5, 0) == 5);
	assert(sc_sock_term(&sock) == 0);

	return NULL;
}

void test_unix(void)
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

void test1(void)
{
	int rc;
	char tmp[5];
	struct sc_sock sock, client, in;

	sc_sock_init(&sock, 0, false, SC_SOCK_INET);
	assert(sc_sock_connect(&sock, "3127.0.0.1", "2131", NULL, NULL) != 0);
	assert(sc_sock_finish_connect(&sock) != 0);
	assert(sc_sock_connect(&sock, "3127.0.0.1", "2131", "127.90.1.1",
			       "50") != 0);
	assert(sc_sock_finish_connect(&sock) != 0);
	assert(sc_sock_send(&sock, "test", 5, 0) == -1);
	assert(sc_sock_recv(&sock, tmp, sizeof(tmp), 0) == -1);
	assert(sc_sock_connect(&sock, "131s::1", "2131", "::1", "50") != 0);
	assert(sc_sock_finish_connect(&sock) != 0);
	assert(sc_sock_connect(&sock, "d::1", "2131", "dsad", "50") != 0);
	assert(sc_sock_finish_connect(&sock) != 0);
	assert(sc_sock_connect(&sock, "dsadas", "2131", "::1", "50") != 0);
	assert(sc_sock_connect(&sock, "dsadas", "2131", NULL, "50") != 0);
	assert(sc_sock_connect(&sock, "dsadas", "2131", "s", NULL) != 0);
	assert(sc_sock_connect(&sock, "127.0.01", "2131", "100.0.0.0", NULL) !=
	       0);
	assert(sc_sock_connect(&sock, "127.0.01", "2131", NULL, "9000") != 0);
	sc_sock_term(&sock);

	sc_sock_init(&sock, 0, false, SC_SOCK_INET);
	assert(sc_sock_listen(&sock, "127.0.0.1", "8080") == 0);

	sc_sock_init(&client, 0, false, SC_SOCK_INET);
	rc = sc_sock_connect(&client, "127.0.0.1", "8080", NULL, NULL);
	assert(rc != -1 || (rc == -1 && errno == EAGAIN));

	sleep(2);
	rc = sc_sock_accept(&sock, &in);
	if (rc != 0) {
		printf("%d, %s \n", rc, sc_sock_error(&sock));
		assert(true);
	}

	assert(sc_sock_finish_connect(&client) == 0);
	assert(sc_sock_term(&sock) == 0);
	assert(sc_sock_term(&client) == 0);
	assert(sc_sock_term(&in) == 0);
}

void test_poll_mass(void)
{
	struct sc_sock_poll poll;
	struct sc_sock_pipe pipe[100];

	assert(sc_sock_poll_init(&poll) == 0);
	for (int i = 0; i < 100; i++) {
		assert(sc_sock_pipe_init(&pipe[i], 0) == 0);
		assert(sc_sock_poll_add(&poll, &pipe[i].fdt, SC_SOCK_READ,
					NULL) == 0);
	}
	for (int i = 0; i < 100; i++) {
		assert(sc_sock_poll_del(&poll, &pipe[i].fdt, SC_SOCK_READ,
					NULL) == 0);
		assert(sc_sock_pipe_term(&pipe[i]) == 0);
		assert(sc_sock_pipe_term(&pipe[i]) == 0);
	}
	assert(sc_sock_poll_term(&poll) == 0);
	assert(sc_sock_poll_term(&poll) == 0);

	assert(sc_sock_poll_init(&poll) == 0);
	for (int i = 0; i < 100; i++) {
		assert(sc_sock_pipe_init(&pipe[i], 0) == 0);
		assert(sc_sock_poll_add(&poll, &pipe[i].fdt,
					SC_SOCK_READ | SC_SOCK_WRITE,
					NULL) == 0);
	}
	for (int i = 0; i < 100; i++) {
		assert(sc_sock_poll_del(&poll, &pipe[i].fdt, SC_SOCK_READ,
					NULL) == 0);
		assert(sc_sock_poll_del(&poll, &pipe[i].fdt, SC_SOCK_WRITE,
					NULL) == 0);
		assert(sc_sock_pipe_term(&pipe[i]) == 0);
		assert(sc_sock_pipe_term(&pipe[i]) == 0);
	}
	assert(sc_sock_poll_term(&poll) == 0);

	assert(sc_sock_poll_init(&poll) == 0);
	for (int i = 0; i < 100; i++) {
		assert(sc_sock_pipe_init(&pipe[i], 0) == 0);
		assert(sc_sock_poll_add(&poll, &pipe[i].fdt,
					SC_SOCK_READ | SC_SOCK_WRITE,
					NULL) == 0);
	}
	for (int i = 0; i < 100; i++) {
		assert(sc_sock_poll_del(&poll, &pipe[i].fdt, SC_SOCK_WRITE,
					NULL) == 0);
		assert(sc_sock_poll_del(&poll, &pipe[i].fdt, SC_SOCK_READ,
					NULL) == 0);
		assert(sc_sock_pipe_term(&pipe[i]) == 0);
		assert(sc_sock_pipe_term(&pipe[i]) == 0);
	}
	assert(sc_sock_poll_term(&poll) == 0);
	assert(sc_sock_poll_term(&poll) == 0);

	assert(sc_sock_poll_init(&poll) == 0);
	for (int i = 0; i < 100; i++) {
		assert(sc_sock_pipe_init(&pipe[i], 0) == 0);
		assert(sc_sock_poll_add(&poll, &pipe[i].fdt, SC_SOCK_WRITE,
					NULL) == 0);
	}
	for (int i = 0; i < 100; i++) {
		assert(sc_sock_poll_del(&poll, &pipe[i].fdt, SC_SOCK_WRITE,
					NULL) == 0);
		assert(sc_sock_poll_del(&poll, &pipe[i].fdt, SC_SOCK_READ,
					NULL) == 0);
		assert(sc_sock_pipe_term(&pipe[i]) == 0);
	}
	assert(sc_sock_poll_term(&poll) == 0);
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

struct sc_mutex {
	pthread_mutex_t mtx;
};

int test_done = 0;
struct sc_mutex mutex;

int sc_mutex_init(struct sc_mutex *mtx)
{
	int rc, rv;
	pthread_mutexattr_t attr;
	pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

	mtx->mtx = mut;

	// May fail on OOM
	rc = pthread_mutexattr_init(&attr);
	if (rc != 0) {
		return -1;
	}

	// This won't fail as long as we pass correct params.
	rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
	assert(rc == 0);

	// May fail on OOM
	rc = pthread_mutex_init(&mtx->mtx, &attr);

	// This won't fail as long as we pass correct param.
	rv = pthread_mutexattr_destroy(&attr);
	assert(rv == 0);

	return rc != 0 ? -1 : 0;
}

int sc_mutex_term(struct sc_mutex *mtx)
{
	int rc;

	rc = pthread_mutex_destroy(&mtx->mtx);
	return rc != 0 ? -1 : 0;
}

void sc_mutex_lock(struct sc_mutex *mtx)
{
	int rc;

	// This won't fail as long as we pass correct param.
	rc = pthread_mutex_lock(&mtx->mtx);
	if (rc != 0) {
		printf("%s \n", strerror(rc));
		fprintf(stderr, "%s \n", strerror(rc));
		fflush(stderr);
		fflush(stdout);
	}
	assert(rc == 0);
}

void sc_mutex_unlock(struct sc_mutex *mtx)
{
	int rc;

	// This won't fail as long as we pass correct param.
	rc = pthread_mutex_unlock(&mtx->mtx);
	assert(rc == 0);
}

bool fail_malloc = false;
void *__real_malloc(size_t n);
void *__wrap_malloc(size_t n)
{
	if (fail_malloc) {
		return NULL;
	}

	return __real_malloc(n);
}

bool fail_realloc = false;
void *__real_realloc(void *p, size_t size);
void *__wrap_realloc(void *p, size_t n)
{
	if (fail_realloc) {
		return NULL;
	}

	return __real_realloc(p, n);
}

bool fail_epollcreate1 = false;
int __real_epoll_create1(int flags);
int __wrap_epoll_create1(int flags)
{
	if (fail_epollcreate1) {
		return -1;
	}

	return __real_epoll_create1(flags);
}

bool fail_close = false;
int __real_close(int fd);
int __wrap_close(int fd)
{
	if (fail_close) {
		__real_close(fd);
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

int fail_setsockopt = INT32_MAX;
int __real_setsockopt(int fd, int level, int optname, const void *optval,
		      socklen_t optlen);
int __wrap_setsockopt(int fd, int level, int optname, const void *optval,
		      socklen_t optlen)
{
	sc_mutex_lock(&mutex);
	if (--fail_setsockopt <= 0) {
		sc_mutex_unlock(&mutex);
		return -1;
	}

	sc_mutex_unlock(&mutex);
	return __real_setsockopt(fd, level, optname, optval, optlen);
}

int fail_listen = false;
int __real_listen(int fd, int n);
int __wrap_listen(int fd, int n)
{
	if (fail_listen) {
		return -1;
	}

	return __real_listen(fd, n);
}

int fail_fcntl = INT32_MAX;
int __real_fcntl(int fd, int cmd, ...);
int __wrap_fcntl(int fd, int cmd, ...)
{
	(void) fd;
	(void) cmd;

	if (test_done) {
		va_list va;
		va_start(va, cmd);
		void *t = va_arg(va, void *);
		return __real_fcntl(fd, cmd, t);
	}

	sc_mutex_lock(&mutex);
	if (--fail_fcntl <= 0) {
		sc_mutex_unlock(&mutex);
		return -1;
	}

	if (cmd == F_GETFL) {
		sc_mutex_unlock(&mutex);
		return __real_fcntl(fd, F_GETFL, 0);
	}

	if (cmd == F_SETFL) {
		va_list va;
		va_start(va, cmd);
		int x = va_arg(va, int);
		sc_mutex_unlock(&mutex);
		return __real_fcntl(fd, F_SETFL, x);
	}

	sc_mutex_unlock(&mutex);
	return -1;
}

int fail_getsockopt;
int fail_getsockopt_result;

extern int __real_getsockopt(int fd, int level, int optname,
			     void *restrict optval, socklen_t *restrict optlen);
extern int __wrap_getsockopt(int fd, int level, int optname,
			     void *restrict optval, socklen_t *restrict optlen)
{
	if (fail_getsockopt) {
		*(int *) optval = fail_getsockopt_result;
		return fail_getsockopt_result != 0 ? 0 : 10;
	}

	return __real_getsockopt(fd, level, optname, optval, optlen);
}

int fail_epoll_wait;
extern int __real_epoll_wait(int epfd, struct epoll_event *events,
			     int maxevents, int timeout);
extern int __wrap_epoll_wait(int epfd, struct epoll_event *events,
			     int maxevents, int timeout)
{
	if (fail_epoll_wait) {
		fail_epoll_wait = 0;
		errno = EINTR;
		return -1;
	}

	return __real_epoll_wait(epfd, events, maxevents, timeout);
}

int fail_socket = false;
int __real_socket(int domain, int type, int protocol);
int __wrap_socket(int domain, int type, int protocol)
{
	if (fail_socket) {
		return -1;
	}

	return __real_socket(domain, type, protocol);
}

int fail_inet_ntop = INT32_MAX;
const char *__real_inet_ntop(int af, const void *restrict cp,
			     char *restrict buf, socklen_t len);
const char *__wrap_inet_ntop(int af, const void *restrict cp,
			     char *restrict buf, socklen_t len)
{
	sc_mutex_lock(&mutex);
	if (--fail_inet_ntop <= 0) {
		sc_mutex_unlock(&mutex);
		return NULL;
	}

	sc_mutex_unlock(&mutex);
	return __real_inet_ntop(af, cp, buf, len);
}

int fail_connect = 0;
int fail_connect_err = 0;
int __real_connect(int fd, const struct sockaddr *addr, socklen_t len);
int __wrap_connect(int fd, const struct sockaddr *addr, socklen_t len)
{
	if (fail_connect) {
		errno = fail_connect_err;
		return -1;
	}

	return __real_connect(fd, addr, len);
}

int success_sendmsg = 0;
ssize_t __real_sendmsg(int fd, const struct msghdr *message, int flags);
ssize_t __wrap_sendmsg(int fd, const struct msghdr *message, int flags)
{
	if (success_sendmsg) {
		return 0;
	}

	return __real_sendmsg(fd, message, flags);
}

int fail_send = INT32_MAX;
int fail_send_err = 0;
int fail_send_errno = 0;
ssize_t __real_send(int fd, const void *buf, size_t n, int flags);
ssize_t __wrap_send(int fd, const void *buf, size_t n, int flags)
{
	if (--fail_send <= 0) {
		errno = (errno == EINTR) ? EINVAL : fail_send_errno;
		return fail_send_err;
	}

	return __real_send(fd, buf, n, flags);
}

int fail_write = INT32_MAX;
int fail_write_err = 0;
int fail_write_errno = 0;
ssize_t __real_write(int fd, const void *buf, size_t n);
ssize_t __wrap_write(int fd, const void *buf, size_t n)
{
	if (--fail_write <= 0) {
		errno = (errno == EINTR) ? EINVAL : fail_write_errno;
		return fail_write_err;
	}

	return __real_write(fd, buf, n);
}

int fail_recv = INT32_MAX;
int fail_recv_err = 0;
int fail_recv_errno = 0;
ssize_t __real_recv(int fd, void *buf, size_t n, int flags);
ssize_t __wrap_recv(int fd, void *buf, size_t n, int flags)
{
	if (--fail_recv <= 0) {
		errno = (errno == EINTR) ? EINVAL : fail_recv_errno;
		return fail_recv_err;
	}

	return __real_recv(fd, buf, n, flags);
}

int fail_read = INT32_MAX;
int fail_read_err = 0;
int fail_read_errno = 0;
ssize_t __real_read(int fd, void *buf, size_t n, int flags);
ssize_t __wrap_read(int fd, void *buf, size_t n, int flags)
{
	if (--fail_read <= 0) {
		errno = (errno == EINTR) ? EINVAL : fail_read_errno;
		return fail_read_err;
	}

	return __real_read(fd, buf, n, flags);
}

int fail_epoll_ctl = false;
int __real_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int __wrap_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
	if (fail_epoll_ctl) {
		return -1;
	}

	return __real_epoll_ctl(epfd, op, fd, event);
}

int fail_getsockname;
int __real_getsockname(int fd, struct sockaddr *addr, socklen_t *len);
int __wrap_getsockname(int fd, struct sockaddr *addr, socklen_t *len)
{
	if (fail_getsockname) {
		struct sockaddr_storage *str = (struct sockaddr_storage *) addr;
		str->ss_family = AF_BRIDGE;
		return 0;
	}

	return __real_getsockname(fd, addr, len);
}

void poll_fail_test(void)
{
	struct sc_sock sock;
	struct sc_sock_poll poll;

	assert(sc_sock_poll_init(&poll) == 0);
	fail_epoll_wait = 1;
	assert(sc_sock_poll_wait(&poll, 10) == 0);
	assert(sc_sock_poll_term(&poll) == 0);
	assert(sc_sock_poll_term(&poll) == 0);

	assert(sc_sock_poll_init(&poll) == 0);
	sc_sock_init(&sock, 0, true, AF_INET);
	fail_epoll_ctl = true;
	assert(sc_sock_poll_add(&poll, &sock.fdt, SC_SOCK_READ, NULL) == -1);
	fail_epoll_ctl = false;
	sc_sock_term(&sock);
	sc_sock_poll_term(&poll);

	assert(sc_sock_poll_init(&poll) == 0);
	sc_sock_init(&sock, 0, true, AF_INET);
	sock.fdt.op = SC_SOCK_READ;
	assert(sc_sock_poll_del(&poll, &sock.fdt, SC_SOCK_READ, NULL) == -1);
	sc_sock_term(&sock);
	sc_sock_poll_term(&poll);

	fail_malloc = true;
	assert(sc_sock_poll_init(&poll) == -1);
	fail_malloc = false;

	fail_epollcreate1 = true;
	assert(sc_sock_poll_init(&poll) == -1);
	fail_epollcreate1 = false;
}

void pipe_fail_test(void)
{
	struct sc_sock_pipe pipe;
	fail_pipe = true;
	assert(sc_sock_pipe_init(&pipe, 0) != 0);
	assert(*sc_sock_pipe_err(&pipe) != '\0');
	fail_pipe = false;
	assert(sc_sock_pipe_init(&pipe, 0) == 0);
	fail_close = true;
	assert(sc_sock_pipe_term(&pipe) == -1);
	fail_close = false;

	assert(sc_sock_pipe_init(&pipe, 0) == 0);
	fail_write = 1;
	fail_write_err = -1;
	fail_write_errno = EINTR;
	assert(sc_sock_pipe_write(&pipe, NULL, 10) == -1);
	sc_sock_pipe_term(&pipe);

	fail_write = INT32_MAX;
	fail_write_err = 0;
	fail_write_errno = 0;

	assert(sc_sock_pipe_init(&pipe, 0) == 0);
	fail_read = 1;
	fail_read_err = -1;
	fail_read_errno = EINTR;
	assert(sc_sock_pipe_read(&pipe, NULL, 10) == -1);
	sc_sock_pipe_term(&pipe);

	fail_read = INT32_MAX;
	fail_read_err = 0;
	fail_read_errno = 0;
}

void sock_fail_test(void)
{
	struct sc_sock sock;
	struct sc_sock sock2;
	struct sc_sock sock3;

	sc_sock_init(&sock, 0, true, SC_SOCK_INET);
	sc_sock_listen(&sock, "127.0.0.1", "8080");
	fail_close = true;
	assert(sc_sock_term(&sock) == -1);
	assert(*sc_sock_error(&sock));
	fail_close = false;

	sc_sock_init(&sock, 0, true, SC_SOCK_INET);

	fail_setsockopt = 1;
	assert(sc_sock_listen(&sock, "127.0.0.1", "8080") == -1);
	fail_setsockopt = 2;
	assert(sc_sock_listen(&sock, "127.0.0.1", "8080") == -1);
	fail_setsockopt = 3;
	assert(sc_sock_listen(&sock, "127.0.0.1", "8080") == 0);
	assert(sc_sock_term(&sock) == 0);

	sc_sock_init(&sock, 0, true, SC_SOCK_UNIX);
	fail_setsockopt = 1;
	assert(sc_sock_listen(&sock, "/tmp/x", NULL) == 0);
	assert(sc_sock_term(&sock) == 0);
	fail_setsockopt = INT32_MAX;

	sc_sock_init(&sock, 0, true, SC_SOCK_INET);
	fail_listen = 1;
	assert(sc_sock_listen(&sock, "127.0.0.1", "8080") == -1);
	fail_listen = 0;
	assert(sc_sock_accept(&sock, &sock2) == -1);
	assert(sc_sock_listen(&sock, "127.0.0.1", "8080") == 0);
	fail_setsockopt = INT32_MAX;

	sc_sock_init(&sock2, 0, true, SC_SOCK_INET);
	assert(sc_sock_connect(&sock2, "127.0.0.1", "8080", NULL, NULL) == 0);
	fail_setsockopt = 1;
	assert(sc_sock_accept(&sock, &sock3) == -1);
	fail_setsockopt = INT32_MAX;
	sc_sock_term(&sock);
	sc_sock_term(&sock2);

	assert(sc_sock_recv(&sock, NULL, -33, 0) == -33);
	assert(sc_sock_send(&sock, NULL, -33, 0) == -33);

	sc_sock_init(&sock, 0, true, SC_SOCK_INET6);
	fail_setsockopt = 1;
	assert(sc_sock_listen(&sock, "::1", "8080") == -1);
	assert(sc_sock_term(&sock) == 0);
	fail_setsockopt = INT32_MAX;

	assert(sc_sock_set_blocking(&sock, true) == -1);
	sc_sock_init(&sock, 0, true, SC_SOCK_INET);
	assert(sc_sock_listen(&sock, "127.0.0.1", "8080") == 0);
	fail_fcntl = 2;
	assert(sc_sock_set_blocking(&sock, true) == -1);
	fail_fcntl = INT32_MAX;
	sc_sock_term(&sock);

	sc_sock_init(&sock, 0, true, SC_SOCK_INET);
	fail_fcntl = 1;
	assert(sc_sock_connect(&sock, "127.0.0.1", "8080", NULL, NULL) == -1);
	assert(sc_sock_set_blocking(&sock, true) == -1);
	fail_fcntl = INT32_MAX;
	sc_sock_term(&sock);

	sc_sock_init(&sock, 0, true, SC_SOCK_INET);
	fail_fcntl = 1;
	assert(sc_sock_listen(&sock, "127.0.0.1", "8080") == -1);
	assert(sc_sock_term(&sock) == 0);
	fail_fcntl = INT32_MAX;

	sc_sock_init(&sock, 0, true, SC_SOCK_UNIX);
	assert(sc_sock_connect(&sock,
			       "/tmp/"
			       "longlonglonglolonglonglonglonglonglonglonglongl"
			       "onglonglongl"
			       "onglonglonglonglonglonglongnglonglonglonglonglo"
			       "ng",
			       "", NULL, NULL) == -1);
	sc_sock_term(&sock);

	sc_sock_init(&sock, 0, true, SC_SOCK_INET);
	fail_socket = 1;
	assert(sc_sock_listen(&sock, "127.0.0.1", "8080") == -1);
	assert(sc_sock_connect(&sock, "127.0.0.1", "8080", NULL, NULL) == -1);
	assert(sc_sock_term(&sock) == 0);

	sc_sock_init(&sock, 0, true, SC_SOCK_UNIX);
	fail_socket = 1;
	assert(sc_sock_listen(&sock, "/tmp/x", "8080") == -1);
	assert(sc_sock_connect(&sock, "/tmp/x", "8080", NULL, NULL) == -1);
	assert(sc_sock_term(&sock) == 0);

	fail_socket = 0;
}

void sock_fail_test2(void)
{
	int rc;
	struct sc_sock sock, client, in;

	sc_sock_init(&sock, 0, false, SC_SOCK_INET);
	assert(sc_sock_listen(&sock, "127.0.0.1", "8080") == 0);
	sc_sock_init(&client, 0, false, SC_SOCK_INET);
	rc = sc_sock_connect(&client, "127.0.0.1", "8080", NULL, NULL);
	assert(rc != -1 || (rc == -1 && errno == EAGAIN));
	sleep(2);
	fail_fcntl = 1;
	assert(sc_sock_accept(&sock, &in) == -1);
	fail_fcntl = INT32_MAX;
	assert(sc_sock_term(&sock) == 0);
	assert(sc_sock_term(&client) == 0);
	assert(sc_sock_term(&in) == 0);

	sc_sock_init(&sock, 0, false, SC_SOCK_INET);
	assert(sc_sock_listen(&sock, "127.0.0.1", "8080") == 0);
	sc_sock_init(&client, 0, false, SC_SOCK_INET);
	rc = sc_sock_connect(&client, "127.0.0.1", "8080", NULL, NULL);
	assert(rc != -1 || (rc == -1 && errno == EAGAIN));
	sleep(2);
	fail_setsockopt = 1;
	assert(sc_sock_accept(&sock, &in) == -1);
	fail_setsockopt = INT32_MAX;
	assert(sc_sock_term(&sock) == 0);
	assert(sc_sock_term(&client) == 0);
	assert(sc_sock_term(&in) == 0);

	sc_sock_init(&sock, 0, false, SC_SOCK_INET);
	assert(sc_sock_listen(&sock, "127.0.0.1", "8080") == 0);
	sc_sock_init(&client, 0, false, SC_SOCK_INET);
	rc = sc_sock_connect(&client, "127.0.0.1", "8080", NULL, NULL);
	assert(rc != -1 || (rc == -1 && errno == EAGAIN));
	sleep(2);
	fail_setsockopt = INT32_MAX;
	assert(sc_sock_term(&sock) == 0);
	assert(sc_sock_term(&client) == 0);
	assert(sc_sock_term(&in) == 0);

	sc_sock_init(&sock, 0, false, SC_SOCK_INET);
	assert(sc_sock_listen(&sock, "127.0.0.1", "8080") == 0);
	sc_sock_init(&client, 0, false, SC_SOCK_INET);
	rc = sc_sock_connect(&client, "127.0.0.1", "8080", NULL, NULL);
	assert(rc != -1 || (rc == -1 && errno == EAGAIN));
	sleep(2);
	fail_setsockopt = INT32_MAX;
	assert(sc_sock_term(&sock) == 0);
	assert(sc_sock_term(&client) == 0);
	assert(sc_sock_term(&in) == 0);

	sc_sock_init(&sock, 0, false, SC_SOCK_UNIX);
	assert(sc_sock_listen(&sock, "/tmp/x", "8080") == 0);
	sc_sock_init(&client, 0, false, SC_SOCK_UNIX);
	fail_setsockopt = 1;
	rc = sc_sock_connect(&client, "/tmp/x", "8080", NULL, NULL);
	assert(rc != -1 || (rc == -1 && errno == EAGAIN));
	fail_setsockopt = INT32_MAX;
	assert(sc_sock_term(&sock) == 0);
	assert(sc_sock_term(&client) == 0);
	assert(sc_sock_term(&in) == 0);

	sc_sock_init(&sock, 0, false, SC_SOCK_UNIX);
	assert(sc_sock_listen(&sock, "/tmp/x", "8080") == 0);
	sc_sock_init(&client, 0, false, SC_SOCK_UNIX);
	fail_setsockopt = 2;
	rc = sc_sock_connect(&client, "/tmp/x", "8080", NULL, NULL);
	assert(rc != -1 || (rc == -1 && errno == EAGAIN));
	fail_setsockopt = INT32_MAX;
	assert(sc_sock_term(&sock) == 0);
	assert(sc_sock_term(&client) == 0);
	assert(sc_sock_term(&in) == 0);

	sc_sock_init(&client, 0, false, SC_SOCK_INET);
	fail_setsockopt = 1;
	rc = sc_sock_connect(&client, "127.0.0.1", "8080", NULL, NULL);
	assert(rc == -1);
	fail_setsockopt = INT32_MAX;
	assert(sc_sock_term(&client) == 0);

	sc_sock_init(&client, 0, false, SC_SOCK_INET);
	fail_setsockopt = 2;
	rc = sc_sock_connect(&client, "127.0.0.1", "8080", NULL, NULL);
	assert(rc == -1);
	fail_setsockopt = INT32_MAX;
	assert(sc_sock_term(&client) == 0);

	sc_sock_init(&client, 0, false, SC_SOCK_INET);
	fail_setsockopt = 3;
	rc = sc_sock_connect(&client, "127.0.0.1", "8080", NULL, NULL);
	assert(rc == -1);
	fail_setsockopt = INT32_MAX;
	assert(sc_sock_term(&client) == 0);

	sc_sock_init(&client, 0, false, SC_SOCK_INET);
	fail_setsockopt = 4;
	rc = sc_sock_connect(&client, "127.0.0.1", "8080", NULL, NULL);
	assert(rc == -1);
	fail_setsockopt = INT32_MAX;
	assert(sc_sock_term(&client) == 0);

	sc_sock_init(&client, 0, false, SC_SOCK_INET);
	sc_sock_listen(&client, "127.0.0.1", "8080");
	fail_inet_ntop = 1;
	sc_sock_print(&client, NULL, 0);
	fail_inet_ntop = INT32_MAX;
	assert(sc_sock_term(&client) == 0);

	char buf[128];
	sc_sock_init(&client, 0, false, SC_SOCK_INET);
	sc_sock_listen(&client, "127.0.0.1", "8080");
	fail_getsockname = 1;
	sc_sock_print(&client, buf, 128);
	fail_getsockname = 0;
	assert(sc_sock_term(&client) == 0);
}

void sock_fail_test3(void)
{
	struct sc_sock client;
	struct sc_sock_poll poll;

	sc_sock_poll_init(&poll);
	fail_close = true;
	assert(sc_sock_poll_term(&poll) == -1);
	fail_close = false;
	assert(sc_sock_poll_term(&poll) == 0);

	sc_sock_init(&client, 0, false, SC_SOCK_INET);
	fail_send = 1;
	fail_send_errno = EAGAIN;
	fail_send_err = -1;
	assert(sc_sock_send(&client, NULL, 10, 0) == -1);
	assert(errno == EAGAIN);
	sc_sock_term(&client);

	sc_sock_init(&client, 0, false, SC_SOCK_INET);
	fail_send = 1;
	fail_send_err = -1;
	fail_send_errno = EINTR;
	assert(sc_sock_send(&client, NULL, 10, 0) == -1);
	sc_sock_term(&client);

	fail_send = INT32_MAX;
	fail_send_err = 0;
	fail_send_errno = 0;

	sc_sock_init(&client, 0, false, SC_SOCK_INET);
	fail_recv = 1;
	fail_recv_errno = EAGAIN;
	fail_recv_err = -1;
	assert(sc_sock_recv(&client, NULL, 10, 0) == -1);
	assert(errno == EAGAIN);
	sc_sock_term(&client);

	sc_sock_init(&client, 0, false, SC_SOCK_INET);
	fail_recv = 1;
	fail_recv_err = -1;
	fail_recv_errno = EINTR;
	assert(sc_sock_recv(&client, NULL, 10, 0) == -1);
	sc_sock_term(&client);

	fail_recv = INT32_MAX;
	fail_recv_err = 0;
	fail_recv_errno = 0;

	sc_sock_init(&client, 0, false, SC_SOCK_INET);
	fail_connect_err = EINPROGRESS;
	fail_connect = -1;
	assert(sc_sock_connect(&client, "127.0.0.1", "8080", NULL, NULL) == -1);
	assert(errno == EAGAIN);

	fail_connect_err = 0;
	fail_connect = 0;
	assert(sc_sock_term(&client) == 0);

	sc_sock_init(&client, 0, false, SC_SOCK_INET);
	fail_connect_err = -999;
	fail_connect = -1;
	assert(sc_sock_connect(&client, "127.0.0.1", "8080", NULL, NULL) == -1);
	fail_connect_err = 0;
	fail_connect = 0;
	assert(sc_sock_term(&client) == 0);

	sc_sock_init(&client, 0, false, SC_SOCK_INET);
	fail_connect_err = EAGAIN;
	fail_connect = -1;
	assert(sc_sock_connect(&client, "127.0.0.1", "8080", NULL, NULL) == -1);
	assert(errno == EAGAIN);

	fail_connect_err = 0;
	fail_connect = 0;
	assert(sc_sock_term(&client) == 0);

	sc_sock_init(&client, 0, false, SC_SOCK_UNIX);
	fail_connect_err = EINPROGRESS;
	fail_connect = -1;
	assert(sc_sock_connect(&client, "/tmp/x", "8080", NULL, NULL) == -1);
	fail_connect_err = 0;
	fail_connect = 0;
	assert(sc_sock_term(&client) == 0);

	sc_sock_init(&client, 0, false, SC_SOCK_INET);
	assert(sc_sock_finish_connect(&client) != 0);
	fail_getsockopt = 1;
	fail_getsockopt_result = -1;
	assert(sc_sock_finish_connect(&client) != 0);
	fail_getsockopt = 0;
	sc_sock_term(&client);

	assert(sc_sock_notify_systemd("test") == -1);
	setenv("NOTIFY_SOCKET", "x", 1);
	assert(sc_sock_notify_systemd("test") == -1);
	setenv("NOTIFY_SOCKET", "/", 1);
	assert(sc_sock_notify_systemd("test") == -1);
	setenv("NOTIFY_SOCKET", "@", 1);
	assert(sc_sock_notify_systemd("test") == -1);

	setenv("NOTIFY_SOCKET", "/tmp/x", 1);
	fail_socket = 1;
	assert(sc_sock_notify_systemd("test") == -1);
	fail_socket = 0;
	setenv("NOTIFY_SOCKET", "@tmp/x", 1);
	assert(sc_sock_notify_systemd("test") == -1);
	setenv("NOTIFY_SOCKET",
	       "@tmp/"
	       "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
	       "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
	       "xxxxxx",
	       1);
	assert(sc_sock_notify_systemd("test") == -1);

	success_sendmsg = 1;
	setenv("NOTIFY_SOCKET", "/tmp/x", 1);
	assert(sc_sock_notify_systemd("test") == 0);
	success_sendmsg = 0;
	unsetenv("NOTIFY_SOCKET");
}

#else
void sock_fail_test(void)
{
}
void sock_fail_test2(void)
{
}
void sock_fail_test3(void)
{
}
void poll_fail_test(void)
{
}

void pipe_fail_test(void)
{
}

#endif

void *server(void *arg)
{
	(void) arg;
	uint32_t ev;
	int wr = 2;
	int rc, received = 0;
	struct sc_sock srv, in, *sock;
	struct sc_sock_poll p;

	assert(sc_sock_poll_init(&p) == 0);
	assert(*sc_sock_poll_err(&p) == '\0');
	assert(sc_sock_poll_term(&p) == 0);
	assert(sc_sock_poll_init(&p) == 0);

	sc_sock_init(&srv, 0, true, SC_SOCK_INET);
	rc = sc_sock_listen(&srv, "127.0.0.1", "11000");
	assert(rc == 0);

	rc = sc_sock_poll_add(&p, &srv.fdt, SC_SOCK_READ, &srv);
	assert(rc == 0);
	bool done = false;

	while (!done) {
		int count = sc_sock_poll_wait(&p, -1);
		assert(count != -1);

		for (int i = 0; i < count; i++) {
			ev = sc_sock_poll_event(&p, i);
			sock = sc_sock_poll_data(&p, i);

			if (ev == 0) {
				continue;
			}

			if (sock == &srv) {
				if (ev & SC_SOCK_READ) {
					rc = sc_sock_accept(&srv, &in);
					assert(rc == 0);
					printf("accepted \n");

					rc = sc_sock_poll_add(&p, &in.fdt,
							      SC_SOCK_WRITE,
							      &in);
					assert(rc == 0);
				}

				if (ev & SC_SOCK_WRITE) {
					assert(false);
				}
			} else {
				if (ev & SC_SOCK_READ) {
					char b;

					rc = sc_sock_recv(&in, &b, 1, 0);
					if (rc == 1) {
						assert(b == 'd');
						received++;
					} else if (rc == 0 || rc < 0) {
						int fl = SC_SOCK_READ |
							 SC_SOCK_WRITE;

						rc = sc_sock_poll_del(
							&p, &in.fdt, fl, &in);
						assert(rc == 0);

						rc = sc_sock_term(&in);
						assert(rc == 0);

						done = true;
						break;
					}
				}

				if (wr > 0) {
					wr--;
					if (wr == 0) {
						rc = sc_sock_poll_del(
							&p, &in.fdt,
							SC_SOCK_WRITE, &in);
						assert(rc == 0);
					} else {
						rc = sc_sock_poll_add(
							&p, &in.fdt,
							SC_SOCK_READ, &in);
					}
				}
			}
		}
	}

	assert(wr == 0);
	assert(sc_sock_term(&srv) == 0);
	assert(sc_sock_poll_term(&p) == 0);

	printf("Received :%d \n", received);
	assert(received == 10000);

	return NULL;
}

void *client(void *arg)
{
	(void) arg;

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
			rc = sc_sock_send(&sock, "d", 1, 0);
			assert(rc == 1);
		}

		rc = sc_sock_term(&sock);
		assert(rc == 0);
		break;
	}

	return NULL;
}

void test_poll(void)
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

void check_poll_empty(struct sc_sock_poll *p, int timeout)
{
	sc_time_sleep(50);
	int count = sc_sock_poll_wait(p, timeout);
	assert(count >= 0);

	for (int i = 0; i < count; i++) {
		assert(0 == sc_sock_poll_event(p, i));
	}
}

void test_poll_edge(void)
{
	uint32_t ev;
	int rc, count, found, timeout = 1;
	struct sc_sock srv, clt, acc, *sock;
	struct sc_sock_poll p;

	rc = sc_sock_poll_init(&p);
	assert(rc == 0);

	sc_sock_init(&srv, 0, false, SC_SOCK_INET);
	rc = sc_sock_listen(&srv, "127.0.0.1", "11000");
	assert(rc == 0);

	rc = sc_sock_poll_add(&p, &srv.fdt, SC_SOCK_READ, &srv);
	assert(rc == 0);

	check_poll_empty(&p, timeout);

	sc_sock_init(&clt, 0, false, SC_SOCK_INET);
	rc = sc_sock_connect(&clt, "127.0.0.1", "11000", NULL, NULL);
	if (rc == -1) {
		assert(errno == EAGAIN);
	} else {
		assert(rc == 0);
	}

	rc = sc_sock_poll_add(&p, &clt.fdt,
			      SC_SOCK_READ | SC_SOCK_WRITE | SC_SOCK_EDGE,
			      &clt);
	assert(rc == 0);

	sc_time_sleep(50);
	count = sc_sock_poll_wait(&p, timeout);
	assert(count >= 2);
	found = 0;

	for (int i = 0; i < count; i++) {
		ev = sc_sock_poll_event(&p, i);
		sock = sc_sock_poll_data(&p, i);

		if (ev == 0) {
			continue;
		}

		if (sock == &srv) {
			if (ev & SC_SOCK_READ) {
				rc = sc_sock_accept(&srv, &acc);
				assert(rc == 0);
			}

			if (ev & SC_SOCK_WRITE) {
				assert(false);
			}
			found++;
		} else if (sock == &clt) {
			if (ev & SC_SOCK_WRITE) {
				rc = sc_sock_finish_connect(&clt);
				assert(rc == 0);
			}

			if (ev & SC_SOCK_READ) {
				assert(false);
			}
			found++;
		} else {
			assert(false);
		}
	}
	assert(found == 2);

	rc = sc_sock_poll_add(&p, &acc.fdt,
			      SC_SOCK_READ | SC_SOCK_WRITE | SC_SOCK_EDGE,
			      &acc);
	assert(rc == 0);

	sc_time_sleep(50);
	count = sc_sock_poll_wait(&p, timeout);
	assert(count >= 1);
	found = 0;

	for (int i = 0; i < count; i++) {
		ev = sc_sock_poll_event(&p, i);
		sock = sc_sock_poll_data(&p, i);

		if (ev == 0) {
			continue;
		}

		assert(sock == &acc);
		assert(ev == SC_SOCK_WRITE);
		found++;
	}
	assert(found == 1);

	check_poll_empty(&p, timeout);

	int total_w = 0;
	int total_r = 0;

	for (;;) {
		int w = sc_sock_send(&acc, "blaBLA", 7, 0);
		if (w < 0) {
			assert(errno == EAGAIN);
			break;
		}

		total_w += w;
	}
	assert(total_w > 0);

	do {
		sc_time_sleep(50);
		count = sc_sock_poll_wait(&p, timeout);
		assert(count >= 1);
		found = 0;

		for (int i = 0; i < count; i++) {
			ev = sc_sock_poll_event(&p, i);
			sock = sc_sock_poll_data(&p, i);

			if (ev == 0) {
				continue;
			}

			if (sock == &clt) {
				assert(ev & SC_SOCK_READ);
				found++;
			} else if (sock == &acc) {
				assert(ev & SC_SOCK_WRITE);
				found++;
			} else {
				assert(false);
			}
		}
		assert(found == 1 || found == 2);

		char rb;

		for (;;) {
			int r = sc_sock_recv(&clt, &rb, 1, 0);
			if (r < 0) {
				assert(errno == EAGAIN);
				break;
			}

			total_r += r;
		}
	} while (total_r < total_w);
	assert(total_r == total_w);

	sc_time_sleep(50);
	count = sc_sock_poll_wait(&p, timeout);
	assert(count >= 1);
	found = 0;

	for (int i = 0; i < count; i++) {
		ev = sc_sock_poll_event(&p, i);
		sock = sc_sock_poll_data(&p, i);

		if (ev == 0) {
			continue;
		}

		assert(sock == &clt || sock == &acc);
		assert(ev == SC_SOCK_WRITE);
		found++;
	}
	assert(found == 1 || found == 2);

	check_poll_empty(&p, timeout);

	assert(srv.fdt.op == SC_SOCK_READ);
	assert(sc_sock_poll_del(&p, &srv.fdt, SC_SOCK_READ, NULL) == 0);
	assert(srv.fdt.op == SC_SOCK_NONE);

	assert(acc.fdt.op == (SC_SOCK_WRITE | SC_SOCK_READ | SC_SOCK_EDGE));
	assert(sc_sock_poll_del(&p, &acc.fdt, SC_SOCK_READ | SC_SOCK_EDGE,
				NULL) == 0);
	assert(acc.fdt.op == SC_SOCK_WRITE);
	assert(sc_sock_poll_add(&p, &acc.fdt, SC_SOCK_EDGE, NULL) == 0);
	assert(acc.fdt.op == (SC_SOCK_WRITE | SC_SOCK_EDGE));
	assert(sc_sock_poll_del(&p, &acc.fdt, SC_SOCK_WRITE, NULL) == 0);
	assert(acc.fdt.op == SC_SOCK_NONE);

	assert(clt.fdt.op == (SC_SOCK_READ | SC_SOCK_WRITE | SC_SOCK_EDGE));
	assert(sc_sock_poll_del(&p, &clt.fdt, SC_SOCK_EDGE, NULL) == 0);
	assert(clt.fdt.op == (SC_SOCK_READ | SC_SOCK_WRITE));
	assert(sc_sock_poll_del(&p, &clt.fdt, SC_SOCK_READ | SC_SOCK_WRITE,
				NULL) == 0);
	assert(clt.fdt.op == SC_SOCK_NONE);

	assert(sc_sock_poll_add(&p, &acc.fdt, SC_SOCK_READ, NULL) == 0);
	assert(sc_sock_poll_add(&p, &acc.fdt, SC_SOCK_WRITE, NULL) == 0);
	assert(sc_sock_poll_add(&p, &acc.fdt, SC_SOCK_EDGE, NULL) == 0);
	assert(acc.fdt.op == (SC_SOCK_READ | SC_SOCK_WRITE | SC_SOCK_EDGE));
	assert(sc_sock_poll_del(&p, &acc.fdt, SC_SOCK_READ, NULL) == 0);
	assert(acc.fdt.op == (SC_SOCK_WRITE | SC_SOCK_EDGE));

	assert(sc_sock_term(&srv) == 0);
	assert(sc_sock_term(&acc) == 0);
	assert(sc_sock_term(&clt) == 0);
	assert(sc_sock_poll_term(&p) == 0);
}

struct poll_and_sock {
	struct sc_sock_poll *poll;
	struct sc_sock *clt;
	bool visited;
	int index;
};

void *client_poll_add(void *arg)
{
	int rc;

	struct poll_and_sock *ps = (struct poll_and_sock *) arg;
	struct sc_sock_poll *poll = ps->poll;

	ps->clt = sc_sock_malloc(sizeof(struct sc_sock));

	sc_sock_init(ps->clt, 0, false, SC_SOCK_INET);
	rc = sc_sock_connect(ps->clt, "127.0.0.1", "11000", NULL, NULL);
	if (rc == -1) {
		assert(errno == EAGAIN);
	}

	// Sleep to make sure we started waiting on sc_sock_poll_wait() in the
	// main thread.
	sc_time_sleep(1000);

	rc = sc_sock_poll_add(poll, &ps->clt->fdt, SC_SOCK_READ | SC_SOCK_WRITE,
			      ps);
	assert(rc == 0);

	return poll;
}

void *client_poll_del(void *arg)
{
	struct poll_and_sock *ps = (struct poll_and_sock *) arg;
	struct sc_sock_poll *poll = ps->poll;
	struct sc_sock *clt = ps->clt;
	int rc;

	if (ps->index & 1) {
		// Partial delete.
		rc = sc_sock_poll_del(poll, &clt->fdt, SC_SOCK_READ, ps);
		assert(rc == 0);

		if ((ps->index >> 1) & 1) {
			// Add again.
			rc = sc_sock_poll_add(poll, &clt->fdt, SC_SOCK_READ,
					      ps);
			assert(rc == 0);
		}
	}

	// Full delete.
	rc = sc_sock_poll_del(poll, &clt->fdt, SC_SOCK_READ | SC_SOCK_WRITE,
			      NULL);
	assert(rc == 0);

	assert(clt->fdt.op == SC_SOCK_NONE);
	assert(sc_sock_term(clt) == 0);

	// Must be able to free fdt right after successful full delete from
	// poll.
	sc_sock_free(clt);
	ps->clt = NULL;

	return poll;
}

void test_poll_threadsafe(void)
{
	enum
	{
		THREAD_COUNT = 100
	};
	uint32_t ev;
	int rc, count, added = 0;

	struct sc_sock srv;
	struct sc_sock_poll poll;

	struct poll_and_sock pss[THREAD_COUNT];
	struct poll_and_sock *ps;

	int accepted = 0;
	struct sc_sock acc[THREAD_COUNT];

	struct sc_thread add_thread[THREAD_COUNT];
	struct sc_thread del_thread[THREAD_COUNT];

	rc = sc_sock_poll_init(&poll);
	assert(rc == 0);

	sc_sock_init(&srv, 0, false, SC_SOCK_INET);
	rc = sc_sock_listen(&srv, "127.0.0.1", "11000");
	assert(rc == 0);

	// Check that we are actually non-blocking.
	rc = sc_sock_accept(&srv, acc);
	assert(rc != 0);
	assert(errno == EAGAIN);
	printf("server socket ready \n");

	for (int i = 0; i < THREAD_COUNT; i++) {
		pss[i] = (struct poll_and_sock){
			.poll = &poll,
			.index = i,
		};
		sc_thread_init(&add_thread[i]);
		sc_thread_init(&del_thread[i]);
	}

	for (int i = 0; i < THREAD_COUNT; i++) {
		rc = sc_thread_start(&add_thread[i], client_poll_add, &pss[i]);
		assert(rc == 0);
	}
	printf("add-threads started \n");

	while (added < THREAD_COUNT) {
		count = sc_sock_poll_wait(&poll, 7200000);

		if (count < 0) {
			printf("poll err: %s \n", sc_sock_poll_err(&poll));
			assert(count >= 0);
		}

		for (int i = 0; i < count; i++) {
			if (sc_sock_accept(&srv, &acc[accepted]) == 0) {
				accepted++;
			}

			ev = sc_sock_poll_event(&poll, i);
			ps = sc_sock_poll_data(&poll, i);

			assert(ps != NULL);

			if (ev == 0 || ps->visited) {
				continue;
			}

			assert((ev & SC_SOCK_READ) == 0);
			assert(ev & SC_SOCK_WRITE);

			ps->visited = true;
			rc = sc_thread_start(&del_thread[added++],
					     client_poll_del, ps);
			assert(rc == 0);
		}
	}
	printf("added sockets: %d \n", added);
	assert(added == THREAD_COUNT);

	for (int i = 0; i < THREAD_COUNT; i++) {
		assert(pss[i].visited);
	}

	for (int i = 0; i < THREAD_COUNT; i++) {
		void *thread_result;
		rc = sc_thread_join(&add_thread[i], &thread_result);
		assert(rc == 0);
		assert(thread_result == &poll);
	}
	printf("add-threads joined \n");

	for (int i = 0; i < THREAD_COUNT; i++) {
		void *thread_result;
		rc = sc_thread_join(&del_thread[i], &thread_result);
		assert(rc == 0);
		assert(thread_result == &poll);
	}
	printf("del-threads joined \n");

	for (int i = 0; i < accepted; i++) {
		assert(sc_sock_term(&acc[i]) == 0);
	}

	assert(sc_sock_term(&srv) == 0);

#if defined(_WIN32) || defined(_WIN64)
	assert(!poll.polling);
	assert(poll.count == 1);
	assert(poll.ops_cap > 0);
	assert(poll.ops_count == 0);
#endif

	assert(sc_sock_poll_term(&poll) == 0);
	assert(poll.events == NULL);
}

void *multithreaded_accept(void *arg)
{
	struct sc_sock_poll *poll = (struct sc_sock_poll *) arg;
	struct sc_sock acc;

	int n = sc_sock_poll_wait(poll, 2000);
	printf("poll %d \n", n);

	if (n == 0) {
		return poll;
	}

	assert(n == 1);

	enum sc_sock_ev ev = sc_sock_poll_event(poll, 0);
	assert(ev == SC_SOCK_READ);

	struct sc_sock *srv = sc_sock_poll_data(poll, 0);
	assert(srv != NULL);

	int rc = sc_sock_accept(srv, &acc);

	if (rc == 0) {
		rc = sc_sock_term(&acc);
		assert(rc == 0);
		return srv;
	}

	assert(rc == -1);
	assert(errno == EAGAIN);
	return poll;
}

void test_poll_multithreaded_accept(void)
{
	enum
	{
		THREAD_COUNT = 8
	};

	int rc;

	struct sc_sock_poll polls[THREAD_COUNT];
	struct sc_thread threads[THREAD_COUNT];

	struct sc_sock srv, clt;
	sc_sock_init(&srv, 0, false, SC_SOCK_INET);
	rc = sc_sock_listen(&srv, "127.0.0.1", "11000");
	assert(rc == 0);

	for (int i = 0; i < THREAD_COUNT; i++) {
		sc_thread_init(&threads[i]);

		rc = sc_sock_poll_init(&polls[i]);
		assert(rc == 0);

		srv.fdt.op = SC_SOCK_NONE;

		rc = sc_sock_poll_add(&polls[i], &srv.fdt, SC_SOCK_READ, &srv);
		assert(rc == 0);
	}

	for (int i = 0; i < THREAD_COUNT; i++) {
		rc = sc_thread_start(&threads[i], multithreaded_accept, &polls[i]);
		assert(rc == 0);
	}

	sc_time_sleep(500);

	sc_sock_init(&clt, 0, true, SC_SOCK_INET);
	rc = sc_sock_connect(&clt, "127.0.0.1", "11000", NULL, NULL);
	assert(rc == 0);

	printf("client connected \n");

	int accepted = 0;

	for (int i = 0; i < THREAD_COUNT; i++) {
		void *thread_result;
		rc = sc_thread_join(&threads[i], &thread_result);
		assert(rc == 0);

		if (thread_result == &srv) {
			accepted++;
		} else {
			assert(thread_result == &polls[i]);
		}
	}

	printf("accepted %d \n", accepted);
	assert(accepted == 1);

	rc = sc_sock_term(&clt);
	assert(rc == 0);

	for (int i = 0; i < THREAD_COUNT; i++) {
		rc = sc_sock_poll_term(&polls[i]);
		assert(rc == 0);
	}

	rc = sc_sock_term(&srv);
	assert(rc == 0);
}

void test_err(void)
{
	struct sc_sock sock;
	char buf[128];

	sc_sock_init(&sock, 0, true, SC_SOCK_INET);
	assert(sc_sock_listen(&sock, "127.0.0.1x", "8004") != 0);
	sc_sock_init(&sock, 0, true, SC_SOCK_UNIX);
	assert(sc_sock_listen(&sock, "/", "8004") != 0);
	sc_sock_init(&sock, 0, true, SC_SOCK_INET6);
	assert(sc_sock_listen(&sock, "/", "8004") != 0);
	sc_sock_init(&sock, 0, true, SC_SOCK_INET6);
	assert(sc_sock_listen(&sock, "0.0.0.0", "99999") != 0);
	sc_sock_init(&sock, 0, true, SC_SOCK_INET);
	assert(sc_sock_listen(&sock, "0.0.0.3", "99999") != 0);
	sc_sock_init(&sock, 0, true, SC_SOCK_INET6);
	assert(sc_sock_connect(&sock, "::1", "8006", NULL, NULL) != 0);
	sc_sock_init(&sock, 0, true, SC_SOCK_UNIX);
	assert(sc_sock_connect(&sock, "/", "8006", NULL, NULL) != 0);
	sc_sock_init(&sock, 0, true, SC_SOCK_INET);
	assert(sc_sock_connect(&sock, "0.0.0.0", "8006", NULL, NULL) != 0);
	sc_sock_print(&sock, buf, 128);
	sc_sock_local_str(&sock, buf, 128);
	assert(*buf == '\0');
	sc_sock_remote_str(&sock, buf, 128);
	assert(*buf == '\0');

	sc_sock_init(&sock, 0, true, SC_SOCK_INET);
	assert(sc_sock_connect(&sock, "0.0.0.0", "8006", "127.0.0.1", "8080") !=
	       0);
	sc_sock_init(&sock, 0, true, SC_SOCK_INET6);
	assert(sc_sock_connect(&sock, "0.0.0.0", "8006", "::1", "8080") != 0);
	assert(sc_sock_term(&sock) == 0);

	struct sc_sock_poll p = {0};
	assert(sc_sock_poll_wait(&p, 100) != 0);
	assert(*sc_sock_poll_err(&p) != '\0');
}

int main(void)
{
#ifdef SC_HAVE_WRAP
	assert(sc_mutex_init(&mutex) == 0);
#endif

	assert(sc_sock_startup() == 0);

	test1();
	test_ip4();

#if defined(__x86_64__)
	test_ip6();
#endif

#if !defined(__APPLE__)
	test_unix();
#endif
	test_pipe();
	pipe_fail_test();
	poll_fail_test();
	sock_fail_test();
	sock_fail_test2();
	sock_fail_test3();
	test_poll();
	test_err();
	test_poll_mass();
	test_poll_edge();
	test_poll_threadsafe();
	test_poll_multithreaded_accept();

	assert(sc_sock_cleanup() == 0);

#ifdef SC_HAVE_WRAP
	assert(sc_mutex_term(&mutex) == 0);
	test_done = 1;
#endif

	return 0;
}

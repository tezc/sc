#include "sc_uri.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void test1(void)
{
	struct sc_uri *uri;
	const char *f = "foo://user:password@example.com:8042/over/"
			"there?name=ferret#nose";

	uri = sc_uri_create("");
	assert(uri == NULL);
	sc_uri_destroy(&uri);
	sc_uri_destroy(NULL);

	uri = sc_uri_create(f);
	assert(uri != NULL);
	assert(strcmp(uri->str, f) == 0);
	assert(strcmp(uri->scheme, "foo") == 0);
	assert(strcmp(uri->userinfo, "user:password") == 0);
	assert(strcmp(uri->host, "example.com") == 0);
	assert(strcmp(uri->port, "8042") == 0);
	assert(strcmp(uri->path, "/over/there") == 0);
	assert(strcmp(uri->query, "name=ferret") == 0);
	assert(strcmp(uri->fragment, "nose") == 0);

	sc_uri_destroy(&uri);
}

void test2(void)
{
	struct sc_uri *uri;
	const char *f = "https://john.doe@www.example.com:123/forum/questions/"
			"?tag=networking&order=newest#top";

	uri = sc_uri_create(f);
	assert(uri != NULL);
	assert(strcmp(uri->str, f) == 0);
	assert(strcmp(uri->scheme, "https") == 0);
	assert(strcmp(uri->userinfo, "john.doe") == 0);
	assert(strcmp(uri->host, "www.example.com") == 0);
	assert(strcmp(uri->port, "123") == 0);
	assert(strcmp(uri->path, "/forum/questions/") == 0);
	assert(strcmp(uri->query, "tag=networking&order=newest") == 0);
	assert(strcmp(uri->fragment, "top") == 0);

	sc_uri_destroy(&uri);
}

void test3(void)
{
	struct sc_uri *uri;
	const char *f = "ldap://[2001:db8::7]/c=GB?objectClass?one";

	uri = sc_uri_create(f);
	assert(uri != NULL);
	assert(strcmp(uri->str, f) == 0);
	assert(strcmp(uri->scheme, "ldap") == 0);
	assert(strcmp(uri->userinfo, "") == 0);
	assert(strcmp(uri->host, "[2001:db8::7]") == 0);
	assert(strcmp(uri->port, "") == 0);
	assert(strcmp(uri->path, "/c=GB") == 0);
	assert(strcmp(uri->query, "objectClass?one") == 0);
	assert(strcmp(uri->fragment, "") == 0);

	sc_uri_destroy(&uri);
}

void test4(void)
{
	struct sc_uri *uri;
	const char *f = "mailto:John.Doe@example.com";

	uri = sc_uri_create(f);
	assert(uri != NULL);
	assert(strcmp(uri->str, f) == 0);
	assert(strcmp(uri->scheme, "mailto") == 0);
	assert(strcmp(uri->userinfo, "") == 0);
	assert(strcmp(uri->host, "") == 0);
	assert(strcmp(uri->port, "") == 0);
	assert(strcmp(uri->path, "John.Doe@example.com") == 0);
	assert(strcmp(uri->query, "") == 0);
	assert(strcmp(uri->fragment, "") == 0);

	sc_uri_destroy(&uri);
}

void test5(void)
{
	struct sc_uri *uri;
	const char *f = "news:comp.infosystems.www.servers.unix";

	uri = sc_uri_create(f);
	assert(uri != NULL);
	assert(strcmp(uri->str, f) == 0);
	assert(strcmp(uri->scheme, "news") == 0);
	assert(strcmp(uri->userinfo, "") == 0);
	assert(strcmp(uri->host, "") == 0);
	assert(strcmp(uri->port, "") == 0);
	assert(strcmp(uri->path, "comp.infosystems.www.servers.unix") == 0);
	assert(strcmp(uri->query, "") == 0);
	assert(strcmp(uri->fragment, "") == 0);

	sc_uri_destroy(&uri);
}

void test6(void)
{
	struct sc_uri *uri;
	const char *f = "tel:+1-816-555-1212";

	uri = sc_uri_create(f);
	assert(uri != NULL);
	assert(strcmp(uri->str, f) == 0);
	assert(strcmp(uri->scheme, "tel") == 0);
	assert(strcmp(uri->userinfo, "") == 0);
	assert(strcmp(uri->host, "") == 0);
	assert(strcmp(uri->port, "") == 0);
	assert(strcmp(uri->path, "+1-816-555-1212") == 0);
	assert(strcmp(uri->query, "") == 0);
	assert(strcmp(uri->fragment, "") == 0);

	sc_uri_destroy(&uri);
}

void test7(void)
{
	struct sc_uri *uri;
	const char *f = "telnet://192.0.2.16:80/";

	uri = sc_uri_create(f);
	assert(uri != NULL);
	assert(strcmp(uri->str, f) == 0);
	assert(strcmp(uri->scheme, "telnet") == 0);
	assert(strcmp(uri->userinfo, "") == 0);
	assert(strcmp(uri->host, "192.0.2.16") == 0);
	assert(strcmp(uri->port, "80") == 0);
	assert(strcmp(uri->path, "/") == 0);
	assert(strcmp(uri->query, "") == 0);
	assert(strcmp(uri->fragment, "") == 0);

	sc_uri_destroy(&uri);
}

void test8(void)
{
	struct sc_uri *uri;
	const char *f = "urn:oasis:names:specification:docbook:dtd:xml:4.1.2";

	uri = sc_uri_create(f);
	assert(uri != NULL);
	assert(strcmp(uri->str, f) == 0);
	assert(strcmp(uri->scheme, "urn") == 0);
	assert(strcmp(uri->userinfo, "") == 0);
	assert(strcmp(uri->host, "") == 0);
	assert(strcmp(uri->port, "") == 0);
	assert(strcmp(uri->path, "oasis:names:specification:docbook:dtd:xml:4."
				 "1.2") == 0);
	assert(strcmp(uri->query, "") == 0);
	assert(strcmp(uri->fragment, "") == 0);

	sc_uri_destroy(&uri);
}

void test9(void)
{
	struct sc_uri *uri;
	const char *f = "foo://info.example.com?fred";

	uri = sc_uri_create(f);
	assert(uri != NULL);
	assert(strcmp(uri->str, f) == 0);
	assert(strcmp(uri->scheme, "foo") == 0);
	assert(strcmp(uri->userinfo, "") == 0);
	assert(strcmp(uri->host, "info.example.com") == 0);
	assert(strcmp(uri->port, "") == 0);
	assert(strcmp(uri->path, "") == 0);
	assert(strcmp(uri->query, "fred") == 0);
	assert(strcmp(uri->fragment, "") == 0);

	sc_uri_destroy(&uri);
}

void test10(void)
{
	struct sc_uri *uri;
	const char *f = "tcp://127.0.0.1:9090";

	uri = sc_uri_create(f);
	assert(uri != NULL);
	assert(strcmp(uri->str, f) == 0);
	assert(strcmp(uri->scheme, "tcp") == 0);
	assert(strcmp(uri->userinfo, "") == 0);
	assert(strcmp(uri->host, "127.0.0.1") == 0);
	assert(strcmp(uri->port, "9090") == 0);
	assert(strcmp(uri->path, "") == 0);
	assert(strcmp(uri->query, "") == 0);
	assert(strcmp(uri->fragment, "") == 0);

	sc_uri_destroy(&uri);
}

void test11(void)
{
	struct sc_uri *uri = NULL;

	assert(sc_uri_create("127.0.0.1") == NULL);
	assert(sc_uri_create("") == NULL);
	assert(sc_uri_create("/dsad") == NULL);
	assert(sc_uri_create(NULL) == NULL);
	assert(sc_uri_create("http://127.0.0.1:") == NULL);
	assert(sc_uri_create("http://127.0.0.1:88888") == NULL);
	assert(sc_uri_create("ldap://[2001:db8::7/c=GB?objectClass?one") ==
	       NULL);
	assert(sc_uri_create("ldap://[2001:db8::7") == NULL);

	sc_uri_destroy(&uri);
}

void test12(void)
{
	struct sc_uri *uri;
	const char *f = "foo://user:password@example.com:1/over/there?x#3";

	uri = sc_uri_create(f);
	assert(uri != NULL);
	assert(strcmp(uri->str, f) == 0);
	assert(strcmp(uri->scheme, "foo") == 0);
	assert(strcmp(uri->userinfo, "user:password") == 0);
	assert(strcmp(uri->host, "example.com") == 0);
	assert(strcmp(uri->port, "1") == 0);
	assert(strcmp(uri->path, "/over/there") == 0);
	assert(strcmp(uri->query, "x") == 0);
	assert(strcmp(uri->fragment, "3") == 0);

	sc_uri_destroy(&uri);
}

void test13(void)
{
	struct sc_uri *uri;
	const char *f = "foo://user:password@example.com:-1/over/there?x#3";
	const char *f2 = "foo://user:password@example.com:100000/over/"
			 "there?x#3";

	uri = sc_uri_create(f);
	assert(uri == NULL);

	uri = sc_uri_create(f2);
	assert(uri == NULL);
}

#ifdef SC_HAVE_WRAP

bool fail_malloc = false;
void *__real_malloc(size_t n);
void *__wrap_malloc(size_t n)
{
	if (fail_malloc) {
		return NULL;
	}

	return __real_malloc(n);
}

int fail_strtoul;
unsigned long int __real_strtoul(const char *nptr, char **endptr, int base);
unsigned long int __wrap_strtoul(const char *nptr, char **endptr, int base)
{
	if (fail_strtoul == 2) {
		return 100000;
	}

	if (fail_strtoul == 1) {
		errno = EINVAL;
		return -1;
	}

	return __real_strtoul(nptr, endptr, base);
}

int fail_snprintf;
int __wrap_snprintf(char *str, size_t size, const char *format, ...)
{
	int rc;
	va_list va;

	if (!fail_snprintf) {
		va_start(va, format);
		rc = vsnprintf(str, size, format, va);
		va_end(va);

		return rc;
	}

	return fail_snprintf;
}

int fail_sprintf;
int __wrap_sprintf(char *str, const char *format, ...)
{
	int rc;
	va_list va;

	if (!fail_sprintf) {
		va_start(va, format);
		rc = vsprintf(str, format, va);
		va_end(va);

		return rc;
	}

	return fail_sprintf;
}

void fail_test()
{
	struct sc_uri *uri;
	fail_malloc = true;
	assert(sc_uri_create("tcp://127.0.0.1") == NULL);
	fail_malloc = false;

	uri = sc_uri_create("tcp://127.0.0.1");
	assert(uri != NULL);
	sc_uri_destroy(&uri);

	uri = sc_uri_create("tcp:/127.0.0.1");
	assert(uri != NULL);
	assert(strcmp(uri->scheme, "tcp") == 0);
	assert(strcmp(uri->path, "/127.0.0.1") == 0);
	sc_uri_destroy(&uri);

	fail_snprintf = -1;
	assert(sc_uri_create("tcp://127.0.0.1") == NULL);
	fail_snprintf = 0;

	fail_snprintf = 100000;
	assert(sc_uri_create("tcp://127.0.0.1") == NULL);
	fail_snprintf = 0;

	fail_sprintf = -1;
	assert(sc_uri_create("tcp://127.0.0.1") == NULL);
	fail_sprintf = 0;

	fail_sprintf = 1000000;
	assert(sc_uri_create("tcp://127.0.0.1") == NULL);
	fail_sprintf = 0;

	fail_strtoul = 1;
	assert(sc_uri_create("tcp://127.0.0.1:9000") == NULL);
	fail_strtoul = 2;
	assert(sc_uri_create("tcp://127.0.0.1:9000") == NULL);
	fail_strtoul = 0;
}
#else
void fail_test()
{
}
#endif

int main()
{
	test1();
	test2();
	test3();
	test4();
	test5();
	test6();
	test7();
	test8();
	test9();
	test10();
	test11();
	test12();
	test13();
	fail_test();
	return 0;
}

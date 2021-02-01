#include "sc_url.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test1(void)
{
    struct sc_url *url;
    const char* f = "foo://user:password@example.com:8042/over/there?name=ferret#nose";

    url = sc_url_create(f);
    assert(url != NULL);
    assert(strcmp(url->str, f) == 0);
    assert(strcmp(url->scheme, "foo") == 0);
    assert(strcmp(url->userinfo, "user:password") == 0);
    assert(strcmp(url->host, "example.com") == 0);
    assert(strcmp(url->port, "8042") == 0);
    assert(strcmp(url->path, "/over/there") == 0);
    assert(strcmp(url->query, "name=ferret") == 0);
    assert(strcmp(url->fragment, "nose") == 0);

    sc_url_destroy(url);
}

void test2(void)
{
    struct sc_url *url;
    const char* f = "https://john.doe@www.example.com:123/forum/questions/?tag=networking&order=newest#top";

    url = sc_url_create(f);
    assert(url != NULL);
    assert(strcmp(url->str, f) == 0);
    assert(strcmp(url->scheme, "https") == 0);
    assert(strcmp(url->userinfo, "john.doe") == 0);
    assert(strcmp(url->host, "www.example.com") == 0);
    assert(strcmp(url->port, "123") == 0);
    assert(strcmp(url->path, "/forum/questions/") == 0);
    assert(strcmp(url->query, "tag=networking&order=newest") == 0);
    assert(strcmp(url->fragment, "top") == 0);

    sc_url_destroy(url);
}

void test3(void)
{
    struct sc_url *url;
    const char* f = "ldap://[2001:db8::7]/c=GB?objectClass?one";

    url = sc_url_create(f);
    assert(url != NULL);
    assert(strcmp(url->str, f) == 0);
    assert(strcmp(url->scheme, "ldap") == 0);
    assert(strcmp(url->userinfo, "") == 0);
    assert(strcmp(url->host, "[2001:db8::7]") == 0);
    assert(strcmp(url->port, "") == 0);
    assert(strcmp(url->path, "/c=GB") == 0);
    assert(strcmp(url->query, "objectClass?one") == 0);
    assert(strcmp(url->fragment, "") == 0);

    sc_url_destroy(url);
}

void test4(void)
{
    struct sc_url *url;
    const char* f = "mailto:John.Doe@example.com";

    url = sc_url_create(f);
    assert(url != NULL);
    assert(strcmp(url->str, f) == 0);
    assert(strcmp(url->scheme, "mailto") == 0);
    assert(strcmp(url->userinfo, "") == 0);
    assert(strcmp(url->host, "") == 0);
    assert(strcmp(url->port, "") == 0);
    assert(strcmp(url->path, "John.Doe@example.com") == 0);
    assert(strcmp(url->query, "") == 0);
    assert(strcmp(url->fragment, "") == 0);

    sc_url_destroy(url);
}

void test5(void)
{
    struct sc_url *url;
    const char* f = "news:comp.infosystems.www.servers.unix";

    url = sc_url_create(f);
    assert(url != NULL);
    assert(strcmp(url->str, f) == 0);
    assert(strcmp(url->scheme, "news") == 0);
    assert(strcmp(url->userinfo, "") == 0);
    assert(strcmp(url->host, "") == 0);
    assert(strcmp(url->port, "") == 0);
    assert(strcmp(url->path, "comp.infosystems.www.servers.unix") == 0);
    assert(strcmp(url->query, "") == 0);
    assert(strcmp(url->fragment, "") == 0);

    sc_url_destroy(url);
}

void test6(void)
{
    struct sc_url *url;
    const char* f = "tel:+1-816-555-1212";

    url = sc_url_create(f);
    assert(url != NULL);
    assert(strcmp(url->str, f) == 0);
    assert(strcmp(url->scheme, "tel") == 0);
    assert(strcmp(url->userinfo, "") == 0);
    assert(strcmp(url->host, "") == 0);
    assert(strcmp(url->port, "") == 0);
    assert(strcmp(url->path, "+1-816-555-1212") == 0);
    assert(strcmp(url->query, "") == 0);
    assert(strcmp(url->fragment, "") == 0);

    sc_url_destroy(url);
}

void test7(void)
{
    struct sc_url *url;
    const char* f = "telnet://192.0.2.16:80/";

    url = sc_url_create(f);
    assert(url != NULL);
    assert(strcmp(url->str, f) == 0);
    assert(strcmp(url->scheme, "telnet") == 0);
    assert(strcmp(url->userinfo, "") == 0);
    assert(strcmp(url->host, "192.0.2.16") == 0);
    assert(strcmp(url->port, "80") == 0);
    assert(strcmp(url->path, "/") == 0);
    assert(strcmp(url->query, "") == 0);
    assert(strcmp(url->fragment, "") == 0);

    sc_url_destroy(url);
}

void test8(void)
{
    struct sc_url *url;
    const char* f = "urn:oasis:names:specification:docbook:dtd:xml:4.1.2";

    url = sc_url_create(f);
    assert(url != NULL);
    assert(strcmp(url->str, f) == 0);
    assert(strcmp(url->scheme, "urn") == 0);
    assert(strcmp(url->userinfo, "") == 0);
    assert(strcmp(url->host, "") == 0);
    assert(strcmp(url->port, "") == 0);
    assert(strcmp(url->path, "oasis:names:specification:docbook:dtd:xml:4.1.2") == 0);
    assert(strcmp(url->query, "") == 0);
    assert(strcmp(url->fragment, "") == 0);

    sc_url_destroy(url);
}

void test9(void)
{
    struct sc_url *url;
    const char* f = "foo://info.example.com?fred";

    url = sc_url_create(f);
    assert(url != NULL);
    assert(strcmp(url->str, f) == 0);
    assert(strcmp(url->scheme, "foo") == 0);
    assert(strcmp(url->userinfo, "") == 0);
    assert(strcmp(url->host, "info.example.com") == 0);
    assert(strcmp(url->port, "") == 0);
    assert(strcmp(url->path, "") == 0);
    assert(strcmp(url->query, "fred") == 0);
    assert(strcmp(url->fragment, "") == 0);

    sc_url_destroy(url);
}

void test10(void)
{
    struct sc_url *url;
    const char* f = "tcp://127.0.0.1:9090";

    url = sc_url_create(f);
    assert(url != NULL);
    assert(strcmp(url->str, f) == 0);
    assert(strcmp(url->scheme, "tcp") == 0);
    assert(strcmp(url->userinfo, "") == 0);
    assert(strcmp(url->host, "127.0.0.1") == 0);
    assert(strcmp(url->port, "9090") == 0);
    assert(strcmp(url->path, "") == 0);
    assert(strcmp(url->query, "") == 0);
    assert(strcmp(url->fragment, "") == 0);

    sc_url_destroy(url);
}

void test11(void)
{
    struct sc_url *url = NULL;

    assert(sc_url_create("127.0.0.1") == NULL);
    assert(sc_url_create("") == NULL);
    assert(sc_url_create("/dsad") == NULL);
    assert(sc_url_create(NULL) == NULL);
    assert(sc_url_create("http://127.0.0.1:") == NULL);
    assert(sc_url_create("http://127.0.0.1:88888") == NULL);
    assert(sc_url_create("ldap://[2001:db8::7/c=GB?objectClass?one") == NULL);
    assert(sc_url_create("ldap://[2001:db8::7") == NULL);


    sc_url_destroy(url);
}

void test12(void)
{
    struct sc_url *url;
    const char* f = "foo://user:password@example.com:1/over/there?x#3";

    url = sc_url_create(f);
    assert(url != NULL);
    assert(strcmp(url->str, f) == 0);
    assert(strcmp(url->scheme, "foo") == 0);
    assert(strcmp(url->userinfo, "user:password") == 0);
    assert(strcmp(url->host, "example.com") == 0);
    assert(strcmp(url->port, "1") == 0);
    assert(strcmp(url->path, "/over/there") == 0);
    assert(strcmp(url->query, "x") == 0);
    assert(strcmp(url->fragment, "3") == 0);

    sc_url_destroy(url);
}

void test13(void)
{
    struct sc_url *url;
    const char* f = "foo://user:password@example.com:-1/over/there?x#3";
    const char* f2 = "foo://user:password@example.com:100000/over/there?x#3";

    url = sc_url_create(f);
    assert(url == NULL);

    url = sc_url_create(f2);
    assert(url == NULL);
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

void fail_test()
{
    struct sc_url *url;
    fail_malloc = true;
    assert(sc_url_create("tcp://127.0.0.1") == NULL);
    fail_malloc = false;

    url = sc_url_create("tcp://127.0.0.1");
    assert(url != NULL);
    sc_url_destroy(url);

    url = sc_url_create("tcp:/127.0.0.1");
    assert(url != NULL);
    assert(strcmp(url->scheme, "tcp") == 0);
    assert(strcmp(url->path, "/127.0.0.1") == 0);
    sc_url_destroy(url);
}
#else
void fail_test()
{

}
#endif

int main(int argc, char *argv[])
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

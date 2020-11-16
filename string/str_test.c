#include "sc_str.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

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

bool fail_realloc = false;
void *__real_realloc(void *p, size_t size);
void *__wrap_realloc(void *p, size_t n)
{
    if (fail_realloc) {
        return NULL;
    }

    return __real_realloc(p, n);
}

bool fail_strlen = false;
size_t __real_strlen(const char* str);
size_t __wrap_strlen(const char* str)
{
    if (fail_strlen) {
        return UINT32_MAX;
    }

    return __real_strlen(str);
}

bool fail_vsnprintf;
int fail_vsnprintf_at = -1;
extern int __real_vsnprintf(char *str, size_t size, const char *format,
                            va_list ap);
int __wrap_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    fail_vsnprintf_at--;
    if (!fail_vsnprintf && (fail_vsnprintf_at) != 0) {
        return __real_vsnprintf(str, size, format, ap);
    }

    return -1;
}

void test1()
{
    assert(sc_str_len(NULL) == -1);
    sc_str_destroy(NULL);
    assert(sc_str_dup(NULL) == NULL);

    char *s1 = sc_str_create("test");
    char *s2 = sc_str_create("test");
    assert(sc_str_len(s1) == 4);
    assert(sc_str_cmp(s1, s2));
    sc_str_set(&s2, "tett");
    assert(sc_str_cmp(s1, s2) == false);
    assert(sc_str_set_fmt(&s1, "test%d", 3) == true);
    assert(strcmp(s1, "test3") == 0);
    fail_malloc = true;
    assert(sc_str_set_fmt(&s1, "test%d", 5) == false);
    assert(strcmp(s1, "test3") == 0);
    fail_malloc = false;

    sc_str_destroy(s1);
    sc_str_destroy(s2);

    fail_malloc = true;
    assert(sc_str_create("test") == NULL);
    fail_malloc = false;

    s1 = malloc(SC_SIZE_MAX + 2);
    memset(s1, 'c', SC_SIZE_MAX + 1);
    s1[SC_SIZE_MAX + 1] = '\0';
    assert(sc_str_create(s1) == NULL);
    free(s1);

    s1 = sc_str_create_fmt("%dtest%d", 5, 5);
    assert(strcmp(s1, "5test5") == 0);
    s2 = sc_str_dup(s1);
    assert(sc_str_cmp(s1, s2) == true);
    sc_str_destroy(s1);
    sc_str_destroy(s2);

    fail_malloc = true;
    s1 = sc_str_create_fmt("%dtest%d", 5, 5);
    assert(s1 == NULL);
    fail_malloc = false;

    s1 = sc_str_create_len("test", 4);
    assert(strcmp(s1, "test") == 0);
    assert(sc_str_len(s1) == 4);
    assert(sc_str_set(&s1, "testtest") == true);
    assert(strcmp(s1, "testtest") == 0);
    assert(sc_str_len(s1) == 8);
    fail_malloc = true;
    fail_realloc = true;
    assert(sc_str_set(&s1, "test") == false);
    assert(strcmp(s1, "testtest") == 0);
    assert(sc_str_len(s1) == 8);
    assert(sc_str_append(&s1, "test") == false);
    assert(sc_str_append_fmt(&s1, "%s", "test") == false);
    assert(strcmp(s1, "testtest") == 0);
    assert(sc_str_len(s1) == 8);
    fail_malloc = false;
    fail_realloc = false;

    sc_str_set(&s1, "text");
    sc_str_append(&s1, "2");
    sc_str_append_fmt(&s1, "%d", 3);
    assert(strcmp(s1, "text23") == 0);
    sc_str_set(&s1, " \n\n;;;;*test ;------;");
    sc_str_trim(&s1, " \n;*-");
    assert(strcmp(s1, "test") == 0);
    sc_str_substring(&s1, 2, 4);
    assert(strcmp(s1, "st") == 0);
    assert(sc_str_substring(&s1, 4, 5) == false);
    assert(sc_str_substring(&s1, 1, 5) == false);

    sc_str_set(&s1, "test");
    fail_malloc = true;
    assert(sc_str_substring(&s1, 1, 2) == false);
    fail_malloc = false;

    fail_vsnprintf = true;
    assert(sc_str_set_fmt(&s1, "test%d", 3) == false);
    assert(strcmp(s1, "test") == 0);
    fail_vsnprintf = false;


    fail_vsnprintf_at = 2;
    s2 = malloc(2000 + 2);
    memset(s2, 'c', 2000 + 1);
    s2[2000 + 1] = '\0';
    assert(sc_str_set_fmt(&s1, "test%s", s2) == false);
    assert(strcmp(s1, "test") == 0);
    fail_vsnprintf_at = -1;
    free(s2);
    sc_str_destroy(s1);

    fail_malloc = false;
    fail_realloc = false;
    fail_vsnprintf = false;
}

void test2()
{
    char buf[4000];
    memset(buf, 'x', 4000);
    buf[3999] = '\0';

    char *c = sc_str_create("--a**00");
    fail_malloc = true;
    assert(sc_str_trim(&c, "-*0") == false);
    fail_malloc = false;
    sc_str_trim(&c, "-*0");
    assert(*c == 'a');
    sc_str_trim(&c, "a");
    assert(*c == '\0');

    sc_str_set(&c, "x003");
    sc_str_trim(&c, "03");
    assert(strcmp(c, "x") == 0);
    sc_str_set(&c, "\n\r\nx");
    sc_str_trim(&c, "\n\r");
    assert(strcmp(c, "x") == 0);

    sc_str_set(&c, "test****");
    fail_malloc = true;
    assert(sc_str_replace(&c, "*", "-"));
    assert(strcmp(c, "test----") == 0);
    assert(!sc_str_replace(&c, "-", ""));
    assert(strcmp(c, "test----") == 0);
    fail_malloc = false;
    assert(!sc_str_replace(&c, "----", buf));
    assert(strcmp(c, "test----") == 0);
    sc_str_replace(&c, "--", "0");
    assert(strcmp(c, "test00") == 0);
    fail_strlen = true;
    assert(sc_str_replace(&c, "*", "2") == false);
    fail_strlen = false;
    sc_str_destroy(c);
}

#endif

void test3()
{
    const char *tokens = "token;token;token;token";
    char *save = NULL;
    const char *token;
    char *str = sc_str_create(tokens);
    int count = 0;

    while ((token = sc_str_token_begin(str, &save, ";")) != NULL) {
        assert(strcmp(token, "token") == 0);
        count++;
    }

    assert(count == 4);
    assert(strcmp(str, tokens) == 0);
    assert(sc_str_len(str) == strlen(tokens));

    count = 0;
    save = NULL;
    while ((token = sc_str_token_begin(str, &save, ";")) != NULL) {
        assert(strcmp(token, "token") == 0);
        count++;
    }
    sc_str_token_end(str, &save);

    assert(count == 4);
    assert(strcmp(str, tokens) == 0);
    assert(sc_str_len(str) == strlen(tokens));

    count = 0;
    save = NULL;
    while ((token = sc_str_token_begin(str, &save, ";")) != NULL) {
        assert(strcmp(token, "token") == 0);
        count++;
        if (count == 4) {
            break;
        }
    }
    sc_str_token_end(str, &save);

    assert(count == 4);
    assert(strcmp(str, tokens) == 0);
    assert(sc_str_len(str) == strlen(tokens));
    sc_str_destroy(str);


}

void test4()
{
    char *save = NULL;
    const char *token;
    char *str;
    char list[10][10];
    int count = 0;

    str = sc_str_create(";;;");
    while ((token = sc_str_token_begin(str, &save, ";")) != NULL) {
        assert(strcmp(token, "") == 0);
        count++;
    }
    assert(count == 4);

    save = NULL;
    count = 0;
    sc_str_set(&str, "tk1;tk2-tk3 tk4  tk5*tk6");
    while ((token = sc_str_token_begin(str, &save, ";- *")) != NULL) {
        strcpy(list[count], token);
        count++;
    }

    assert(count == 7);
    assert(strcmp(list[0], "tk1") == 0);
    assert(strcmp(list[1], "tk2") == 0);
    assert(strcmp(list[2], "tk3") == 0);
    assert(strcmp(list[3], "tk4") == 0);
    assert(strcmp(list[4], "") == 0);
    assert(strcmp(list[5], "tk5") == 0);
    assert(strcmp(list[6], "tk6") == 0);

    sc_str_token_end(str, &save);


    save = NULL;
    count = 0;
    sc_str_set(&str, "tk1;tk2-tk3 tk4  tk5*tk6");
    while ((token = sc_str_token_begin(str, &save, ";- *")) != NULL) {
        strcpy(list[count], token);
        count++;
        if (count == 3) {
            break;
        }
    }

    sc_str_token_end(str, &save);
    assert(strcmp(str, "tk1;tk2-tk3 tk4  tk5*tk6") == 0);

    save = NULL;
    count = 0;
    sc_str_set(&str, "tk1;tk2-tk3 tk4  tk5*tk6");
    while ((token = sc_str_token_begin(str, &save, ";- *")) != NULL) {
        strcpy(list[count], token);
        count++;
        if (count == 3) {
            break;
        }
    }

    sc_str_token_end(str, NULL);
    assert(strcmp(str, "tk1;tk2-tk3 tk4  tk5*tk6") == 0);

    sc_str_destroy(str);
}

void test5()
{
    char* s1, *s2;

    s1 = sc_str_create("test");
    assert(strcmp(s1, "test") == 0);
    assert(sc_str_len(s1) == 4);
    s2 = sc_str_dup(s1);
    assert(strcmp(s2, "test") == 0);
    assert(sc_str_len(s2) == 4);
    assert(strcmp(s1, s2) == 0);
    sc_str_destroy(s2);

    sc_str_set(&s1, "test2");
    assert(strcmp(s1, "test2") == 0);
    sc_str_set_fmt(&s1, "test%d", 3);
    assert(strcmp(s1, "test3") == 0);
    sc_str_append(&s1, "5");
    assert(strcmp(s1, "test35") == 0);
    sc_str_append_fmt(&s1, "%d", 7);
    assert(strcmp(s1, "test357") == 0);
    sc_str_substring(&s1, 0, 4);
    assert(strcmp(s1, "test") == 0);
    sc_str_trim(&s1, "tes");
    assert(strcmp(s1, "") == 0);
    sc_str_set_fmt(&s1, "-;;;- \n \n \n 351234");
    sc_str_trim(&s1, "-; 34\n");
    assert(strcmp(s1, "512") == 0);

    sc_str_set(&s1, "test t1t1 test");
    sc_str_replace(&s1, "t1t1", "-");
    assert(strcmp(s1, "test - test") == 0);
    sc_str_replace(&s1, "test", "longer");
    assert(strcmp(s1, "longer - longer") == 0);
    sc_str_replace(&s1, "-", "");
    sc_str_replace(&s1, " ", "");
    assert(strcmp(s1, "longerlonger") == 0);
    assert(sc_str_len(s1) == strlen("longerlonger"));
    assert(sc_str_replace(&s1, "as", "r"));
    assert(strcmp(s1, "longerlonger") == 0);
    assert(sc_str_replace(&s1, "r", "R"));
    assert(strcmp(s1, "longeRlongeR") == 0);
    assert(sc_str_replace(&s1, "longeR", ""));
    assert(strcmp(s1, "") == 0);
    sc_str_destroy(s1);
}


int main(int argc, char *argv[])
{

#ifdef SC_HAVE_WRAP
    test1();
    test2();
#endif
    test3();
    test4();
    test5();
    return 0;
}

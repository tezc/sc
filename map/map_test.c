#include "sc_map.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void example(void)
{
    const char *key, *value;
    struct sc_map_str map;

    sc_map_init_str(&map, 0, 0);

    sc_map_put_str(&map, "jack", "chicago");
    sc_map_put_str(&map, "jane", "new york");
    sc_map_put_str(&map, "janie", "atlanta");

    sc_map_foreach (&map, key, value) {
        printf("Key:[%s], Value:[%s] \n", key, value);
    }

    sc_map_term_str(&map);
}

static char *str_random(size_t size)
{
    static char ch[] = "0123456789"
                       "abcdefghijklmnopqrstuvwxyz"
                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    uint32_t index;
    char *dest = malloc(size + 1);

    for (int i = 0; i < size; ++i) {
        index = (uint32_t)((double) rand() / RAND_MAX * (sizeof(ch) - 1));
        dest[i] = ch[index];
    }

    dest[size - 1] = '\0';

    return dest;
}

void test1()
{
    struct sc_map_str map;
    char *keys[128];
    char *values[128];
    const char *key, *value;

    for (int i = 0; i < 128; i++) {
        keys[i] = str_random((rand() % 64) + 32);
        values[i] = str_random((rand() % 64) + 32);
    }

    assert(!sc_map_init_str(&map, 0, -1));
    assert(!sc_map_init_str(&map, 0, 24));
    assert(!sc_map_init_str(&map, 0, 96));
    assert(sc_map_init_str(&map, 0, 0));
    assert(sc_map_size_str(&map) == 0);
    sc_map_clear_str(&map);
    assert(sc_map_size_str(&map) == 0);
    sc_map_term_str(&map);
    assert(sc_map_init_str(&map, 0, 0));

    sc_map_foreach (&map, key, value) {
        assert(false);
    }

    sc_map_foreach_key (&map, key) {
        assert(false);
    }

    sc_map_foreach_value (&map, value) {
        assert(false);
    }

    assert(sc_map_put_str(&map, "key", "value"));
    assert(sc_map_put_str(&map, "key", "value2"));
    assert(sc_map_get_str(&map, "key", &value));
    assert(strcmp(value, "value2") == 0);

    assert(sc_map_del_str(&map, "key", NULL));
    assert(!sc_map_get_str(&map, "key", &value));
    assert(sc_map_put_str(&map, "key", "value3"));
    assert(sc_map_del_str(&map, "key", &value));
    assert(strcmp(value, "value3") == 0);
    assert(!sc_map_del_str(&map, "key", &value));

    assert(sc_map_put_str(&map, "key", "value"));
    assert(sc_map_size_str(&map) == 1);
    assert(sc_map_put_str(&map, NULL, "nullvalue"));
    assert(sc_map_size_str(&map) == 2);
    assert(sc_map_get_str(&map, NULL, &value));
    assert(strcmp(value, "nullvalue") == 0);
    assert(sc_map_del_str(&map, NULL, NULL));
    assert(sc_map_size_str(&map) == 1);

    sc_map_clear_str(&map);
    assert(sc_map_size_str(&map) == 0);

    for (int i = 0; i < 100; i++) {
        assert(sc_map_put_str(&map, keys[i], values[i]));
    }

    for (int i = 0; i < 100; i++) {
        assert(sc_map_get_str(&map, keys[i], &value));
        assert(strcmp(value, values[i]) == 0);
    }

    sc_map_put_str(&map, keys[0], values[101]);
    assert(sc_map_size_str(&map) == 100);
    sc_map_put_str(&map, keys[101], values[102]);
    assert(sc_map_size_str(&map) == 101);
    sc_map_clear_str(&map);
    assert(sc_map_size_str(&map) == 0);

    for (int i = 0; i < 100; i++) {
        assert(sc_map_put_str(&map, keys[i], values[i]));
    }

    for (int i = 0; i < 100; i++) {
        assert(sc_map_get_str(&map, keys[i], &value));
        assert(strcmp(value, values[i]) == 0);
    }

    sc_map_term_str(&map);

    assert(sc_map_init_str(&map, 0, 0));
    for (int i = 0; i < 100; i++) {
        assert(sc_map_put_str(&map, keys[i], values[i]));
    }

    bool found;
    sc_map_foreach (&map, key, value) {
        found = false;
        for (int j = 0; j < 100; j++) {
            if (strcmp(key, keys[j]) == 0 && strcmp(value, values[j]) == 0) {
                found = true;
                break;
            }
        }
        assert(found);
    }

    sc_map_foreach_key (&map, key) {
        found = false;
        for (int j = 0; j < 100; j++) {
            if (strcmp(key, keys[j]) == 0) {
                found = true;
                break;
            }
        }
        assert(found);
    }

    sc_map_foreach_value (&map, value) {
        found = false;
        for (int j = 0; j < 100; j++) {
            if (strcmp(value, values[j]) == 0) {
                found = true;
                break;
            }
        }
        assert(found);
    }

    sc_map_term_str(&map);
    for (int i = 0; i < 128; i++) {
        free(keys[i]);
        free(values[i]);
    }
}

void test2()
{
    struct sc_map_32 map;
    uint32_t keys[128];
    uint32_t values[128];
    uint32_t key, value;
    int random;

    for (int i = 0; i < 128; i++) {
retry:
        random = rand();
        for (int j = 0; j < i; j++) {
            if (keys[j] == random) {
                goto retry;
            }
        }

        keys[i] = random;
        values[i] = rand();
    }

    assert(sc_map_init_32(&map, 16, 50));
    assert(sc_map_size_32(&map) == 0);
    assert(sc_map_put_32(&map, 0, 0));
    sc_map_clear_32(&map);
    assert(sc_map_size_32(&map) == 0);

    for (int i = 0; i < 100; i++) {
        assert(sc_map_put_32(&map, keys[i], values[i]));
        assert(sc_map_get_32(&map, keys[i], &value));
        assert(value == values[i]);
        assert(sc_map_put_32(&map, keys[i], values[i]));
        assert(sc_map_del_32(&map, keys[i], &value));
        assert(value == values[i]);
    }

    for (int i = 0; i < 128; i++) {
        assert(sc_map_put_32(&map, keys[i], values[i]));
    }

    assert(sc_map_size_32(&map) == 128);

    bool found;
    sc_map_foreach (&map, key, value) {
        found = false;
        for (int j = 0; j < 128; j++) {
            if (key == keys[j] && value == values[j]) {
                found = true;
                break;
            }
        }
        assert(found);
    }

    sc_map_foreach_key (&map, key) {
        found = false;
        for (int j = 0; j < 128; j++) {
            if (key == keys[j]) {
                found = true;
                break;
            }
        }
        assert(found);
    }

    sc_map_foreach_value (&map, value) {
        found = false;
        for (int j = 0; j < 128; j++) {
            if (value == values[j]) {
                found = true;
                break;
            }
        }
        assert(found);
    }

    sc_map_term_32(&map);
}

void test3()
{
    struct sc_map_64 map;
    uint64_t keys[128];
    uint64_t values[128];
    uint64_t key, value;
    int random;

    for (int i = 0; i < 128; i++) {
retry:
        random = rand();
        for (int j = 0; j < i; j++) {
            if (keys[j] == random) {
                goto retry;
            }
        }

        keys[i] = random;
        values[i] = rand();
    }

    assert(sc_map_init_64(&map, 16, 50));
    assert(sc_map_size_64(&map) == 0);
    assert(sc_map_put_64(&map, 0, 0));
    sc_map_clear_64(&map);
    assert(sc_map_size_64(&map) == 0);

    for (int i = 0; i < 100; i++) {
        assert(sc_map_put_64(&map, keys[i], values[i]));
        assert(sc_map_get_64(&map, keys[i], &value));
        assert(value == values[i]);
        assert(sc_map_put_64(&map, keys[i], values[i]));
        assert(sc_map_del_64(&map, keys[i], &value));
        assert(value == values[i]);
    }

    for (int i = 0; i < 128; i++) {
        assert(sc_map_put_64(&map, keys[i], values[i]));
    }

    assert(sc_map_size_64(&map) == 128);

    bool found;
    sc_map_foreach (&map, key, value) {
        found = false;
        for (int j = 0; j < 128; j++) {
            if (key == keys[j] && value == values[j]) {
                found = true;
                break;
            }
        }
        assert(found);
    }

    sc_map_foreach_key (&map, key) {
        found = false;
        for (int j = 0; j < 128; j++) {
            if (key == keys[j]) {
                found = true;
                break;
            }
        }
        assert(found);
    }

    sc_map_foreach_value (&map, value) {
        found = false;
        for (int j = 0; j < 128; j++) {
            if (value == values[j]) {
                found = true;
                break;
            }
        }
        assert(found);
    }

    sc_map_term_64(&map);
}

void test4()
{
    const char *c;
    struct sc_map_64s map64s;

    assert(sc_map_init_64s(&map64s, 1, 87));
    for (int i = 0; i < 100; i++) {
        assert(sc_map_put_64s(&map64s, i, NULL));
        assert(sc_map_get_64s(&map64s, i, &c));
        assert(c == NULL);
    }
    assert(sc_map_size_64s(&map64s) == 100);
    for (int i = 0; i < 100; i++) {
        assert(sc_map_del_64s(&map64s, i, &c));
        assert(c == NULL);
    }
    assert(sc_map_size_64s(&map64s) == 0);
    assert(sc_map_put_64s(&map64s, 3, NULL));
    assert(sc_map_size_64s(&map64s) == 1);
    sc_map_clear_64s(&map64s);
    assert(sc_map_size_64s(&map64s) == 0);

    sc_map_term_64s(&map64s);

    void *v;
    struct sc_map_64v map64v;

    assert(sc_map_init_64v(&map64v, 1, 87));
    for (int i = 0; i < 100; i++) {
        assert(sc_map_put_64v(&map64v, i, NULL));
        assert(sc_map_get_64v(&map64v, i, &v));
        assert(c == NULL);
    }
    assert(sc_map_size_64v(&map64v) == 100);
    for (int i = 0; i < 100; i++) {
        assert(sc_map_del_64v(&map64v, i, &v));
        assert(v == NULL);
    }
    assert(sc_map_size_64v(&map64v) == 0);
    assert(sc_map_put_64v(&map64v, 3, NULL));
    assert(sc_map_size_64v(&map64v) == 1);
    sc_map_clear_64v(&map64v);
    assert(sc_map_size_64v(&map64v) == 0);

    sc_map_term_64v(&map64v);

    char *keys[128];
    char *values[128];

    for (int i = 0; i < 128; i++) {
        keys[i] = str_random((rand() % 64) + 32);
        values[i] = str_random((rand() % 64) + 32);
    }

    struct sc_map_sv mapsv;

    assert(sc_map_init_sv(&mapsv, 1, 87));
    for (int i = 0; i < 100; i++) {
        assert(sc_map_put_sv(&mapsv, keys[i], values[i]));
        assert(sc_map_get_sv(&mapsv, keys[i], &v));
        assert(v == values[i]);
    }
    assert(sc_map_size_sv(&mapsv) == 100);
    assert(sc_map_del_sv(&mapsv, keys[0], &v));
    assert(v == values[0]);
    assert(sc_map_size_sv(&mapsv) == 99);
    assert(!sc_map_del_sv(&mapsv, keys[0], &v));
    sc_map_clear_sv(&mapsv);
    assert(sc_map_size_sv(&mapsv) == 0);
    sc_map_term_sv(&mapsv);

    uint64_t val;
    struct sc_map_s64 maps64;

    assert(sc_map_init_s64(&maps64, 0, 26));
    for (int i = 0; i < 64; i++) {
        assert(sc_map_put_s64(&maps64, keys[i], i));
    }

    assert(sc_map_get_s64(&maps64, keys[0], &val));
    assert(val == 0);
    assert(sc_map_size_s64(&maps64) == 64);
    assert(sc_map_del_s64(&maps64, keys[12], &val));
    assert(val == 12);
    assert(sc_map_size_s64(&maps64) == 63);
    sc_map_clear_s64(&maps64);
    sc_map_term_s64(&maps64);

    for (int i = 0; i < 128; i++) {
        free(keys[i]);
        free(values[i]);
    }
}

static void test5()
{
    int t = 0;
    uint64_t key, value;
    struct sc_map_64 map;

    sc_map_init_64(&map, 0, 0);
    sc_map_put_64(&map, 0, 111);

    t = 0;
    sc_map_foreach (&map, key, value) {
        assert(key == 0);
        assert(value == 111);

        t++;
    }
    assert(t == 1);

    t = 0;
    sc_map_foreach_key (&map, key) {
        assert(key == 0);
        t++;
    }
    assert(t == 1);

    t = 0;
    sc_map_foreach_value (&map, value) {
        assert(value == 111);
        t++;
    }
    assert(t == 1);

    sc_map_term_64(&map);
}


#ifdef SC_HAVE_WRAP

bool fail_calloc = false;
void *__real_calloc(size_t n, size_t size);
void *__wrap_calloc(size_t n, size_t size)
{
    if (fail_calloc) {
        return NULL;
    }

    return __real_calloc(n, size);
}

void fail_test()
{
    struct sc_map_32 map;

    fail_calloc = true;
    assert(!sc_map_init_32(&map, 10, 0));
    fail_calloc = false;
    assert(sc_map_init_32(&map, 10, 0));

    fail_calloc = true;
    bool success = true;
    for (int i = 0; i < 20; i++) {
        success = sc_map_put_32(&map, i, i);
    }
    assert(!success);
    fail_calloc = false;
    assert(sc_map_put_32(&map, 44444, 44444));

    for (int i = 0; i < SC_SIZE_MAX; i++) {
        success = sc_map_put_32(&map, i, i);
    }
    assert(!success);
    fail_calloc = false;

    sc_map_term_32(&map);
}
#else
void fail_test(void)
{
}
#endif

int main(int argc, char *argv[])
{
    example();
    fail_test();
    test1();
    test2();
    test3();
    test4();
    test5();

    return 0;
}

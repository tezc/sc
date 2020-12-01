#include "sc_buf.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>


void test1()
{
    struct sc_buf buf, buf2;

    sc_buf_init(&buf2, 100);
    sc_buf_init(&buf, 100);

    assert(sc_buf_bool_len(true) == 1);
    assert(sc_buf_8bit_len(true) == 1);
    assert(sc_buf_16bit_len(true) == 2);
    assert(sc_buf_32bit_len(true) == 4);
    assert(sc_buf_64bit_len(true) == 8);
    assert(sc_buf_double_len(true) == 8);
    assert(sc_buf_strlen("test") == 9);
    assert(sc_buf_strlen(NULL) == 4);
    assert(sc_buf_blob_len("test", 4) == 8);

    sc_buf_put_8(&buf, 8);
    assert(sc_buf_get_8(&buf) == 8);
    sc_buf_put_16(&buf, 65111);
    assert(sc_buf_get_16(&buf) == 65111);
    sc_buf_put_32(&buf, 2132132131);
    assert(sc_buf_get_32(&buf) == 2132132131);
    sc_buf_put_64(&buf, UINT64_C(2132132213122131));
    assert(sc_buf_get_64(&buf) == UINT64_C(2132132213122131));
    sc_buf_put_double(&buf, 123211.323321);
    double x = sc_buf_get_double(&buf);
    assert(x == (double)123211.323321);
    sc_buf_put_str(&buf, "test");
    assert(strcmp("test", sc_buf_get_str(&buf)) == 0);
    sc_buf_put_str(&buf, NULL);
    assert(sc_buf_get_str(&buf) == NULL);
    sc_buf_clear(&buf);
    assert(sc_buf_size(&buf) == 0);
    assert(sc_buf_cap(&buf) == 100);
    sc_buf_compact(&buf);
    sc_buf_put_fmt(&buf, "%d", 3);
    assert(strcmp("3", sc_buf_get_str(&buf)) == 0);
    sc_buf_put_bool(&buf, true);
    assert(sc_buf_get_bool(&buf) == true);
    sc_buf_put_bool(&buf, false);
    assert(sc_buf_get_bool(&buf) == false);
    sc_buf_put_blob(&buf, "test", 5);
    assert(strcmp("test", sc_buf_get_blob(&buf, sc_buf_get_32(&buf))) == 0);
    sc_buf_clear(&buf);

    sc_buf_put_64(&buf, 122);
    sc_buf_put_64(&buf, 133);
    sc_buf_get_64(&buf);
    assert(sc_buf_get_64(&buf) == 133);
    sc_buf_clear(&buf);

    sc_buf_put_32(&buf, 122);
    sc_buf_put_32(&buf, 144);
    sc_buf_get_32(&buf);
    assert(sc_buf_get_32(&buf) == 144);
    sc_buf_clear(&buf);
    sc_buf_put_64(&buf, 222);
    sc_buf_mark_read(&buf, 8);
    assert(sc_buf_size(&buf) == 0);
    char *c = sc_buf_write_buf(&buf);
    *c = 'c';
    sc_buf_mark_write(&buf, 1);
    assert(sc_buf_get_8(&buf) == 'c');
    sc_buf_clear(&buf);

    sc_buf_clear(&buf);
    sc_buf_put_32(&buf, 2323);
    sc_buf_put_32(&buf, 3311);
    sc_buf_set_write_pos(&buf, 8);
    sc_buf_set_read_pos(&buf, 4);
    assert(sc_buf_get_32(&buf) == 3311);
    sc_buf_clear(&buf);
    sc_buf_put_64(&buf, UINT64_MAX);
    assert(sc_buf_get_64(&buf) == UINT64_MAX);
    sc_buf_put_64(&buf, UINT64_MAX);

    unsigned char* z = sc_buf_read_buf(&buf);
    assert(*z == 0xFF);
    assert(sc_buf_get_write_pos(&buf) == 16);
    assert(sc_buf_quota(&buf) == 100 - 16);
    assert(sc_buf_get_blob(&buf, 0) == NULL);

    sc_buf_move(&buf2, &buf);
    assert(sc_buf_get_64(&buf2) == UINT64_MAX);
    assert(sc_buf_size(&buf) == 0);

    char tmp[] = "testtesttesttesttesttesttesttesttesttesttesttesttetesttesttesttesttesttesttesttesttesttesttesttesttestteststtest";
    sc_buf_put_str(&buf, tmp);
    assert(strcmp(sc_buf_get_str(&buf), tmp) == 0);
    sc_buf_clear(&buf);
    sc_buf_put_8(&buf, 'x');
    sc_buf_put_8(&buf, 'y');
    assert(*(char*)sc_buf_at(&buf, 1) == 'y');
    sc_buf_clear(&buf);

    sc_buf_put_64(&buf, 444);
    sc_buf_put_64(&buf, 43);
    sc_buf_mark_read(&buf, 8);
    sc_buf_compact(&buf);
    assert(sc_buf_get_read_pos(&buf) == 0);
    assert(sc_buf_get_write_pos(&buf) == 8);
    assert(sc_buf_valid(&buf));
    sc_buf_term(&buf);

    sc_buf_init(&buf, 100);
    sc_buf_limit(&buf, 128);
    sc_buf_put_str(&buf, tmp);
    assert(sc_buf_valid(&buf) == false);
    assert(sc_buf_get_64(&buf) == 0);
    sc_buf_term(&buf);

    sc_buf_init(&buf, 100);
    sc_buf_limit(&buf, 128);
    sc_buf_put_fmt(&buf, tmp);
    assert(sc_buf_valid(&buf) == false);
    assert(sc_buf_get_64(&buf) == 0);
    sc_buf_term(&buf);
    sc_buf_term(&buf2);

    char text[128];
    buf = sc_buf_wrap(text, sizeof(text), true);
    sc_buf_put_text(&buf, "test %d test %s", 1, "test");
    assert(strcmp(sc_buf_read_buf(&buf), "test 1 test test") == 0);
}

int main()
{
    test1();
    return 0;
}

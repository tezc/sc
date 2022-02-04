#include "sc_buf.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

void test1()
{
	const char *xstr;
	struct sc_buf buf, buf2;

	sc_buf_init(&buf, 100);
	assert(buf.mem != NULL);
	sc_buf_term(&buf);

	sc_buf_init(&buf2, 0);
	assert(buf.mem == buf2.mem);
	assert(buf.limit == buf2.limit);
	assert(buf.wpos == buf2.wpos);
	assert(buf.rpos == buf2.rpos);
	assert(buf.ref == buf2.ref);
	assert(buf.cap == buf2.cap);
	assert(buf.err == buf2.err);
	sc_buf_term(&buf2);

	sc_buf_put_64(&buf, 100);
	assert(sc_buf_get_64(&buf) == 100);
	sc_buf_put_raw(&buf, NULL, 0);
	assert(sc_buf_size(&buf) == 0);
	sc_buf_term(&buf);

	sc_buf_init(&buf, 100);
	sc_buf_set_rpos(&buf, 1);
	assert(sc_buf_valid(&buf) == false);
	sc_buf_clear(&buf);
	assert(sc_buf_valid(&buf) == true);
	sc_buf_set_wpos(&buf, 101);
	assert(sc_buf_valid(&buf) == false);
	sc_buf_term(&buf);

	sc_buf_init(&buf, 100);
	sc_buf_put_32(&buf, 1000);
	assert(sc_buf_get_str(&buf) == NULL);
	assert(sc_buf_valid(&buf) == false);
	sc_buf_term(&buf);

	sc_buf_init(&buf, 100);
	sc_buf_init(&buf2, 100);
	sc_buf_put_str(&buf, "s");
	sc_buf_move(&buf2, &buf);
	assert(sc_buf_size(&buf) == 0);
	sc_buf_term(&buf);
	sc_buf_term(&buf2);

	sc_buf_init(&buf, 100);
	sc_buf_init(&buf2, 3);
	sc_buf_put_str(&buf, "s");
	sc_buf_move(&buf2, &buf);
	assert(sc_buf_size(&buf) != 0);
	sc_buf_term(&buf);
	sc_buf_term(&buf2);

	sc_buf_init(&buf2, 100);
	sc_buf_init(&buf, 100);

	assert(sc_buf_bool_len(true) == 1);
	assert(sc_buf_8_len(true) == 1);
	assert(sc_buf_16_len(true) == 2);
	assert(sc_buf_32_len(true) == 4);
	assert(sc_buf_64_len(true) == 8);
	assert(sc_buf_double_len(true) == 8);
	assert(sc_buf_str_len("test") == 13);
	assert(sc_buf_str_len(NULL) == 8);
	assert(sc_buf_blob_len("test", 4) == 12);

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
	assert(x == (double) 123211.323321);
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
	assert(strcmp("test", sc_buf_get_blob(&buf, sc_buf_get_64(&buf))) == 0);
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
	char *c = sc_buf_wbuf(&buf);
	*c = 'c';
	sc_buf_mark_write(&buf, 1);
	assert(sc_buf_get_8(&buf) == 'c');
	sc_buf_clear(&buf);

	sc_buf_clear(&buf);
	sc_buf_put_32(&buf, 2323);
	sc_buf_put_32(&buf, 3311);
	sc_buf_set_wpos(&buf, 8);
	sc_buf_set_rpos(&buf, 4);
	assert(sc_buf_get_32(&buf) == 3311);
	sc_buf_clear(&buf);
	sc_buf_put_64(&buf, UINT64_MAX);
	assert(sc_buf_get_64(&buf) == UINT64_MAX);
	sc_buf_put_64(&buf, UINT64_MAX);

	unsigned char *z = sc_buf_rbuf(&buf);
	assert(*z == 0xFF);
	assert(sc_buf_wpos(&buf) == 16);
	assert(sc_buf_quota(&buf) == 100 - 16);
	assert(sc_buf_get_blob(&buf, 0) == NULL);

	sc_buf_move(&buf2, &buf);
	assert(sc_buf_get_64(&buf2) == UINT64_MAX);
	assert(sc_buf_size(&buf) == 0);

	char tmp[] = "testtesttesttesttesttesttesttesttesttesttesttesttetesttes"
		     "ttesttesttesttesttesttesttesttesttesttesttestteststtest";
	sc_buf_put_str(&buf, tmp);
	assert(strcmp(sc_buf_get_str(&buf), tmp) == 0);
	sc_buf_clear(&buf);
	sc_buf_put_8(&buf, 'x');
	sc_buf_put_8(&buf, 'y');
	assert(*(char *) sc_buf_at(&buf, 1) == 'y');
	sc_buf_clear(&buf);

	sc_buf_put_64(&buf, 444);
	sc_buf_put_64(&buf, 43);
	sc_buf_mark_read(&buf, 8);
	sc_buf_compact(&buf);
	assert(sc_buf_rpos(&buf) == 0);
	assert(sc_buf_wpos(&buf) == 8);
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
	sc_buf_put_str(&buf, tmp);
	assert(sc_buf_valid(&buf) == false);
	assert(sc_buf_get_32(&buf) == 0);
	sc_buf_term(&buf);

	sc_buf_init(&buf, 100);
	sc_buf_limit(&buf, 128);
	sc_buf_put_str(&buf, tmp);
	assert(sc_buf_valid(&buf) == false);
	assert(sc_buf_get_16(&buf) == 0);
	sc_buf_term(&buf);

	sc_buf_init(&buf, 100);
	sc_buf_put_64(&buf, 1000);
	assert(sc_buf_valid(&buf) == true);
	assert(sc_buf_get_str(&buf) == 0);
	sc_buf_term(&buf);

	sc_buf_init(&buf, 100);
	sc_buf_limit(&buf, 128);
	sc_buf_put_str(&buf, tmp);
	assert(sc_buf_valid(&buf) == false);
	assert(sc_buf_get_8(&buf) == 0);
	sc_buf_term(&buf);

	sc_buf_init(&buf, 100);
	sc_buf_limit(&buf, 128);
	sc_buf_put_str(&buf, tmp);
	assert(sc_buf_valid(&buf) == false);
	assert(sc_buf_get_double(&buf) == 0);
	sc_buf_term(&buf);

	sc_buf_init(&buf, 100);
	sc_buf_limit(&buf, 128);
	sc_buf_put_fmt(&buf, tmp);
	assert(sc_buf_valid(&buf) == false);
	assert(sc_buf_get_64(&buf) == 0);
	sc_buf_term(&buf);
	sc_buf_term(&buf2);

	char text[128];
	buf = sc_buf_wrap(text, sizeof(text), SC_BUF_REF);
	sc_buf_put_text(&buf, "test %d test %s", 1, "test");
	assert(strcmp(sc_buf_rbuf(&buf), "test 1 test test") == 0);
	sc_buf_term(&buf);

	buf = sc_buf_wrap(text, sizeof(text), SC_BUF_REF);
	sc_buf_put_str(&buf, "1test");
	sc_buf_put_str(&buf, "2test");

	buf2 = sc_buf_wrap(sc_buf_rbuf(&buf), sc_buf_size(&buf), SC_BUF_READ);
	assert(sc_buf_size(&buf2) == sc_buf_size(&buf));
	assert(strcmp(sc_buf_get_str(&buf2), "1test") == 0);
	assert(strcmp(sc_buf_get_str(&buf2), "2test") == 0);
	sc_buf_term(&buf);
	sc_buf_term(&buf2);

	sc_buf_init(&buf, 1);
	for (int i = 0; i < 1000; i++) {
		sc_buf_put_fmt(&buf, "testtesttesttesttesttesttesttest%d", i);
	}

	for (int i = 0; i < 1000; i++) {
		char xtmp[128];
		snprintf(xtmp, sizeof(xtmp),
			 "testtesttesttesttesttesttesttest%d", i);

		xstr = sc_buf_get_str(&buf);
		assert(strcmp(xstr, xtmp) == 0);
	}

	assert(sc_buf_valid(&buf));
	sc_buf_term(&buf);

	sc_buf_init(&buf, 100);
	sc_buf_limit(&buf, 100000000);

	for (int i = 0; i < 1000000; i++) {
		sc_buf_put_str(&buf, "teessssssssssssssssssssssssssssssssssssss"
				     "sssss");
	}

	assert(sc_buf_valid(&buf) == false);
	sc_buf_term(&buf);
}

void test2()
{
	unsigned char tmp[32];
	struct sc_buf buf;
	sc_buf_init(&buf, 100);

	sc_buf_put_64(&buf, 100);
	sc_buf_put_64(&buf, 200);
	sc_buf_set_64_at(&buf, 8, 300);
	assert(sc_buf_peek_64_at(&buf, 8) == 300);
	sc_buf_set_32_at(&buf, 8, 111);
	assert(sc_buf_peek_32_at(&buf, 8) == 111);
	sc_buf_set_8_at(&buf, 0, 0);
	sc_buf_set_8_at(&buf, 1, 0);
	sc_buf_set_8_at(&buf, 2, 0);
	sc_buf_set_8_at(&buf, 3, 0);
	assert(sc_buf_peek_32_at(&buf, 0) == 0);

	sc_buf_clear(&buf);
	sc_buf_put_64(&buf, 1000);
	sc_buf_put_64(&buf, 3000);
	sc_buf_set_wpos(&buf, 0);
	sc_buf_set_64(&buf, 2000);
	sc_buf_set_wpos(&buf, 16);
	assert(sc_buf_peek_64(&buf) == 2000);
	assert(sc_buf_peek_64_at(&buf, 8) == 3000);

	sc_buf_clear(&buf);
	sc_buf_put_32(&buf, 10);
	assert(sc_buf_peek_32(&buf) == 10);
	sc_buf_set_wpos(&buf, 0);
	sc_buf_set_32(&buf, 1000);
	sc_buf_set_wpos(&buf, 4);
	assert(sc_buf_peek_32(&buf) == 1000);

	sc_buf_clear(&buf);
	sc_buf_put_text(&buf, "test");
	sc_buf_put_text(&buf, "test");
	assert(strcmp(sc_buf_rbuf(&buf), "testtest") == 0);

	sc_buf_clear(&buf);
	sc_buf_put_32(&buf, 100);
	sc_buf_put_32(&buf, 100);
	sc_buf_put_32(&buf, 100);
	sc_buf_put_32(&buf, 100);
	assert(sc_buf_peek_32_at(&buf, 100) == 0);
	assert(sc_buf_valid(&buf) == false);

	sc_buf_term(&buf);

	sc_buf_init(&buf, 1000);
	sc_buf_mark_write(&buf, 1000);
	sc_buf_mark_read(&buf, 800);
	sc_buf_reserve(&buf, 400);
	sc_buf_term(&buf);

	sc_buf_init(&buf, 100);
	sc_buf_get_str(&buf);
	sc_buf_set_8(&buf, 100);
	sc_buf_set_16(&buf, 100);
	sc_buf_set_32(&buf, 100);
	sc_buf_set_64(&buf, 100);
	sc_buf_set_data(&buf, 19, "d", 1);
	sc_buf_peek_data(&buf, 10, tmp, 0);
	assert(!sc_buf_valid(&buf));
	sc_buf_term(&buf);

	sc_buf_init(&buf, 32);
	sc_buf_put_64(&buf, 100);
	sc_buf_put_64(&buf, 200);
	sc_buf_put_64(&buf, 300);
	sc_buf_put_64(&buf, 400);
	sc_buf_get_64(&buf);
	sc_buf_get_64(&buf);
	sc_buf_shrink(&buf, 24);
	assert(sc_buf_get_64(&buf) == 300);
	assert(sc_buf_get_64(&buf) == 400);
	assert(sc_buf_size(&buf) == 0);
	sc_buf_term(&buf);

	sc_buf_init(&buf, 4096);
	sc_buf_shrink(&buf, 4096 * 2);
	sc_buf_shrink(&buf, 128);

	for (int i = 0; i < 4000; i++) {
		sc_buf_put_64(&buf, i);
	}

	sc_buf_shrink(&buf, 0);

	for (int i = 0; i < 3700; i++) {
		sc_buf_get_64(&buf);
	}

	sc_buf_shrink(&buf, 4096);

	for (int i = 0; i < 300; i++) {
		assert(sc_buf_get_64(&buf) == (uint64_t) 3700 + i);
	}

	sc_buf_term(&buf);
}

#ifdef SC_HAVE_WRAP

bool fail_calloc = false;
void *__real_calloc(size_t m, size_t n);
void *__wrap_calloc(size_t m, size_t n)
{
	if (fail_calloc) {
		return NULL;
	}

	return __real_calloc(m, n);
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

bool mock_strlen = false;
extern size_t __real_strlen(const char *s);
size_t __wrap_strlen(const char *s)
{
	if (mock_strlen) {
		return SIZE_MAX;
	}

	return __real_strlen(s);
}

bool fail_vsnprintf;
int fail_vsnprintf_value = -1;
int fail_vsnprintf_at = -1;
extern int __real_vsnprintf(char *str, size_t size, const char *format,
			    va_list ap);
int __wrap_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
	fail_vsnprintf_at--;
	if (!fail_vsnprintf && (fail_vsnprintf_at) != 0) {
		return __real_vsnprintf(str, size, format, ap);
	}

	return fail_vsnprintf_value;
}

void fail_test()
{
	int tmp;
	unsigned char *p;
	struct sc_buf buf;

	fail_calloc = true;
	assert(sc_buf_init(&buf, 100) == false);
	fail_calloc = false;

	assert(sc_buf_init(&buf, 0) == true);
	sc_buf_put_32(&buf, 100);
	assert(sc_buf_valid(&buf) == true);
	sc_buf_term(&buf);

	fail_calloc = true;
	fail_realloc = true;
	assert(sc_buf_init(&buf, 0) == true);
	sc_buf_put_32(&buf, 100);
	assert(sc_buf_valid(&buf) == false);
	sc_buf_term(&buf);
	fail_calloc = false;
	fail_realloc = false;

	sc_buf_init(&buf, 10);
	sc_buf_set_data(&buf, 20, "", 1);
	assert(sc_buf_valid(&buf) == false);
	sc_buf_term(&buf);

	sc_buf_init(&buf, 10);
	fail_vsnprintf = true;
	sc_buf_put_fmt(&buf, "test");
	assert(sc_buf_valid(&buf) == false);
	fail_vsnprintf = false;
	sc_buf_term(&buf);

	sc_buf_init(&buf, 3);
	fail_vsnprintf_at = 2;
	sc_buf_put_fmt(&buf, "test");
	assert(sc_buf_valid(&buf) == false);
	sc_buf_term(&buf);

	sc_buf_init(&buf, 100);
	mock_strlen = true;
	sc_buf_put_str(&buf, "t");
	mock_strlen = false;
	assert(sc_buf_valid(&buf) == false);
	sc_buf_term(&buf);

	sc_buf_init(&buf, 10);
	fail_vsnprintf = true;
	sc_buf_put_text(&buf, "test");
	assert(sc_buf_valid(&buf) == false);
	fail_vsnprintf = false;
	sc_buf_term(&buf);

	sc_buf_init(&buf, 3);
	fail_vsnprintf_at = 2;
	sc_buf_put_text(&buf, "test");
	assert(sc_buf_valid(&buf) == false);
	sc_buf_term(&buf);

	sc_buf_init(&buf, 3);
	fail_vsnprintf_at = 2;
	fail_realloc = true;
	sc_buf_put_text(&buf, "test");
	assert(sc_buf_valid(&buf) == false);
	sc_buf_term(&buf);

	sc_buf_init(&buf, 0);
	sc_buf_peek_8(&buf);
	assert(sc_buf_valid(&buf) == false);
	sc_buf_clear(&buf);

	sc_buf_peek_16(&buf);
	assert(sc_buf_valid(&buf) == false);
	sc_buf_clear(&buf);

	sc_buf_peek_32(&buf);
	assert(sc_buf_valid(&buf) == false);
	sc_buf_clear(&buf);

	sc_buf_peek_64(&buf);
	assert(sc_buf_valid(&buf) == false);
	sc_buf_clear(&buf);

	sc_buf_set_8(&buf, 10);
	assert(sc_buf_valid(&buf) == false);
	sc_buf_clear(&buf);

	sc_buf_set_16(&buf, 10);
	assert(sc_buf_valid(&buf) == false);
	sc_buf_clear(&buf);

	sc_buf_set_32(&buf, 10);
	assert(sc_buf_valid(&buf) == false);
	sc_buf_clear(&buf);

	sc_buf_set_64(&buf, 10);
	assert(sc_buf_valid(&buf) == false);
	sc_buf_clear(&buf);

	sc_buf_get_data(&buf, &tmp, sizeof(tmp));
	assert(sc_buf_valid(&buf) == false);

	fail_realloc = false;
	sc_buf_clear(&buf);
	sc_buf_put_32(&buf, 10);
	sc_buf_get_data(&buf, &tmp, sizeof(tmp));
	assert(sc_buf_valid(&buf) == true);

	sc_buf_term(&buf);
	sc_buf_init(&buf, 0);
	fail_realloc = true;
	sc_buf_put_64(&buf, 0);
	assert(sc_buf_valid(&buf) == false);
	fail_realloc = false;
	sc_buf_clear(&buf);

	sc_buf_put_str_len(&buf, "tesstt", 6);
	assert(strcmp(sc_buf_get_str(&buf), "tesstt") == 0);
	sc_buf_term(&buf);
	sc_buf_init(&buf, 0);
	fail_realloc = true;
	sc_buf_put_str_len(&buf, "tesstt", 6);
	assert(sc_buf_valid(&buf) == false);
	fail_realloc = false;

	sc_buf_term(&buf);
	sc_buf_init(&buf, 0);
	sc_buf_put_str_len(&buf, NULL, 100);
	assert(sc_buf_get_str(&buf) == NULL);
	assert(sc_buf_valid(&buf) == true);

	sc_buf_put_str_len(&buf, "", 0);
	assert(strcmp(sc_buf_get_str(&buf), "") == 0);
	assert(sc_buf_valid(&buf) == true);

	sc_buf_term(&buf);
	sc_buf_init(&buf, 0);
	fail_realloc = true;
	sc_buf_put_16(&buf, 10);
	assert(sc_buf_valid(&buf) == false);

	sc_buf_term(&buf);
	sc_buf_init(&buf, 0);
	sc_buf_get_blob(&buf, 100);
	assert(sc_buf_valid(&buf) == false);

	sc_buf_term(&buf);
	sc_buf_init(&buf, 0);
	sc_buf_peek_data(&buf, 100, (unsigned char *) &p, sizeof(p));
	assert(sc_buf_valid(&buf) == false);

	sc_buf_term(&buf);

	fail_realloc = false;
	sc_buf_init(&buf, 0);
	sc_buf_reserve(&buf, 100);
	assert(sc_buf_quota(&buf) > 100);
	sc_buf_term(&buf);

	buf = sc_buf_wrap(&p, 8, SC_BUF_REF);
	assert(sc_buf_reserve(&buf, 100) == false);

	fail_vsnprintf_at = -1;
	fail_realloc = false;

	sc_buf_init(&buf, 3);
	fail_vsnprintf_at = 2;
	fail_vsnprintf_value = 1000000;
	sc_buf_put_text(&buf, "test");
	assert(sc_buf_valid(&buf) == false);
	sc_buf_term(&buf);

	sc_buf_init(&buf, 3);
	fail_vsnprintf_at = 2;
	fail_vsnprintf_value = -1;
	sc_buf_put_text(&buf, "test");
	assert(sc_buf_valid(&buf) == false);
	sc_buf_term(&buf);

	sc_buf_init(&buf, 3);
	fail_vsnprintf_at = 2;
	fail_vsnprintf_value = 1000000;
	sc_buf_put_fmt(&buf, "%s", "test");
	assert(sc_buf_valid(&buf) == false);
	sc_buf_term(&buf);

	fail_vsnprintf_value = -1;
	fail_vsnprintf_at = -1;
	fail_realloc = false;

	sc_buf_init(&buf, 3);
	sc_buf_put_text(&buf, "test");
	assert(sc_buf_valid(&buf) == true);
	sc_buf_term(&buf);

	sc_buf_init(&buf, 4096 * 8);
	fail_realloc = true;
	assert(sc_buf_shrink(&buf, 4096) == false);
	fail_realloc = false;
	assert(sc_buf_shrink(&buf, 4096) == true);
	assert(sc_buf_cap(&buf) == 4096);
	sc_buf_term(&buf);
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
	fail_test();
	return 0;
}

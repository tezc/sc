

#include "sc.h"

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void test1()
{
	int64_t x;
	char *t;
	char buf[32];

	const int64_t kb = (int64_t) 1024;
	const int64_t mb = (int64_t) 1024 * 1024;
	const int64_t gb = (int64_t) 1024 * 1024 * 1024;
	const int64_t tb = (int64_t) 1024 * 1024 * 1024 * 1024;
	const int64_t pb = (int64_t) 1024 * 1024 * 1024 * 1024 * 1024;
	const int64_t eb = (int64_t) 1024 * 1024 * 1024 * 1024 * 1024 * 1024;

	assert(sc_min(199, 299) == 199);
	assert(sc_max(199, 299) == 299);
	assert(sc_is_pow2(0) == false);
	assert(sc_is_pow2(1) == true);
	assert(sc_is_pow2(3) == false);

	x = sc_to_pow2(0);
	assert(x == 1);
	assert(sc_is_pow2(x) == true);
	x = sc_to_pow2(1);
	assert(x == 1);
	assert(sc_is_pow2(x) == true);
	x = sc_to_pow2(1023);
	assert(x == 1024);
	assert(sc_is_pow2(x) == true);

	x = sc_size_to_bytes("1");
	assert(x == 1);
	x = sc_size_to_bytes("313");
	assert(x == 313);
	x = sc_size_to_bytes("1b");
	assert(x == 1);

	x = sc_size_to_bytes("1k");
	assert(x == kb);
	x = sc_size_to_bytes("1kb");
	assert(x == kb);

	x = sc_size_to_bytes("1m");
	assert(x == mb);
	x = sc_size_to_bytes("1mb");
	assert(x == mb);

	x = sc_size_to_bytes("1g");
	assert(x == gb);
	x = sc_size_to_bytes("1gb");
	assert(x == gb);

	x = sc_size_to_bytes("1t");
	assert(x == tb);
	x = sc_size_to_bytes("1tb");
	assert(x == tb);

	x = sc_size_to_bytes("1e");
	assert(x == eb);
	x = sc_size_to_bytes("1eb");
	assert(x == eb);

	x = sc_size_to_bytes("1gx");
	assert(x == -1);

	x = sc_size_to_bytes("gx");
	assert(x == -1);

	x = sc_size_to_bytes("1xgx");
	assert(x == -1);

	x = sc_size_to_bytes("1xb");
	assert(x == -1);

	x = sc_size_to_bytes("1p");
	assert(x == pb);
	x = sc_size_to_bytes("1pb");
	assert(x == pb);

	x = sc_size_to_bytes("22eb");
	assert(x == -1);
	x = sc_size_to_bytes("31024pb");
	assert(x == -1);
	x = sc_size_to_bytes("31024111tb");
	assert(x == -1);
	x = sc_size_to_bytes("31024111111gb");
	assert(x == -1);
	x = sc_size_to_bytes("31024111111111mb");
	assert(x == -1);
	x = sc_size_to_bytes("31024111111111111kb");
	assert(x == -1);

	t = sc_bytes_to_size(buf, sizeof(buf), 313);
	assert(strcmp(t, "313 B") == 0);

	t = sc_bytes_to_size(buf, sizeof(buf), 1024);
	assert(strcmp(t, "1.00 KB") == 0);

	t = sc_bytes_to_size(buf, sizeof(buf), 2 * 1024);
	assert(strcmp(t, "2.00 KB") == 0);

	t = sc_bytes_to_size(buf, sizeof(buf), 2 * 1024 * 1024);
	assert(strcmp(t, "2.00 MB") == 0);

	t = sc_bytes_to_size(buf, sizeof(buf),
			     (uint64_t) 2 * 1024 * 1024 * 1024);
	assert(strcmp(t, "2.00 GB") == 0);

	t = sc_bytes_to_size(buf, sizeof(buf),
			     (uint64_t) 2 * 1024 * 1024 * 1024 * 1024);
	assert(strcmp(t, "2.00 TB") == 0);

	t = sc_bytes_to_size(buf, sizeof(buf),
			     (uint64_t) 2 * 1024 * 1024 * 1024 * 1024 * 1024);
	assert(strcmp(t, "2.00 PB") == 0);

	t = sc_bytes_to_size(buf, sizeof(buf), UINT64_MAX);
	assert(strcmp(t, "16.00 EB") == 0);
}

void test_rand()
{
	unsigned char tmp[256] = {0, 1, 2, 3, 4, 5, 6, 6,
				  6, 6, 1, 5, 3, 5, 5, 6};
	unsigned char tmp2[256] = {0, 1, 2, 3, 4, 3, 6, 6,
				   6, 6, 1, 2, 3, 5, 5, 6};

	unsigned char out1[256];
	unsigned char out2[256];

	struct sc_rand rc4_1;
	struct sc_rand rc4_2;

	sc_rand_init(&rc4_1, tmp);
	sc_rand_init(&rc4_2, tmp);

	sc_rand_read(&rc4_1, out1, sizeof(out1));
	sc_rand_read(&rc4_2, out2, sizeof(out2));

	assert(memcmp(out1, out2, sizeof(out1)) == 0);

	sc_rand_init(&rc4_1, tmp);
	sc_rand_init(&rc4_2, tmp2);

	sc_rand_read(&rc4_1, out1, sizeof(out1));
	sc_rand_read(&rc4_2, out2, sizeof(out2));

	assert(memcmp(out1, out2, sizeof(out1)) != 0);

	sc_rand_read(&rc4_1, out1, 0);
	sc_rand_read(&rc4_1, NULL, 0);
	sc_rand_read(&rc4_1, NULL, 10);
}

#ifdef SC_HAVE_WRAP

#include <errno.h>

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

int fail_strtoll;
int fail_strtoll_errno;

extern long long int __real_strtoll(const char *__restrict nptr,
				    char **__restrict endptr, int base);
extern long long int __wrap_strtoll(const char *__restrict nptr,
				    char **__restrict endptr, int base)
{
	if (fail_strtoll) {
		errno = fail_strtoll_errno;
		*endptr = (char *) nptr;
		return 0;
	}

	return __real_strtoll(nptr, endptr, base);
}

void fail_test()
{
	char *t;
	int64_t val;
	char buf[32];

	fail_snprintf = -1;
	t = sc_bytes_to_size(buf, sizeof(buf), 2 * 1024);
	assert(t == NULL);
	fail_snprintf = 0;

	fail_snprintf = -1;
	t = sc_bytes_to_size(buf, sizeof(buf), 313);
	assert(t == NULL);
	fail_snprintf = 0;

	fail_snprintf = 10000;
	t = sc_bytes_to_size(buf, sizeof(buf), 2 * 1024);
	assert(t == NULL);
	fail_snprintf = 0;

	fail_snprintf = 10000;
	t = sc_bytes_to_size(buf, sizeof(buf), 313);
	assert(t == NULL);
	fail_snprintf = 0;

	fail_strtoll = 1;
	val = sc_size_to_bytes("10kb");
	assert(val == -1);
	fail_strtoll = 0;

	fail_strtoll = 1;
	fail_strtoll_errno = 100;
	val = sc_size_to_bytes("10kb");
	assert(val == -1);
	fail_strtoll = 0;
}

#else
void fail_test()
{
}
#endif
int main()
{
	test1();
	test_rand();
	fail_test();

	return 0;
}

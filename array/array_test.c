#include "sc_array.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

void example_str()
{
	const char *it;
	struct sc_array_str arr;

	sc_array_init(&arr);

	sc_array_add(&arr, "item0");
	sc_array_add(&arr, "item1");
	sc_array_add(&arr, "item2");

	printf("\nDelete first element \n\n");
	sc_array_del(&arr, 0);

	sc_array_foreach (&arr, it) {
		printf("Elem = %s \n", it);
	}

	sc_array_term(&arr);
}

void example_int()
{
	struct sc_array_int arr;

	sc_array_init(&arr);

	sc_array_add(&arr, 0);
	sc_array_add(&arr, 1);
	sc_array_add(&arr, 2);

	for (size_t i = 0; i < sc_array_size(&arr); i++) {
		printf("Elem = %d \n", arr.elems[i]);
	}

	sc_array_term(&arr);
}

static int compare(const void *a, const void *b)
{
	const int *x = a;
	const int *y = b;

	return *x - *y;
}

static void test1(void)
{

	int total = 0;
	struct sc_array_int arr;

	sc_array_init(&arr);
	sc_array_term(&arr);
	assert(sc_array_size(&arr) == 0);
	sc_array_add(&arr, 1);
	sc_array_add(&arr, 2);
	sc_array_add(&arr, 3);
	assert(arr.elems[0] == 1);
	assert(arr.elems[1] == 2);
	assert(arr.elems[2] == 3);
	sc_array_term(&arr);

	sc_array_init(&arr);
	sc_array_add(&arr, 3);
	sc_array_add(&arr, 4);
	sc_array_add(&arr, 5);

	assert(sc_array_size(&arr) == 3);

	sc_array_del(&arr, 0);
	assert(arr.elems[0] == 4);
	sc_array_del_last(&arr);
	assert(arr.elems[0] == 4);

	sc_array_add(&arr, 1);
	sc_array_add(&arr, 3);
	sc_array_add(&arr, 2);
	sc_array_add(&arr, 0);

	assert(sc_array_last(&arr) == 0);

	sc_array_sort(&arr, compare);

	for (size_t i = 0; i < sc_array_size(&arr); i++) {
		total += arr.elems[i];
	}

	assert(total == 10);

	for (size_t i = 0; i < sc_array_size(&arr); i++) {
		assert(arr.elems[i] == (int) i);
	}

	sc_array_term(&arr);
}

void test2()
{
	int val;
	struct sc_array_int arr;

	sc_array_init(&arr);

	sc_array_foreach (&arr, val) {
		assert(true);
	}
	sc_array_term(&arr);

	sc_array_init(&arr);

	sc_array_foreach (&arr, val) {
		assert(true);
	}
	sc_array_term(&arr);

	sc_array_init(&arr);

	sc_array_add(&arr, 1);
	assert(!sc_array_oom(&arr));

	sc_array_foreach (&arr, val) {
		assert(val == 1);
	}
	sc_array_del_last(&arr);
	sc_array_foreach (&arr, val) {
		assert(true);
	}

	sc_array_add(&arr, 1);
	assert(!sc_array_oom(&arr));

	sc_array_del_unordered(&arr, 0);

	sc_array_foreach (&arr, val) {
		assert(true);
	}

	sc_array_term(&arr);

	sc_array_init(&arr);
	sc_array_add(&arr, 100);
	sc_array_add(&arr, 200);
	sc_array_add(&arr, 300);
	assert(sc_array_at(&arr, 0) == 100);
	sc_array_del_last(&arr);
	assert(sc_array_at(&arr, 0) == 100);
	sc_array_del(&arr, 0);
	assert(sc_array_at(&arr, 0) == 200);
	sc_array_term(&arr);
}

void bounds_test()
{
	int total = 0;
	int val;
	struct sc_array_int arr;

	sc_array_init(&arr);
	sc_array_add(&arr, 3);
	sc_array_add(&arr, 4);

	sc_array_foreach (&arr, val) {
		total += val;
	}

	assert(total == 7);

	sc_array_term(&arr);

	total = 0;

	sc_array_init(&arr);
	sc_array_foreach (&arr, val) {
		total += val;
	}

	sc_array_foreach (&arr, val) {
		total += val;
	}

	assert(total == 0);

	sc_array_term(&arr);

	sc_array_init(&arr);
	sc_array_add(&arr, 0);
	sc_array_add(&arr, 1);
	sc_array_add(&arr, 2);
	sc_array_add(&arr, 4);
	sc_array_add(&arr, 3);

	sc_array_del(&arr, 3);
	for (size_t i = 0; i < sc_array_size(&arr); i++) {
		assert((int) i == arr.elems[i]);
	}

	sc_array_add(&arr, 3);
	sc_array_add(&arr, 4);

	sc_array_del(&arr, 3);
	for (size_t i = 0; i < sc_array_size(&arr); i++) {
		assert((int) i == arr.elems[i]);
	}

	sc_array_term(&arr);
}

#ifdef SC_HAVE_WRAP

bool fail_realloc = false;
void *__real_realloc(void *p, size_t size);
void *__wrap_realloc(void *p, size_t n)
{
	if (fail_realloc) {
		return NULL;
	}

	return __real_realloc(p, n);
}

void fail_test()
{
	int tmp;
	int total = 0;
	struct sc_array_int arr;

	sc_array_init(&arr);

	sc_array_add(&arr, 0);
	assert(!sc_array_oom(&arr));
	sc_array_term(&arr);

	sc_array_init(&arr);
	assert(sc_array_size(&arr) == 0);

	sc_array_foreach (&arr, tmp) {
		assert(false);
	}

	size_t count = SC_ARRAY_MAX / sizeof(int);

	for (size_t i = 0; i < count + 5; i++) {
		sc_array_add(&arr, i);
	}

	assert(sc_array_oom(&arr));
	sc_array_del(&arr, 0);
	sc_array_add(&arr, 400);
	assert(sc_array_oom(&arr) == false);

	sc_array_term(&arr);

	sc_array_init(&arr);
	assert(sc_array_size(&arr) == 0);

	fail_realloc = true;
	sc_array_add(&arr, 0);
	assert(sc_array_oom(&arr));

	fail_realloc = false;
	sc_array_add(&arr, 222);
	assert(!sc_array_oom(&arr));
	sc_array_term(&arr);

	fail_realloc = true;
	sc_array_init(&arr);
	sc_array_add(&arr, 3);
	assert(sc_array_oom(&arr));
	fail_realloc = false;
	sc_array_add(&arr, 3);
	assert(sc_array_size(&arr) == 1);
	assert(sc_array_oom(&arr) == false);
	sc_array_term(&arr);

	sc_array_init(&arr);
	fail_realloc = true;
	sc_array_add(&arr, 222);
	assert(sc_array_oom(&arr));
	fail_realloc = false;

	sc_array_add(&arr, 3);
	sc_array_add(&arr, 4);
	sc_array_add(&arr, 5);

	assert(sc_array_size(&arr) == 3);

	sc_array_del(&arr, 0);
	assert(arr.elems[0] == 4);
	sc_array_del_last(&arr);
	assert(arr.elems[0] == 4);

	sc_array_add(&arr, 1);
	sc_array_add(&arr, 3);
	sc_array_add(&arr, 2);
	sc_array_add(&arr, 0);

	sc_array_sort(&arr, compare);

	for (size_t i = 0; i < sc_array_size(&arr); i++) {
		total += arr.elems[i];
	}

	assert(total == 10);

	for (size_t i = 0; i < sc_array_size(&arr); i++) {
		assert(arr.elems[i] == (int) i);
	}

	total = 0;
	sc_array_foreach (&arr, tmp) {
		total += tmp;
	}
	assert(total == 10);

	sc_array_sort(&arr, compare);
	sc_array_del_unordered(&arr, 0);
	assert(arr.elems[0] == 4);
	assert(sc_array_size(&arr) == 4);
	sc_array_clear(&arr);
	assert(sc_array_size(&arr) == 0);
	sc_array_add(&arr, 10);
	assert(sc_array_size(&arr) == 1);
	assert(arr.elems[0] == 10);

	sc_array_term(&arr);
}

#else
void fail_test(void)
{
}
#endif

int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;

	example_str();
	example_int();
	test1();
	test2();
	fail_test();
	bounds_test();

	return 0;
}

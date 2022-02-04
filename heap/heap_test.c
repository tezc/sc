#include "sc_heap.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int example(void)
{
	struct data {
		int priority;
		char *data;
	};

	struct data n[] = {{1, "first"},
			   {4, "fourth"},
			   {5, "fifth"},
			   {3, "third"},
			   {2, "second"}};

	struct sc_heap_data *elem;
	struct sc_heap heap;

	sc_heap_init(&heap, 0);

	// Min-heap usage
	for (int i = 0; i < 5; i++) {
		sc_heap_add(&heap, n[i].priority, n[i].data);
	}

	while ((elem = sc_heap_pop(&heap)) != NULL) {
		printf("key = %d, data = %s \n",
		       (int) elem->key, (char*) elem->data);
	}
	printf("---------------- \n");

	// Max-heap usage, negate when adding into heap and negate back after
	// pop :
	for (int i = 0; i < 5; i++) {
		sc_heap_add(&heap, -(n[i].priority), n[i].data);
	}

	while ((elem = sc_heap_pop(&heap)) != NULL) {
		printf("key = %d, data = %s \n",
		       (int) elem->key, (char*) elem->data);
	}

	sc_heap_term(&heap);

	return 0;
}

void test1(void)
{
	struct sc_heap heap;
	struct sc_heap_data *elem;

	sc_heap_init(&heap, 0);
	sc_heap_add(&heap, 100, "test");
	elem = sc_heap_pop(&heap);
	assert(elem->key == 100);
	assert(strcmp("test", elem->data) == 0);
	sc_heap_term(&heap);

	sc_heap_add(&heap, 100, "test");
	elem = sc_heap_pop(&heap);
	assert(elem->key == 100);
	assert(strcmp("test", elem->data) == 0);
	sc_heap_term(&heap);

	assert(sc_heap_init(&heap, SIZE_MAX / 2) == false);
	assert(sc_heap_init(&heap, 3) == true);

	for (int i = 0; i < 1000; i++) {
		assert(sc_heap_add(&heap, i, (void *) (uintptr_t) i) == true);
		elem = sc_heap_pop(&heap);
		assert(elem != NULL);
		assert(elem->key == (intptr_t) elem->data);
	}

	int64_t arr[] = {1, 0, 4, 5, 7, 9, 8, 6, 3, 2};

	for (int i = 0; i < 10; i++) {
		assert(sc_heap_add(&heap, arr[i],
				   (void *) (uintptr_t) (arr[i] * 2)) == true);
	}

	for (int i = 0; i < 10; i++) {
		elem = sc_heap_pop(&heap);
		assert(elem != NULL);
		assert(elem->key == i);
		assert((intptr_t) elem->data == i * 2);
	}

	sc_heap_term(&heap);
}

void test2(void)
{
	static const int64_t arr[] = {1, 0, 4, 5, 7, 9, 8, 6, 3, 2};
	struct sc_heap_data *elem;
	struct sc_heap heap;

	assert(sc_heap_init(&heap, 0) == true);

	for (int i = 0; i < 1000; i++) {
		assert(sc_heap_add(&heap, i, (void *) (uintptr_t) i) == true);
		elem = sc_heap_pop(&heap);
		assert(elem != NULL);
		assert(elem->key == (intptr_t) elem->data);
		assert(elem->key == (intptr_t) i);
	}

	for (int i = 0; i < 10; i++) {
		assert(sc_heap_add(&heap, -arr[i],
				   (void *) (uintptr_t) (arr[i] * 2)) == true);
	}

	for (int i = 0; i < 10; i++) {
		elem = sc_heap_pop(&heap);
		assert(elem != NULL);
		assert(-elem->key == 9 - i);
		assert((intptr_t) elem->data == (9 - i) * 2);
	}

	sc_heap_term(&heap);
}

void test3(void)
{
	int arr[100];
	struct sc_heap_data *elem;
	struct sc_heap heap;

	assert(sc_heap_init(&heap, 2) == true);
	assert(sc_heap_add(&heap, 9, (void *) (uintptr_t) 9) == true);

	elem = sc_heap_peek(&heap);
	assert(elem != NULL);
	assert(elem->key == 9);
	assert(elem->data == (void *) (uintptr_t) 9);

	elem = sc_heap_pop(&heap);
	assert(elem != NULL);
	assert(elem->key == 9);
	assert(elem->data == (void *) (uintptr_t) 9);

	for (int i = 0; i < 100; i++) {
		arr[i] = i;
	}

	for (int i = 0; i < 100; i++) {
		int k = arr[i];
		arr[i] = arr[(i * 15) % 100];
		arr[(i * 15) % 100] = k;
	}

	for (int i = 0; i < 100; i++) {
		assert(sc_heap_add(&heap, i, (void *) (uintptr_t) i) == true);
	}

	for (int i = 0; i < 100; i++) {
		elem = sc_heap_pop(&heap);
		assert(elem != NULL);
		assert(elem->key == i);
		assert(elem->data == (void *) (uintptr_t) i);
	}

	assert(sc_heap_peek(&heap) == NULL);
	assert(sc_heap_pop(&heap) == NULL);

	assert(sc_heap_size(&heap) == 0);
	assert(sc_heap_add(&heap, 1, NULL) == true);
	assert(sc_heap_size(&heap) == 1);
	sc_heap_clear(&heap);
	assert(sc_heap_size(&heap) == 0);
	assert(sc_heap_add(&heap, 1, NULL) == true);
	assert(sc_heap_size(&heap) == 1);

	sc_heap_term(&heap);
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

bool fail_realloc = false;
void *__real_realloc(void *p, size_t size);
void *__wrap_realloc(void *p, size_t n)
{
	if (fail_realloc) {
		return NULL;
	}

	return __real_realloc(p, n);
}

void fail_test(void)
{
	struct sc_heap heap;

	assert(sc_heap_init(&heap, 2) == true);

	size_t count = SC_HEAP_MAX;
	bool success = false;

	for (size_t i = 0; i < count + 5; i++) {
		success = sc_heap_add(&heap, i, (void *) (uintptr_t) i);
	}

	assert(!success);

	sc_heap_term(&heap);

	assert(sc_heap_init(&heap, 0) == true);

	fail_realloc = true;
	assert(sc_heap_add(&heap, 1, NULL) == false);
	fail_realloc = false;

	sc_heap_term(&heap);

	fail_malloc = true;
	assert(sc_heap_init(&heap, 1) == false);

	fail_malloc = false;
	assert(sc_heap_init(&heap, 1) == true);

	sc_heap_term(&heap);
}
#else
void fail_test()
{
}
#endif

int main()
{
	example();
	fail_test();
	test1();
	test2();
	test3();

	return 0;
}

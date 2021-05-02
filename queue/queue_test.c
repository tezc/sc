#include "sc_queue.h"

#include <stddef.h>
#include <stdio.h>

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

void fail_test(void)
{
	struct sc_queue_int q;

	fail_calloc = true;
	sc_queue_init(&q);
	assert(sc_queue_oom(&q) == true);
	fail_calloc = false;
	sc_queue_init(&q);
	assert(sc_queue_oom(&q) == false);
	fail_calloc = true;
	for (int i = 0; i < 10; i++) {
		sc_queue_add_last(&q, 3);
	}

	assert(sc_queue_oom(&q) == true);
	fail_calloc = false;
	sc_queue_add_last(&q, 3);
	assert(sc_queue_oom(&q) == false);
	sc_queue_term(&q);

	sc_queue_init(&q);
	fail_calloc = true;
	for (int i = 0; i < 10; i++) {
		sc_queue_add_first(&q, 3);
	}
	assert(sc_queue_oom(&q) == true);
	fail_calloc = false;
	sc_queue_add_first(&q, 3);
	assert(sc_queue_oom(&q) == false);
	sc_queue_term(&q);

	sc_queue_init(&q);

	fail_calloc = true;
	for (int i = 0; i < 8; i++) {
		sc_queue_add_last(&q, i);
		if (sc_queue_oom(&q)) {
			break;
		}
	}

	assert(sc_queue_oom(&q));
	assert(sc_queue_size(&q) == 7);
	fail_calloc = false;
	sc_queue_add_last(&q, 7);
	assert(sc_queue_oom(&q) == false);

	for (int i = 0; i < 8; i++) {
		assert(sc_queue_del_first(&q) == i);
	}
	assert(sc_queue_size(&q) == 0);

	fail_calloc = false;

	size_t max = SC_QUEUE_MAX;
	for (size_t i = 0; i < max + 500; i++) {
		sc_queue_add_last(&q, i);
		if (sc_queue_oom(&q)) {
			break;
		}
	}

	assert(sc_queue_oom(&q));
	fail_calloc = false;
	sc_queue_add_last(&q, 100);
	assert(sc_queue_oom(&q));
	sc_queue_clear(&q);
	assert(sc_queue_size(&q) == 0);
	sc_queue_term(&q);
	sc_queue_init(&q);
	sc_queue_term(&q);
	fail_calloc = false;
}
#else
void fail_test(void)
{
}

#endif

void example(void)
{
	int elem;
	struct sc_queue_int queue;

	sc_queue_init(&queue);

	sc_queue_add_last(&queue, 2);
	sc_queue_add_last(&queue, 3);
	sc_queue_add_last(&queue, 4);
	sc_queue_add_first(&queue, 1);

	sc_queue_foreach (&queue, elem) {
		printf("elem = [%d] \n", elem);
	}

	elem = sc_queue_del_last(&queue);
	printf("Last element was : [%d] \n", elem);

	elem = sc_queue_del_first(&queue);
	printf("First element was : [%d] \n", elem);

	sc_queue_term(&queue);
}

void test0()
{
	uint32_t val;
	struct sc_queue_32 q;

	sc_queue_init(&q);

	for (uint32_t i = 0; i < 10000; i++) {
		sc_queue_add_last(&q, i);
	}

	for (uint32_t i = 0; i < 10000; i++) {
		assert(sc_queue_at(&q, i) == i);
	}

	assert(sc_queue_peek_first(&q) == 0);
	assert(sc_queue_peek_last(&q) == 9999);

	uint32_t j = 0;
	sc_queue_foreach (&q, val) {
		assert(val == j++);
	}


	j = 10000;
	while (j-- > 0) {
		assert(sc_queue_del_last(&q) == j);
	}

	assert(sc_queue_size(&q) == 0);
	sc_queue_term(&q);
}

void test1(void)
{
	int count = 0;
	int t;
	int i = 0;
	struct sc_queue_int p;

	sc_queue_init(&p);
	sc_queue_term(&p);

	sc_queue_init(&p);
	sc_queue_term(&p);
	sc_queue_term(&p);

	sc_queue_init(&p);
	sc_queue_add_first(&p, 1);
	sc_queue_add_first(&p, 2);
	sc_queue_add_first(&p, 3);
	assert(sc_queue_del_first(&p) == 3);
	assert(sc_queue_del_first(&p) == 2);
	assert(sc_queue_del_first(&p) == 1);
	sc_queue_term(&p);

	sc_queue_init(&p);
	assert(sc_queue_oom(&p) == false);

	sc_queue_foreach (&p, t) {
		(void) t;
		count++;
	}
	assert(count == 0);
	assert(sc_queue_empty(&p) == true);
	assert(sc_queue_size(&p) == 0);

	sc_queue_add_first(&p, 2);
	sc_queue_add_first(&p, 3);
	sc_queue_add_first(&p, 4);
	sc_queue_add_first(&p, 5);
	sc_queue_add_first(&p, 6);
	sc_queue_add_last(&p, 1);
	sc_queue_add_last(&p, 0);

	assert(sc_queue_empty(&p) == false);

	i = 6;
	sc_queue_foreach (&p, t) {
		assert(t == i--);
		count += t;
	}
	assert(count == 6 * 7 / 2);
	assert(sc_queue_size(&p) == 7);

	assert(sc_queue_peek_first(&p) == 6);
	assert(sc_queue_size(&p) == 7);
	assert(sc_queue_peek_last(&p) == 0);
	assert(sc_queue_size(&p) == 7);

	t = sc_queue_del_first(&p);
	assert(t == 6);
	assert(sc_queue_size(&p) == 6);

	t = sc_queue_del_last(&p);
	assert(t == 0);
	assert(sc_queue_size(&p) == 5);

	sc_queue_clear(&p);
	assert(sc_queue_size(&p) == 0);
	assert(sc_queue_empty(&p) == true);

	sc_queue_term(&p);
	sc_queue_term(&p);

	sc_queue_init(&p);
	sc_queue_add_first(&p, 100);
	sc_queue_add_first(&p, 200);
	sc_queue_add_first(&p, 300);
	sc_queue_add_first(&p, 400);
	sc_queue_add_first(&p, 500);
	assert(sc_queue_at(&p, 0) == 500);
	assert(sc_queue_at(&p, 4) == 100);
	sc_queue_term(&p);

	sc_queue_init(&p);

	sc_queue_foreach (&p, t) {
		assert(true);
	}

	sc_queue_add_last(&p, 2);
	sc_queue_add_first(&p, 1);
	sc_queue_add_first(&p, 0);
	sc_queue_add_last(&p, 3);

	i = 0;
	sc_queue_foreach (&p, t) {
		assert(sc_queue_at(&p, i) == i);
		i++;
	}

	sc_queue_term(&p);
}

int main()
{
	fail_test();
	example();
	test0();
	test1();
	return 0;
}

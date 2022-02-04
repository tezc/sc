#include "sc_map.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

void example_int_to_str()
{
	uint32_t key;
	const char *value;
	struct sc_map_64s map;

	sc_map_init_64s(&map, 0, 0);

	sc_map_put_64s(&map, 100, "chicago");
	sc_map_put_64s(&map, 200, "new york");
	sc_map_put_64s(&map, 300, "atlanta");

	value = sc_map_get_64s(&map, 200);
	if (sc_map_found(&map)) {
		printf("Found Value:[%s] \n", value);
	}

	value = sc_map_del_64s(&map, 100);
	if (sc_map_found(&map)) {
		printf("Deleted : %s \n", value);
	}

	sc_map_foreach (&map, key, value) {
		printf("Key:[%d], Value:[%s] \n", key, value);
	}

	value = sc_map_del_64s(&map, 200);
	if (sc_map_found(&map)) {
		printf("Found : %s \n", value);
	}

	value = sc_map_put_64s(&map, 300, "los angeles");
	if (sc_map_found(&map)) {
		printf("overridden : %s \n", value);
	}

	sc_map_term_64s(&map);
}

static char *str_random(size_t size)
{
	static char ch[] = "0123456789"
			   "abcdefghijklmnopqrstuvwxyz"
			   "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	uint32_t index;
	char *dest = malloc(size + 1);

	for (size_t i = 0; i < size; ++i) {
		index = (uint32_t) ((double) rand() / RAND_MAX *
				    (sizeof(ch) - 1));
		dest[i] = ch[index];
	}

	dest[size - 1] = '\0';

	return dest;
}

void test_32()
{
	struct sc_map_32 map;

	assert(sc_map_init_32(&map, 0, 0));
	sc_map_term_32(&map);
	assert(sc_map_init_32(&map, 0, 1) == false);
	assert(sc_map_init_32(&map, 0, 99) == false);

	assert(sc_map_init_32(&map, 16, 94));

	sc_map_del_32(&map, 0);
	assert(sc_map_found(&map) == false);

	sc_map_del_32(&map, 1);
	assert(sc_map_found(&map) == false);

	for (int i = 0; i < 14; i++) {
		sc_map_put_32(&map, i, i);
		assert(!sc_map_oom(&map));
	}

	for (int i = 100; i < 200; i++) {
		sc_map_del_32(&map, i);
		assert(sc_map_found(&map) == false);
	}

	sc_map_clear_32(&map);
	sc_map_clear_32(&map);

	for (int i = 0; i < 5; i++) {
		sc_map_put_32(&map, i, i);
	}
	sc_map_put_32(&map, 31, 15);
	sc_map_put_32(&map, 15, 15);
	sc_map_put_32(&map, 46, 15);

	sc_map_get_32(&map, 19);
	assert(sc_map_found(&map) == false);

	for (int i = 0; i < 5; i++) {
		sc_map_put_32(&map, (5 * i) + i, i);
	}

	sc_map_del_32(&map, 4);
	assert(sc_map_found(&map));

	sc_map_del_32(&map, 46);
	assert(sc_map_found(&map));

	sc_map_del_32(&map, 15);
	assert(sc_map_found(&map));

	sc_map_clear_32(&map);
	for (int i = 1; i < 4; i++) {
		sc_map_put_32(&map, 16 * i, i);
	}
	for (int i = 1; i < 4; i++) {
		sc_map_put_32(&map, 1024 * i, i);
	}

	for (int i = 1; i < 4; i++) {
		sc_map_put_32(&map, 512 * i, i);
	}

	sc_map_del_32(&map, 512);
	assert(sc_map_found(&map));

	sc_map_del_32(&map, 1024);
	assert(sc_map_found(&map));

	sc_map_del_32(&map, 48);
	assert(sc_map_found(&map));

	sc_map_term_32(&map);
}

void test_64()
{
	struct sc_map_64 map;

	assert(sc_map_init_64(&map, 0, 0));
	sc_map_term_64(&map);
	assert(sc_map_init_64(&map, 0, 1) == false);
	assert(sc_map_init_64(&map, 0, 99) == false);

	assert(sc_map_init_64(&map, 16, 94));

	sc_map_del_64(&map, 0);
	assert(!sc_map_found(&map));

	sc_map_del_64(&map, 1);
	assert(!sc_map_found(&map));

	for (int i = 0; i < 14; i++) {
		sc_map_put_64(&map, i, i);
	}

	for (int i = 100; i < 200; i++) {
		sc_map_del_64(&map, i);
		assert(!sc_map_found(&map));
	}

	sc_map_clear_64(&map);
	sc_map_clear_64(&map);

	for (int i = 0; i < 5; i++) {
		sc_map_put_64(&map, i, i);
	}
	sc_map_put_64(&map, 31, 15);
	sc_map_put_64(&map, 15, 15);
	sc_map_put_64(&map, 46, 15);

	sc_map_get_64(&map, 19);
	assert(!sc_map_found(&map));

	for (int i = 0; i < 5; i++) {
		sc_map_put_64(&map, (5 * i) + i, i);
	}

	sc_map_del_64(&map, 4);
	assert(sc_map_found(&map));

	sc_map_del_64(&map, 46);
	assert(sc_map_found(&map));

	sc_map_del_64(&map, 15);
	assert(sc_map_found(&map));

	sc_map_clear_64(&map);
	for (int i = 1; i < 4; i++) {
		sc_map_put_64(&map, 16 * i, i);
	}
	for (int i = 1; i < 4; i++) {
		sc_map_put_64(&map, 1024 * i, i);
	}

	for (int i = 1; i < 4; i++) {
		sc_map_put_64(&map, 512 * i, i);
	}

	sc_map_del_64(&map, 512);
	assert(sc_map_found(&map));

	sc_map_del_64(&map, 1024);
	assert(sc_map_found(&map));

	sc_map_del_64(&map, 48);
	assert(sc_map_found(&map));

	sc_map_term_64(&map);
}

void test_64v()
{
	struct sc_map_64v map;

	assert(sc_map_init_64v(&map, 0, 0));
	sc_map_term_64v(&map);
	assert(sc_map_init_64v(&map, 0, 1) == false);
	assert(sc_map_init_64v(&map, 0, 99) == false);

	assert(sc_map_init_64v(&map, 16, 94));
	sc_map_del_64v(&map, 0);
	assert(sc_map_found(&map) == false);

	sc_map_del_64v(&map, 1);
	assert(sc_map_found(&map) == false);

	for (int i = 0; i < 14; i++) {
		sc_map_put_64v(&map, i, NULL);
	}

	for (int i = 100; i < 200; i++) {
		sc_map_del_64v(&map, i);
		assert(sc_map_found(&map) == false);
	}

	sc_map_clear_64v(&map);
	sc_map_clear_64v(&map);

	for (int i = 0; i < 5; i++) {
		sc_map_put_64v(&map, i, NULL);
		assert(!sc_map_found(&map));
	}
	sc_map_put_64v(&map, 31, NULL);
	sc_map_put_64v(&map, 15, NULL);
	sc_map_put_64v(&map, 46, NULL);

	sc_map_get_64v(&map, 19);
	assert(sc_map_found(&map) == false);

	for (int i = 0; i < 5; i++) {
		sc_map_put_64v(&map, (5 * i) + i, NULL);
	}

	sc_map_del_64v(&map, 4);
	assert(sc_map_found(&map));

	sc_map_del_64v(&map, 46);
	assert(sc_map_found(&map));

	sc_map_del_64v(&map, 15);
	assert(sc_map_found(&map));

	sc_map_clear_64v(&map);
	for (int i = 1; i < 4; i++) {
		sc_map_put_64v(&map, 16 * i, NULL);
	}
	for (int i = 1; i < 4; i++) {
		sc_map_put_64v(&map, 1024 * i, NULL);
	}

	for (int i = 1; i < 4; i++) {
		sc_map_put_64v(&map, 512 * i, NULL);
	}

	sc_map_del_64v(&map, 512);
	assert(sc_map_found(&map));

	sc_map_del_64v(&map, 1024);
	assert(sc_map_found(&map));

	sc_map_del_64v(&map, 48);
	assert(sc_map_found(&map));

	sc_map_term_64v(&map);
}

void test_64s()
{
	struct sc_map_64s map;

	assert(sc_map_init_64s(&map, 0, 0));
	sc_map_term_64s(&map);
	assert(sc_map_init_64s(&map, 0, 1) == false);
	assert(sc_map_init_64s(&map, 0, 99) == false);

	assert(sc_map_init_64s(&map, 16, 94));
	sc_map_del_64s(&map, 0);
	assert(!sc_map_found(&map));

	sc_map_del_64s(&map, 1);
	assert(!sc_map_found(&map));

	for (int i = 0; i < 14; i++) {
		sc_map_put_64s(&map, i, NULL);
	}

	for (int i = 100; i < 200; i++) {
		sc_map_del_64s(&map, i);
		assert(!sc_map_found(&map));
	}

	sc_map_clear_64s(&map);
	sc_map_clear_64s(&map);

	for (int i = 0; i < 5; i++) {
		sc_map_put_64s(&map, i, NULL);
	}
	sc_map_put_64s(&map, 31, NULL);
	sc_map_put_64s(&map, 15, NULL);
	sc_map_put_64s(&map, 46, NULL);

	sc_map_get_64s(&map, 19);
	assert(sc_map_found(&map) == false);

	for (int i = 0; i < 5; i++) {
		sc_map_put_64s(&map, (5 * i) + i, NULL);
	}

	sc_map_del_64s(&map, 4);
	assert(sc_map_found(&map));

	sc_map_del_64s(&map, 46);
	assert(sc_map_found(&map));

	sc_map_del_64s(&map, 15);
	assert(sc_map_found(&map));

	sc_map_clear_64s(&map);
	for (int i = 1; i < 4; i++) {
		sc_map_put_64s(&map, 16 * i, NULL);
	}
	for (int i = 1; i < 4; i++) {
		sc_map_put_64s(&map, 1024 * i, NULL);
	}

	for (int i = 1; i < 4; i++) {
		sc_map_put_64s(&map, 512 * i, NULL);
	}

	sc_map_del_64s(&map, 512);
	assert(sc_map_found(&map));

	sc_map_del_64s(&map, 1024);
	assert(sc_map_found(&map));

	sc_map_del_64s(&map, 48);
	assert(sc_map_found(&map));

	sc_map_term_64s(&map);
}

void test_str()
{
	const char *arr = "abcdefghijklmnoprstuvyzabcdefghijklmnoprstuvyzabcdef"
			  "ghijklmnoprstuvyz";
	struct sc_map_str map;

	assert(sc_map_init_str(&map, 0, 0));
	sc_map_term_str(&map);
	assert(sc_map_init_str(&map, 0, 1) == false);
	assert(sc_map_init_str(&map, 0, 99) == false);

	assert(sc_map_init_str(&map, 16, 94));

	sc_map_del_str(&map, NULL);
	assert(!sc_map_found(&map));

	sc_map_del_str(&map, "");
	assert(!sc_map_found(&map));

	for (int i = 0; i < 14; i++) {
		sc_map_put_str(&map, &arr[i], NULL);
	}

	for (int i = 15; i < 30; i++) {
		sc_map_del_str(&map, &arr[i]);
		assert(!sc_map_found(&map));
	}

	sc_map_clear_str(&map);
	sc_map_clear_str(&map);

	sc_map_put_str(&map, "h", NULL);
	sc_map_put_str(&map, "z", NULL);
	sc_map_get_str(&map, "13");
	assert(!sc_map_found(&map));

	sc_map_get_str(&map, NULL);
	assert(!sc_map_found(&map));

	sc_map_get_str(&map, "h");
	assert(sc_map_found(&map));

	sc_map_get_str(&map, "z");
	assert(sc_map_found(&map));

	sc_map_get_str(&map, "x");
	assert(!sc_map_found(&map));

	sc_map_put_str(&map, NULL, NULL);

	sc_map_get_str(&map, NULL);
	assert(sc_map_found(&map));

	sc_map_del_str(&map, NULL);
	assert(sc_map_found(&map));

	sc_map_del_str(&map, "h");
	assert(sc_map_found(&map));

	sc_map_del_str(&map, "13");
	assert(!sc_map_found(&map));

	sc_map_clear_str(&map);
	assert(sc_map_size_str(&map) == 0);

	for (int i = 0; i < 5; i++) {
		sc_map_put_str(&map, &arr[i], NULL);
	}
	sc_map_put_str(&map, &arr[15], NULL);
	sc_map_put_str(&map, &arr[7], NULL);
	sc_map_put_str(&map, &arr[9], NULL);

	sc_map_get_str(&map, &arr[16]);
	assert(!sc_map_found(&map));

	for (int i = 0; i < 5; i++) {
		sc_map_put_str(&map, &arr[(5 * i) + i], NULL);
	}

	sc_map_del_str(&map, &arr[4]);
	assert(sc_map_found(&map));

	sc_map_del_str(&map, &arr[6]);
	assert(sc_map_found(&map));

	sc_map_del_str(&map, &arr[15]);
	assert(sc_map_found(&map));

	sc_map_clear_str(&map);
	sc_map_put_str(&map, "h", NULL);
	sc_map_put_str(&map, "z", NULL);
	sc_map_del_str(&map, "h");
	assert(sc_map_found(&map));

	sc_map_clear_str(&map);

	sc_map_put_str(&map, "h", NULL);
	sc_map_put_str(&map, "z", NULL);
	sc_map_put_str(&map, "13", NULL);
	sc_map_del_str(&map, "z");
	assert(sc_map_found(&map));

	sc_map_term_str(&map);
}

void test_sv()
{
	const char *arr = "abcdefghijklmnoprstuvyzabcdefghijklmnoprstuvyzabcdef"
			  "ghijklmnoprstuvyz";
	struct sc_map_sv map;

	assert(sc_map_init_sv(&map, 0, 0));
	sc_map_term_sv(&map);
	assert(sc_map_init_sv(&map, 0, 1) == false);
	assert(sc_map_init_sv(&map, 0, 99) == false);

	assert(sc_map_init_sv(&map, 16, 94));
	sc_map_del_sv(&map, NULL);
	assert(!sc_map_found(&map));

	sc_map_del_sv(&map, "");
	assert(!sc_map_found(&map));

	for (int i = 0; i < 14; i++) {
		sc_map_put_sv(&map, &arr[i], NULL);
	}

	for (int i = 15; i < 30; i++) {
		sc_map_del_sv(&map, &arr[i]);
		assert(!sc_map_found(&map));
	}

	sc_map_clear_sv(&map);
	sc_map_clear_sv(&map);

	sc_map_put_sv(&map, "h", NULL);
	sc_map_put_sv(&map, "z", NULL);

	sc_map_get_sv(&map, "13");
	assert(!sc_map_found(&map));

	sc_map_get_sv(&map, NULL);
	assert(!sc_map_found(&map));

	sc_map_get_sv(&map, "h");
	assert(sc_map_found(&map));

	sc_map_get_sv(&map, "z");
	assert(sc_map_found(&map));

	sc_map_get_sv(&map, "x");
	assert(!sc_map_found(&map));

	sc_map_put_sv(&map, NULL, NULL);
	sc_map_get_sv(&map, NULL);
	assert(sc_map_found(&map));

	sc_map_del_sv(&map, NULL);
	assert(sc_map_found(&map));

	sc_map_del_sv(&map, "h");
	assert(sc_map_found(&map));

	sc_map_del_sv(&map, "13");
	assert(!sc_map_found(&map));

	sc_map_clear_sv(&map);
	assert(sc_map_size_sv(&map) == 0);

	for (int i = 0; i < 5; i++) {
		sc_map_put_sv(&map, &arr[i], NULL);
	}
	sc_map_put_sv(&map, &arr[15], NULL);
	sc_map_put_sv(&map, &arr[7], NULL);
	sc_map_put_sv(&map, &arr[9], NULL);

	sc_map_get_sv(&map, &arr[16]);
	assert(!sc_map_found(&map));

	for (int i = 0; i < 5; i++) {
		sc_map_put_sv(&map, &arr[(5 * i) + i], NULL);
	}

	sc_map_del_sv(&map, &arr[4]);
	assert(sc_map_found(&map));

	sc_map_del_sv(&map, &arr[6]);
	assert(sc_map_found(&map));

	sc_map_del_sv(&map, &arr[15]);
	assert(sc_map_found(&map));

	sc_map_clear_sv(&map);
	sc_map_put_sv(&map, "h", NULL);
	sc_map_put_sv(&map, "z", NULL);
	sc_map_del_sv(&map, "h");
	assert(sc_map_found(&map));
	sc_map_clear_sv(&map);

	sc_map_put_sv(&map, "h", NULL);
	sc_map_put_sv(&map, "z", NULL);
	sc_map_put_sv(&map, "13", NULL);
	sc_map_del_sv(&map, "z");
	assert(sc_map_found(&map));

	sc_map_term_sv(&map);
}

void test_s64()
{
	const char *arr = "abcdefghijklmnoprstuvyzabcdefghijklmnoprstuvyzabcdef"
			  "ghijklmnoprstuvyz";
	struct sc_map_s64 map;

	assert(sc_map_init_s64(&map, 0, 0));
	sc_map_term_s64(&map);
	assert(sc_map_init_s64(&map, 0, 1) == false);
	assert(sc_map_init_s64(&map, 0, 99) == false);

	assert(sc_map_init_s64(&map, 16, 94));
	sc_map_del_s64(&map, NULL);
	assert(!sc_map_found(&map));

	sc_map_del_s64(&map, "");
	assert(!sc_map_found(&map));

	for (int i = 0; i < 14; i++) {
		sc_map_put_s64(&map, &arr[i], 0);
	}

	for (int i = 15; i < 30; i++) {
		sc_map_del_s64(&map, &arr[i]);
		assert(!sc_map_found(&map));
	}

	sc_map_clear_s64(&map);
	sc_map_clear_s64(&map);

	sc_map_put_s64(&map, "h", 0);
	sc_map_put_s64(&map, "z", 0);

	sc_map_get_s64(&map, "13");
	assert(!sc_map_found(&map));

	sc_map_get_s64(&map, NULL);
	assert(!sc_map_found(&map));

	sc_map_get_s64(&map, "h");
	assert(sc_map_found(&map));

	sc_map_get_s64(&map, "z");
	assert(sc_map_found(&map));

	sc_map_get_s64(&map, "x");
	assert(!sc_map_found(&map));

	sc_map_put_s64(&map, NULL, 0);
	assert(!sc_map_found(&map));

	sc_map_get_s64(&map, NULL);
	assert(sc_map_found(&map));

	sc_map_del_s64(&map, NULL);
	assert(sc_map_found(&map));

	sc_map_del_s64(&map, "h");
	assert(sc_map_found(&map));

	sc_map_del_s64(&map, "13");
	assert(!sc_map_found(&map));
	sc_map_clear_s64(&map);
	assert(sc_map_size_s64(&map) == 0);

	for (int i = 0; i < 5; i++) {
		sc_map_put_s64(&map, &arr[i], 0);
	}
	sc_map_put_s64(&map, &arr[15], 0);
	sc_map_put_s64(&map, &arr[7], 0);
	sc_map_put_s64(&map, &arr[9], 0);

	assert(!sc_map_oom(&map));

	sc_map_get_s64(&map, &arr[16]);
	assert(!sc_map_found(&map));

	for (int i = 0; i < 5; i++) {
		sc_map_put_s64(&map, &arr[(5 * i) + i], 0);
		assert(!sc_map_oom(&map));
	}

	sc_map_del_s64(&map, &arr[4]);
	assert(sc_map_found(&map));

	sc_map_del_s64(&map, &arr[6]);
	assert(sc_map_found(&map));

	sc_map_del_s64(&map, &arr[15]);
	assert(sc_map_found(&map));

	sc_map_clear_s64(&map);
	sc_map_put_s64(&map, "h", 0);
	sc_map_put_s64(&map, "z", 0);
	sc_map_del_s64(&map, "h");
	assert(sc_map_found(&map));

	sc_map_clear_s64(&map);

	sc_map_put_s64(&map, "h", 0);
	sc_map_put_s64(&map, "z", 0);
	sc_map_put_s64(&map, "13", 0);
	sc_map_del_s64(&map, "z");
	assert(sc_map_found(&map));

	sc_map_term_s64(&map);
}

void test0()
{
	uint64_t val;
	struct sc_map_64 map;

	sc_map_init_64(&map, 128, 0);

	sc_map_put_64(&map, 100, 100);
	assert(!sc_map_oom(&map));
	assert(!sc_map_found(&map));

	val = sc_map_get_64(&map, 100);
	assert(val == 100);
	assert(sc_map_found(&map));

	val = sc_map_put_64(&map, 100, 200);
	assert(sc_map_found(&map));
	assert(val == 100);

	val = sc_map_get_64(&map, 100);
	assert(val == 200);
	assert(sc_map_found(&map));

	val = sc_map_del_64(&map, 100);
	assert(sc_map_found(&map));
	assert(val == 200);

	val = sc_map_put_64(&map, 1, 1);
	assert(!sc_map_found(&map));
	val = sc_map_del_64(&map, 2);
	assert(!sc_map_found(&map));
	val = sc_map_get_64(&map, 1);
	assert(sc_map_found(&map));
	assert(val == 1);
	val = sc_map_put_64(&map, 2, 2);
	assert(!sc_map_found(&map));

	val = sc_map_del_64(&map, 1);
	assert(sc_map_found(&map));
	assert(val == 1);

	val = sc_map_del_64(&map, 2);
	assert(sc_map_found(&map));
	assert(val == 2);

	sc_map_term_64(&map);
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

	sc_map_init_str(&map, 0, 0);
	sc_map_put_str(&map, "100", "200");
	value = sc_map_get_str(&map, "100");
	assert(strcmp(value, "200") == 0);
	sc_map_term_str(&map);
	sc_map_put_str(&map, "100", "200");
	value = sc_map_get_str(&map, "100");
	assert(strcmp(value, "200") == 0);
	sc_map_term_str(&map);

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

	sc_map_put_str(&map, "key", "value");
	sc_map_put_str(&map, "key", "value2");

	value = sc_map_get_str(&map, "key");
	assert(sc_map_found(&map));
	assert(strcmp(value, "value2") == 0);

	value = sc_map_del_str(&map, "key");
	assert(sc_map_found(&map));
	value = sc_map_get_str(&map, "key");
	assert(!sc_map_found(&map));
	sc_map_put_str(&map, "key", "value3");
	value = sc_map_del_str(&map, "key");
	assert(sc_map_found(&map));
	assert(strcmp(value, "value3") == 0);
	key = sc_map_del_str(&map, "key");
	assert(!sc_map_found(&map));

	sc_map_put_str(&map, "key", "value");
	assert(sc_map_size_str(&map) == 1);
	sc_map_put_str(&map, NULL, "nullvalue");
	assert(sc_map_size_str(&map) == 2);
	value = sc_map_get_str(&map, NULL);
	assert(sc_map_found(&map));
	assert(strcmp(value, "nullvalue") == 0);
	sc_map_del_str(&map, NULL);
	assert(sc_map_found(&map));
	assert(sc_map_size_str(&map) == 1);

	sc_map_clear_str(&map);
	assert(sc_map_size_str(&map) == 0);

	for (int i = 0; i < 100; i++) {
		sc_map_put_str(&map, keys[i], values[i]);
	}

	for (int i = 0; i < 100; i++) {
		value = sc_map_get_str(&map, keys[i]);
		assert(sc_map_found(&map));
		assert(strcmp(value, values[i]) == 0);
	}

	sc_map_put_str(&map, keys[0], values[101]);
	assert(sc_map_size_str(&map) == 100);
	sc_map_put_str(&map, keys[101], values[102]);
	assert(sc_map_size_str(&map) == 101);
	sc_map_clear_str(&map);
	assert(sc_map_size_str(&map) == 0);

	for (int i = 0; i < 100; i++) {
		sc_map_put_str(&map, keys[i], values[i]);
	}

	for (int i = 0; i < 100; i++) {
		value = sc_map_get_str(&map, keys[i]);
		assert(sc_map_found(&map));
		assert(strcmp(value, values[i]) == 0);
	}

	sc_map_term_str(&map);

	assert(sc_map_init_str(&map, 0, 0));
	for (int i = 0; i < 100; i++) {
		sc_map_put_str(&map, keys[i], values[i]);
	}

	bool found;
	sc_map_foreach (&map, key, value) {
		found = false;
		for (int j = 0; j < 100; j++) {
			if (strcmp(key, keys[j]) == 0 &&
			    strcmp(value, values[j]) == 0) {
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
	uint32_t random;

	for (int i = 0; i < 128; i++) {
retry:
		random = (uint32_t) rand();
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
	sc_map_put_32(&map, 0, 0);
	sc_map_clear_32(&map);
	assert(sc_map_size_32(&map) == 0);

	for (int i = 0; i < 100; i++) {
		sc_map_put_32(&map, keys[i], values[i]);
		value = sc_map_get_32(&map, keys[i]);
		assert(sc_map_found(&map));
		assert(value == values[i]);
		sc_map_put_32(&map, keys[i], values[i]);

		value = sc_map_del_32(&map, keys[i]);
		assert(sc_map_found(&map));
		assert(value == values[i]);
	}

	for (int i = 0; i < 128; i++) {
		sc_map_put_32(&map, keys[i], values[i]);
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
	uint32_t random;

	for (int i = 0; i < 128; i++) {
retry:
		random = (uint32_t) rand();
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
	sc_map_put_64(&map, 0, 0);
	sc_map_clear_64(&map);
	assert(sc_map_size_64(&map) == 0);

	for (int i = 0; i < 100; i++) {
		sc_map_put_64(&map, keys[i], values[i]);
		value = sc_map_get_64(&map, keys[i]);
		assert(sc_map_found(&map));
		assert(value == values[i]);

		sc_map_put_64(&map, keys[i], values[i]);
		value = sc_map_del_64(&map, keys[i]);
		assert(sc_map_found(&map));
		assert(value == values[i]);
	}

	for (int i = 0; i < 128; i++) {
		sc_map_put_64(&map, keys[i], values[i]);
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
		sc_map_put_64s(&map64s, i, NULL);
		c = sc_map_get_64s(&map64s, i);
		assert(sc_map_found(&map64s));
		assert(c == NULL);
	}
	assert(sc_map_size_64s(&map64s) == 100);
	for (int i = 0; i < 100; i++) {
		c = sc_map_del_64s(&map64s, i);
		assert(sc_map_found(&map64s));
		assert(c == NULL);
	}
	assert(sc_map_size_64s(&map64s) == 0);
	sc_map_put_64s(&map64s, 3, NULL);
	assert(sc_map_size_64s(&map64s) == 1);
	sc_map_clear_64s(&map64s);
	assert(sc_map_size_64s(&map64s) == 0);

	sc_map_term_64s(&map64s);

	const char *v;
	struct sc_map_64v map64v;

	assert(sc_map_init_64v(&map64v, 1, 87));
	for (int i = 0; i < 100; i++) {
		sc_map_put_64v(&map64v, i, NULL);
		v = sc_map_get_64v(&map64v, i);
		assert(sc_map_found(&map64v));
		assert(v == NULL);
	}
	assert(sc_map_size_64v(&map64v) == 100);
	for (int i = 0; i < 100; i++) {
		v = sc_map_del_64v(&map64v, i);
		assert(sc_map_found(&map64v));
		assert(v == NULL);
	}
	assert(sc_map_size_64v(&map64v) == 0);
	sc_map_put_64v(&map64v, 3, NULL);
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

	struct sc_map_str mapstr;
	assert(sc_map_init_str(&mapstr, 0, 26));
	for (int i = 0; i < 64; i++) {
		sc_map_put_str(&mapstr, keys[i], (void *) (uintptr_t) i);
	}

	v = sc_map_get_str(&mapstr, keys[0]);
	assert(sc_map_found(&mapstr));
	assert(v == 0);
	assert(sc_map_size_str(&mapstr) == 64);

	v = sc_map_del_str(&mapstr, keys[12]);
	assert(sc_map_found(&mapstr));
	assert(v == (void *) 12);
	assert(sc_map_size_str(&mapstr) == 63);
	sc_map_clear_str(&mapstr);
	sc_map_term_str(&mapstr);

	struct sc_map_sv mapsv;
	assert(sc_map_init_sv(&mapsv, 0, 26));
	for (int i = 0; i < 64; i++) {
		sc_map_put_sv(&mapsv, keys[i], (void *) (uintptr_t) i);
	}
	v = sc_map_get_sv(&mapsv, keys[0]);
	assert(sc_map_found(&mapsv));
	assert(v == 0);
	assert(sc_map_size_sv(&mapsv) == 64);

	v = sc_map_del_sv(&mapsv, keys[12]);
	assert(sc_map_found(&mapsv));
	assert(v == (void *) 12);
	assert(sc_map_size_sv(&mapsv) == 63);
	sc_map_clear_sv(&mapsv);
	sc_map_term_sv(&mapsv);

	uint64_t val;
	struct sc_map_s64 maps64;

	assert(sc_map_init_s64(&maps64, 0, 26));
	for (int i = 0; i < 64; i++) {
		sc_map_put_s64(&maps64, keys[i], i);
	}

	val = sc_map_get_s64(&maps64, keys[0]);
	assert(sc_map_found(&maps64));
	assert(val == 0);
	assert(sc_map_size_s64(&maps64) == 64);

	val = sc_map_del_s64(&maps64, keys[12]);
	assert(sc_map_found(&maps64));
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

static void test6()
{
	const int count = 120000;
	uint32_t val;
	struct sc_map_32 map;

	srand(time(NULL));

	uint32_t *keys = malloc(sizeof(uint32_t) * count);
	uint32_t *values = malloc(sizeof(uint32_t) * count);

	for (int i = 0; i < count; i++) {
		keys[i] = i;
		values[i] = (i * 33);
	}

	sc_map_init_32(&map, 0, 0);
	for (int i = 0; i < count; i++) {
		sc_map_put_32(&map, keys[i], values[i]);

		if (i % 7 == 0 || i % 17 == 0 || i % 79 == 0) {
			val = sc_map_del_32(&map, keys[i]);
			assert(sc_map_found(&map));
			assert(val == values[i]);
		}
	}

	for (int i = 0; i < count; i++) {
		if (i % 7 == 0 || i % 17 == 0 || i % 79 == 0) {
			continue;
		}

		val = sc_map_get_32(&map, keys[i]);
		assert(sc_map_found(&map));
		assert(val == values[i]);
	}

	sc_map_term_32(&map);

	free(keys);
	free(values);
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

void fail_test_32()
{
	struct sc_map_32 map;

	fail_calloc = true;
	assert(!sc_map_init_32(&map, 10, 0));
	fail_calloc = false;
	assert(sc_map_init_32(&map, 10, 0));

	fail_calloc = true;

	for (int i = 0; i < 20; i++) {
		sc_map_put_32(&map, i, i);
	}
	assert(sc_map_oom(&map));
	fail_calloc = false;
	sc_map_put_32(&map, 44444, 44444);

	for (size_t i = 0; i < SC_MAP_MAX; i++) {
		sc_map_put_32(&map, i, i);
	}
	assert(sc_map_oom(&map));
	fail_calloc = false;

	sc_map_term_32(&map);
}

void fail_test_64()
{
	struct sc_map_64 map;

	fail_calloc = true;
	assert(!sc_map_init_64(&map, 10, 0));
	fail_calloc = false;
	assert(sc_map_init_64(&map, 10, 0));

	fail_calloc = true;

	for (int i = 0; i < 20; i++) {
		sc_map_put_64(&map, i, i);
	}
	assert(sc_map_oom(&map));
	fail_calloc = false;
	sc_map_put_64(&map, 44444, 44444);

	for (size_t i = 0; i < SC_MAP_MAX; i++) {
		sc_map_put_64(&map, i, i);
	}
	assert(sc_map_oom(&map));
	fail_calloc = false;

	sc_map_term_64(&map);
}

void fail_test_64v()
{
	struct sc_map_64v map;

	fail_calloc = true;
	assert(!sc_map_init_64v(&map, 10, 0));
	fail_calloc = false;
	assert(sc_map_init_64v(&map, 10, 0));

	fail_calloc = true;

	for (int i = 0; i < 20; i++) {
		sc_map_put_64v(&map, i, NULL);
	}
	assert(sc_map_oom(&map));
	fail_calloc = false;
	sc_map_put_64v(&map, 44444, NULL);

	for (size_t i = 0; i < SC_MAP_MAX; i++) {
		sc_map_put_64v(&map, i, NULL);
	}
	assert(sc_map_oom(&map));
	fail_calloc = false;

	sc_map_term_64v(&map);
}

void fail_test_64s()
{
	struct sc_map_64s map;

	fail_calloc = true;
	assert(!sc_map_init_64s(&map, 10, 0));
	fail_calloc = false;
	assert(sc_map_init_64s(&map, 10, 0));

	fail_calloc = true;

	for (int i = 0; i < 20; i++) {
		sc_map_put_64s(&map, i, NULL);
	}
	assert(sc_map_oom(&map));
	fail_calloc = false;
	sc_map_put_64s(&map, 44444, NULL);

	for (size_t i = 0; i < SC_MAP_MAX; i++) {
		sc_map_put_64s(&map, i, NULL);
	}
	assert(sc_map_oom(&map));
	fail_calloc = false;

	sc_map_term_64s(&map);
}

void fail_test_str()
{
	struct sc_map_str map;
	const char *v;
	const char *s = "abcdefghijklmnoprstuvyz10111213141516";

	fail_calloc = true;
	assert(!sc_map_init_str(&map, 10, 0));
	fail_calloc = false;
	assert(sc_map_init_str(&map, 10, 0));

	fail_calloc = true;

	for (int i = 0; i < 20; i++) {
		sc_map_put_str(&map, &s[i], NULL);
	}
	assert(sc_map_oom(&map));
	fail_calloc = false;
	sc_map_put_str(&map, &s[21], NULL);
	sc_map_clear_str(&map);

	for (size_t i = 0; i < SC_MAP_MAX; i++) {
		char *c = str_random(32);

		sc_map_put_str(&map, c, NULL);
		if (sc_map_oom(&map)) {
			free(c);
			break;
		}
	}
	assert(sc_map_oom(&map));
	fail_calloc = false;

	sc_map_foreach_key (&map, v) {
		free((void *) v);
	}

	sc_map_term_str(&map);
}

void fail_test_sv()
{
	struct sc_map_sv map;
	const char *v;
	const char *s = "abcdefghijklmnoprstuvyz10111213141516";

	fail_calloc = true;
	assert(!sc_map_init_sv(&map, 10, 0));
	fail_calloc = false;
	assert(sc_map_init_sv(&map, 10, 0));

	fail_calloc = true;

	for (int i = 0; i < 20; i++) {
		sc_map_put_sv(&map, &s[i], NULL);
	}
	assert(sc_map_oom(&map));
	fail_calloc = false;
	sc_map_put_sv(&map, &s[21], NULL);
	sc_map_clear_sv(&map);

	for (size_t i = 0; i < SC_MAP_MAX; i++) {
		char *c = str_random(32);
		sc_map_put_sv(&map, c, NULL);
		if (sc_map_oom(&map)) {
			free(c);
			break;
		}
	}
	assert(sc_map_oom(&map));
	fail_calloc = false;

	sc_map_foreach_key (&map, v) {
		free((void *) v);
	}

	sc_map_term_sv(&map);
}

void fail_test_s64()
{
	struct sc_map_s64 map;
	const char *v;
	const char *s = "abcdefghijklmnoprstuvyz10111213141516";

	fail_calloc = true;
	assert(!sc_map_init_s64(&map, 10, 0));
	fail_calloc = false;
	assert(sc_map_init_s64(&map, 10, 0));

	fail_calloc = true;

	for (int i = 0; i < 20; i++) {
		sc_map_put_s64(&map, &s[i], 0);
	}
	assert(sc_map_oom(&map));
	fail_calloc = false;
	sc_map_put_s64(&map, &s[21], 0);
	sc_map_clear_s64(&map);

	for (size_t i = 0; i < SC_MAP_MAX; i++) {
		char *c = str_random(32);
		sc_map_put_s64(&map, c, 0);
		if (sc_map_oom(&map)) {
			free(c);
			break;
		}
	}
	assert(sc_map_oom(&map));
	fail_calloc = false;

	sc_map_foreach_key (&map, v) {
		free((void *) v);
	}

	sc_map_term_s64(&map);
}

#else
void fail_test_32(void)
{
}
void fail_test_64(void)
{
}
void fail_test_64v(void)
{
}
void fail_test_64s(void)
{
}
void fail_test_str(void)
{
}
void fail_test_sv(void)
{
}
void fail_test_s64(void)
{
}
#endif

int main()
{
	example();
	example_int_to_str();
	fail_test_32();
	fail_test_64();
	fail_test_64v();
	fail_test_64s();
	fail_test_str();
	fail_test_sv();
	fail_test_s64();
	test0();
	test1();
	test2();
	test3();
	test4();
	test5();
	test6();
	test_32();
	test_64();
	test_64s();
	test_64v();
	test_str();
	test_sv();
	test_s64();

	return 0;
}

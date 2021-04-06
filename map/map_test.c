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

static char *str_random(size_t size)
{
	static char ch[] = "0123456789"
			   "abcdefghijklmnopqrstuvwxyz"
			   "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	uint32_t index;
	char *dest = malloc(size + 1);

	for (size_t i = 0; i < size; ++i) {
		index = (uint32_t)((double) rand() / RAND_MAX *
				   (sizeof(ch) - 1));
		dest[i] = ch[index];
	}

	dest[size - 1] = '\0';

	return dest;
}

void test_32()
{
	uint32_t val;
	struct sc_map_32 map;

	assert(sc_map_init_32(&map, 0, 0));
	sc_map_term_32(&map);
	assert(sc_map_init_32(&map, 0, 1) == false);
	assert(sc_map_init_32(&map, 0, 99) == false);

	assert(sc_map_init_32(&map, 16, 94));
	assert(sc_map_del_32(&map, 0, NULL) == false);
	assert(sc_map_del_32(&map, 1, NULL) == false);

	for (int i = 0; i < 14; i++) {
		assert(sc_map_put_32(&map, i, i) == true);
	}

	for (int i = 100; i < 200; i++) {
		assert(sc_map_del_32(&map, i, NULL) == false);
	}

	sc_map_clear_32(&map);
	sc_map_clear_32(&map);

	for (int i = 0; i < 5; i++) {
		assert(sc_map_put_32(&map, i, i) == true);
	}
	assert(sc_map_put_32(&map, 31, 15) == true);
	assert(sc_map_put_32(&map, 15, 15) == true);
	assert(sc_map_put_32(&map, 46, 15) == true);

	assert(sc_map_get_32(&map, 19, &val) == false);

	for (int i = 0; i < 5; i++) {
		assert(sc_map_put_32(&map, (5 * i) + i, i) == true);
	}

	assert(sc_map_del_32(&map, 4, NULL) == true);
	assert(sc_map_del_32(&map, 46, NULL) == true);
	assert(sc_map_del_32(&map, 15, NULL) == true);

	sc_map_clear_32(&map);
	for (int i = 1; i < 4; i++) {
		assert(sc_map_put_32(&map, 16 * i, i) == true);
	}
	for (int i = 1; i < 4; i++) {
		assert(sc_map_put_32(&map, 1024 * i, i) == true);
	}

	for (int i = 1; i < 4; i++) {
		assert(sc_map_put_32(&map, 512 * i, i) == true);
	}

	assert(sc_map_del_32(&map, 512, NULL) == true);
	assert(sc_map_del_32(&map, 1024, NULL) == true);
	assert(sc_map_del_32(&map, 48, NULL) == true);

	sc_map_term_32(&map);
}

void test_64()
{
	uint64_t val;
	struct sc_map_64 map;

	assert(sc_map_init_64(&map, 0, 0));
	sc_map_term_64(&map);
	assert(sc_map_init_64(&map, 0, 1) == false);
	assert(sc_map_init_64(&map, 0, 99) == false);

	assert(sc_map_init_64(&map, 16, 94));
	assert(sc_map_del_64(&map, 0, NULL) == false);
	assert(sc_map_del_64(&map, 1, NULL) == false);

	for (int i = 0; i < 14; i++) {
		assert(sc_map_put_64(&map, i, i) == true);
	}

	for (int i = 100; i < 200; i++) {
		assert(sc_map_del_64(&map, i, NULL) == false);
	}

	sc_map_clear_64(&map);
	sc_map_clear_64(&map);

	for (int i = 0; i < 5; i++) {
		assert(sc_map_put_64(&map, i, i) == true);
	}
	assert(sc_map_put_64(&map, 31, 15) == true);
	assert(sc_map_put_64(&map, 15, 15) == true);
	assert(sc_map_put_64(&map, 46, 15) == true);

	assert(sc_map_get_64(&map, 19, &val) == false);

	for (int i = 0; i < 5; i++) {
		assert(sc_map_put_64(&map, (5 * i) + i, i) == true);
	}

	assert(sc_map_del_64(&map, 4, NULL) == true);
	assert(sc_map_del_64(&map, 46, NULL) == true);
	assert(sc_map_del_64(&map, 15, NULL) == true);

	sc_map_clear_64(&map);
	for (int i = 1; i < 4; i++) {
		assert(sc_map_put_64(&map, 16 * i, i) == true);
	}
	for (int i = 1; i < 4; i++) {
		assert(sc_map_put_64(&map, 1024 * i, i) == true);
	}

	for (int i = 1; i < 4; i++) {
		assert(sc_map_put_64(&map, 512 * i, i) == true);
	}

	assert(sc_map_del_64(&map, 512, NULL) == true);
	assert(sc_map_del_64(&map, 1024, NULL) == true);
	assert(sc_map_del_64(&map, 48, NULL) == true);

	sc_map_term_64(&map);
}

void test_64v()
{
	void *val;
	struct sc_map_64v map;

	assert(sc_map_init_64v(&map, 0, 0));
	sc_map_term_64v(&map);
	assert(sc_map_init_64v(&map, 0, 1) == false);
	assert(sc_map_init_64v(&map, 0, 99) == false);

	assert(sc_map_init_64v(&map, 16, 94));
	assert(sc_map_del_64v(&map, 0, NULL) == false);
	assert(sc_map_del_64v(&map, 1, NULL) == false);

	for (int i = 0; i < 14; i++) {
		assert(sc_map_put_64v(&map, i, NULL) == true);
	}

	for (int i = 100; i < 200; i++) {
		assert(sc_map_del_64v(&map, i, NULL) == false);
	}

	sc_map_clear_64v(&map);
	sc_map_clear_64v(&map);

	for (int i = 0; i < 5; i++) {
		assert(sc_map_put_64v(&map, i, NULL) == true);
	}
	assert(sc_map_put_64v(&map, 31, NULL) == true);
	assert(sc_map_put_64v(&map, 15, NULL) == true);
	assert(sc_map_put_64v(&map, 46, NULL) == true);

	assert(sc_map_get_64v(&map, 19, &val) == false);

	for (int i = 0; i < 5; i++) {
		assert(sc_map_put_64v(&map, (5 * i) + i, NULL) == true);
	}

	assert(sc_map_del_64v(&map, 4, NULL) == true);
	assert(sc_map_del_64v(&map, 46, NULL) == true);
	assert(sc_map_del_64v(&map, 15, NULL) == true);

	sc_map_clear_64v(&map);
	for (int i = 1; i < 4; i++) {
		assert(sc_map_put_64v(&map, 16 * i, NULL) == true);
	}
	for (int i = 1; i < 4; i++) {
		assert(sc_map_put_64v(&map, 1024 * i, NULL) == true);
	}

	for (int i = 1; i < 4; i++) {
		assert(sc_map_put_64v(&map, 512 * i, NULL) == true);
	}

	assert(sc_map_del_64v(&map, 512, NULL) == true);
	assert(sc_map_del_64v(&map, 1024, NULL) == true);
	assert(sc_map_del_64v(&map, 48, NULL) == true);

	sc_map_term_64v(&map);
}

void test_64s()
{
	const char *val;
	struct sc_map_64s map;

	assert(sc_map_init_64s(&map, 0, 0));
	sc_map_term_64s(&map);
	assert(sc_map_init_64s(&map, 0, 1) == false);
	assert(sc_map_init_64s(&map, 0, 99) == false);

	assert(sc_map_init_64s(&map, 16, 94));
	assert(sc_map_del_64s(&map, 0, NULL) == false);
	assert(sc_map_del_64s(&map, 1, NULL) == false);

	for (int i = 0; i < 14; i++) {
		assert(sc_map_put_64s(&map, i, NULL) == true);
	}

	for (int i = 100; i < 200; i++) {
		assert(sc_map_del_64s(&map, i, NULL) == false);
	}

	sc_map_clear_64s(&map);
	sc_map_clear_64s(&map);

	for (int i = 0; i < 5; i++) {
		assert(sc_map_put_64s(&map, i, NULL) == true);
	}
	assert(sc_map_put_64s(&map, 31, NULL) == true);
	assert(sc_map_put_64s(&map, 15, NULL) == true);
	assert(sc_map_put_64s(&map, 46, NULL) == true);

	assert(sc_map_get_64s(&map, 19, &val) == false);

	for (int i = 0; i < 5; i++) {
		assert(sc_map_put_64s(&map, (5 * i) + i, NULL) == true);
	}

	assert(sc_map_del_64s(&map, 4, NULL) == true);
	assert(sc_map_del_64s(&map, 46, NULL) == true);
	assert(sc_map_del_64s(&map, 15, NULL) == true);

	sc_map_clear_64s(&map);
	for (int i = 1; i < 4; i++) {
		assert(sc_map_put_64s(&map, 16 * i, NULL) == true);
	}
	for (int i = 1; i < 4; i++) {
		assert(sc_map_put_64s(&map, 1024 * i, NULL) == true);
	}

	for (int i = 1; i < 4; i++) {
		assert(sc_map_put_64s(&map, 512 * i, NULL) == true);
	}

	assert(sc_map_del_64s(&map, 512, NULL) == true);
	assert(sc_map_del_64s(&map, 1024, NULL) == true);
	assert(sc_map_del_64s(&map, 48, NULL) == true);

	sc_map_term_64s(&map);
}

void test_str()
{
	const char *arr = "abcdefghijklmnoprstuvyzabcdefghijklmnoprstuvyzabcdef"
			  "ghijklmnoprstuvyz";
	const char *val;
	struct sc_map_str map;

	assert(sc_map_init_str(&map, 0, 0));
	sc_map_term_str(&map);
	assert(sc_map_init_str(&map, 0, 1) == false);
	assert(sc_map_init_str(&map, 0, 99) == false);

	assert(sc_map_init_str(&map, 16, 94));
	assert(sc_map_del_str(&map, NULL, NULL) == false);
	assert(sc_map_del_str(&map, "", NULL) == false);

	for (int i = 0; i < 14; i++) {
		assert(sc_map_put_str(&map, &arr[i], NULL) == true);
	}

	for (int i = 15; i < 30; i++) {
		assert(sc_map_del_str(&map, &arr[i], NULL) == false);
	}

	sc_map_clear_str(&map);
	sc_map_clear_str(&map);

	assert(sc_map_put_str(&map, "h", NULL) == true);
	assert(sc_map_put_str(&map, "z", NULL) == true);
	assert(sc_map_get_str(&map, "13", &val) == false);
	assert(sc_map_get_str(&map, NULL, &val) == false);
	assert(sc_map_get_str(&map, "h", &val) == true);
	assert(sc_map_get_str(&map, "z", &val) == true);
	assert(sc_map_get_str(&map, "x", &val) == false);
	assert(sc_map_put_str(&map, NULL, NULL) == true);
	assert(sc_map_get_str(&map, NULL, &val) == true);
	assert(sc_map_del_str(&map, NULL, &val) == true);
	assert(sc_map_del_str(&map, "h", &val) == true);
	assert(sc_map_del_str(&map, "13", &val) == false);
	sc_map_clear_str(&map);
	assert(sc_map_size_str(&map) == 0);

	for (int i = 0; i < 5; i++) {
		assert(sc_map_put_str(&map, &arr[i], NULL) == true);
	}
	assert(sc_map_put_str(&map, &arr[15], NULL) == true);
	assert(sc_map_put_str(&map, &arr[7], NULL) == true);
	assert(sc_map_put_str(&map, &arr[9], NULL) == true);

	assert(sc_map_get_str(&map, &arr[16], &val) == false);

	for (int i = 0; i < 5; i++) {
		assert(sc_map_put_str(&map, &arr[(5 * i) + i], NULL) == true);
	}

	assert(sc_map_del_str(&map, &arr[4], NULL) == true);
	assert(sc_map_del_str(&map, &arr[6], NULL) == true);
	assert(sc_map_del_str(&map, &arr[15], NULL) == true);

	sc_map_clear_str(&map);
	assert(sc_map_put_str(&map, "h", NULL) == true);
	assert(sc_map_put_str(&map, "z", NULL) == true);
	assert(sc_map_del_str(&map, "h", NULL) == true);
	sc_map_clear_str(&map);

	assert(sc_map_put_str(&map, "h", NULL) == true);
	assert(sc_map_put_str(&map, "z", NULL) == true);
	assert(sc_map_put_str(&map, "13", NULL) == true);
	assert(sc_map_del_str(&map, "z", NULL) == true);

	sc_map_term_str(&map);
}

void test_sv()
{
	const char *arr = "abcdefghijklmnoprstuvyzabcdefghijklmnoprstuvyzabcdef"
			  "ghijklmnoprstuvyz";
	void *val;
	struct sc_map_sv map;

	assert(sc_map_init_sv(&map, 0, 0));
	sc_map_term_sv(&map);
	assert(sc_map_init_sv(&map, 0, 1) == false);
	assert(sc_map_init_sv(&map, 0, 99) == false);

	assert(sc_map_init_sv(&map, 16, 94));
	assert(sc_map_del_sv(&map, NULL, NULL) == false);
	assert(sc_map_del_sv(&map, "", NULL) == false);

	for (int i = 0; i < 14; i++) {
		assert(sc_map_put_sv(&map, &arr[i], NULL) == true);
	}

	for (int i = 15; i < 30; i++) {
		assert(sc_map_del_sv(&map, &arr[i], NULL) == false);
	}

	sc_map_clear_sv(&map);
	sc_map_clear_sv(&map);

	assert(sc_map_put_sv(&map, "h", NULL) == true);
	assert(sc_map_put_sv(&map, "z", NULL) == true);
	assert(sc_map_get_sv(&map, "13", &val) == false);
	assert(sc_map_get_sv(&map, NULL, &val) == false);
	assert(sc_map_get_sv(&map, "h", &val) == true);
	assert(sc_map_get_sv(&map, "z", &val) == true);
	assert(sc_map_get_sv(&map, "x", &val) == false);
	assert(sc_map_put_sv(&map, NULL, NULL) == true);
	assert(sc_map_get_sv(&map, NULL, &val) == true);
	assert(sc_map_del_sv(&map, NULL, &val) == true);
	assert(sc_map_del_sv(&map, "h", &val) == true);
	assert(sc_map_del_sv(&map, "13", &val) == false);
	sc_map_clear_sv(&map);
	assert(sc_map_size_sv(&map) == 0);

	for (int i = 0; i < 5; i++) {
		assert(sc_map_put_sv(&map, &arr[i], NULL) == true);
	}
	assert(sc_map_put_sv(&map, &arr[15], NULL) == true);
	assert(sc_map_put_sv(&map, &arr[7], NULL) == true);
	assert(sc_map_put_sv(&map, &arr[9], NULL) == true);

	assert(sc_map_get_sv(&map, &arr[16], &val) == false);

	for (int i = 0; i < 5; i++) {
		assert(sc_map_put_sv(&map, &arr[(5 * i) + i], NULL) == true);
	}

	assert(sc_map_del_sv(&map, &arr[4], NULL) == true);
	assert(sc_map_del_sv(&map, &arr[6], NULL) == true);
	assert(sc_map_del_sv(&map, &arr[15], NULL) == true);

	sc_map_clear_sv(&map);
	assert(sc_map_put_sv(&map, "h", NULL) == true);
	assert(sc_map_put_sv(&map, "z", NULL) == true);
	assert(sc_map_del_sv(&map, "h", NULL) == true);
	sc_map_clear_sv(&map);

	assert(sc_map_put_sv(&map, "h", NULL) == true);
	assert(sc_map_put_sv(&map, "z", NULL) == true);
	assert(sc_map_put_sv(&map, "13", NULL) == true);
	assert(sc_map_del_sv(&map, "z", NULL) == true);

	sc_map_term_sv(&map);
}

void test_s64()
{
	const char *arr = "abcdefghijklmnoprstuvyzabcdefghijklmnoprstuvyzabcdef"
			  "ghijklmnoprstuvyz";
	uint64_t val;
	struct sc_map_s64 map;

	assert(sc_map_init_s64(&map, 0, 0));
	sc_map_term_s64(&map);
	assert(sc_map_init_s64(&map, 0, 1) == false);
	assert(sc_map_init_s64(&map, 0, 99) == false);

	assert(sc_map_init_s64(&map, 16, 94));
	assert(sc_map_del_s64(&map, NULL, NULL) == false);
	assert(sc_map_del_s64(&map, "", NULL) == false);

	for (int i = 0; i < 14; i++) {
		assert(sc_map_put_s64(&map, &arr[i], 0) == true);
	}

	for (int i = 15; i < 30; i++) {
		assert(sc_map_del_s64(&map, &arr[i], NULL) == false);
	}

	sc_map_clear_s64(&map);
	sc_map_clear_s64(&map);

	assert(sc_map_put_s64(&map, "h", 0) == true);
	assert(sc_map_put_s64(&map, "z", 0) == true);
	assert(sc_map_get_s64(&map, "13", &val) == false);
	assert(sc_map_get_s64(&map, NULL, &val) == false);
	assert(sc_map_get_s64(&map, "h", &val) == true);
	assert(sc_map_get_s64(&map, "z", &val) == true);
	assert(sc_map_get_s64(&map, "x", &val) == false);
	assert(sc_map_put_s64(&map, NULL, 0) == true);
	assert(sc_map_get_s64(&map, NULL, &val) == true);
	assert(sc_map_del_s64(&map, NULL, &val) == true);
	assert(sc_map_del_s64(&map, "h", &val) == true);
	assert(sc_map_del_s64(&map, "13", &val) == false);
	sc_map_clear_s64(&map);
	assert(sc_map_size_s64(&map) == 0);

	for (int i = 0; i < 5; i++) {
		assert(sc_map_put_s64(&map, &arr[i], 0) == true);
	}
	assert(sc_map_put_s64(&map, &arr[15], 0) == true);
	assert(sc_map_put_s64(&map, &arr[7], 0) == true);
	assert(sc_map_put_s64(&map, &arr[9], 0) == true);

	assert(sc_map_get_s64(&map, &arr[16], &val) == false);

	for (int i = 0; i < 5; i++) {
		assert(sc_map_put_s64(&map, &arr[(5 * i) + i], 0) == true);
	}

	assert(sc_map_del_s64(&map, &arr[4], NULL) == true);
	assert(sc_map_del_s64(&map, &arr[6], NULL) == true);
	assert(sc_map_del_s64(&map, &arr[15], NULL) == true);

	sc_map_clear_s64(&map);
	assert(sc_map_put_s64(&map, "h", 0) == true);
	assert(sc_map_put_s64(&map, "z", 0) == true);
	assert(sc_map_del_s64(&map, "h", 0) == true);
	sc_map_clear_s64(&map);

	assert(sc_map_put_s64(&map, "h", 0) == true);
	assert(sc_map_put_s64(&map, "z", 0) == true);
	assert(sc_map_put_s64(&map, "13", 0) == true);
	assert(sc_map_del_s64(&map, "z", 0) == true);

	sc_map_term_s64(&map);
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

	struct sc_map_str mapstr;
	assert(sc_map_init_str(&mapstr, 0, 26));
	for (int i = 0; i < 64; i++) {
		assert(sc_map_put_str(&mapstr, keys[i],
				      (void *) (uintptr_t) i));
	}
	assert(sc_map_get_str(&mapstr, keys[0], (const char **) &v));
	assert(v == 0);
	assert(sc_map_size_str(&mapstr) == 64);
	assert(sc_map_del_str(&mapstr, keys[12], (const char **) &v));
	assert(v == (void *) 12);
	assert(sc_map_size_str(&mapstr) == 63);
	sc_map_clear_str(&mapstr);
	sc_map_term_str(&mapstr);

	struct sc_map_sv mapsv;
	assert(sc_map_init_sv(&mapsv, 0, 26));
	for (int i = 0; i < 64; i++) {
		assert(sc_map_put_sv(&mapsv, keys[i], (void *) (uintptr_t) i));
	}
	assert(sc_map_get_sv(&mapsv, keys[0], &v));
	assert(v == 0);
	assert(sc_map_size_sv(&mapsv) == 64);
	assert(sc_map_del_sv(&mapsv, keys[12], &v));
	assert(v == (void *) 12);
	assert(sc_map_size_sv(&mapsv) == 63);
	sc_map_clear_sv(&mapsv);
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

static void test6()
{
	const int count = 120000;
	uint32_t val;
	bool b, exists;
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
		b = sc_map_put_32(&map, keys[i], values[i]);
		assert(b);

		if (i % 7 == 0 || i % 17 == 0 || i % 79 == 0) {
			b = sc_map_del_32(&map, keys[i], &val);
			assert(b);
			assert(val == values[i]);
		}
	}

	for (int i = 0; i < count; i++) {
		exists = true;
		if (i % 7 == 0 || i % 17 == 0 || i % 79 == 0) {
			exists = false;
		}

		b = sc_map_get_32(&map, keys[i], &val);
		assert(b == exists);

		if (b) {
			assert(val == values[i]);
		}
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
	bool success = true;
	for (int i = 0; i < 20; i++) {
		success = sc_map_put_32(&map, i, i);
	}
	assert(!success);
	fail_calloc = false;
	assert(sc_map_put_32(&map, 44444, 44444));

	for (size_t i = 0; i < SC_SIZE_MAX; i++) {
		success = sc_map_put_32(&map, i, i);
	}
	assert(!success);
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
	bool success = true;
	for (int i = 0; i < 20; i++) {
		success = sc_map_put_64(&map, i, i);
	}
	assert(!success);
	fail_calloc = false;
	assert(sc_map_put_64(&map, 44444, 44444));

	for (size_t i = 0; i < SC_SIZE_MAX; i++) {
		success = sc_map_put_64(&map, i, i);
	}
	assert(!success);
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
	bool success = true;
	for (int i = 0; i < 20; i++) {
		success = sc_map_put_64v(&map, i, NULL);
	}
	assert(!success);
	fail_calloc = false;
	assert(sc_map_put_64v(&map, 44444, NULL));

	for (size_t i = 0; i < SC_SIZE_MAX; i++) {
		success = sc_map_put_64v(&map, i, NULL);
	}
	assert(!success);
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
	bool success = true;
	for (int i = 0; i < 20; i++) {
		success = sc_map_put_64s(&map, i, NULL);
	}
	assert(!success);
	fail_calloc = false;
	assert(sc_map_put_64s(&map, 44444, NULL));

	for (size_t i = 0; i < SC_SIZE_MAX; i++) {
		success = sc_map_put_64s(&map, i, NULL);
	}
	assert(!success);
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
	bool success = true;
	for (int i = 0; i < 20; i++) {
		success = sc_map_put_str(&map, &s[i], NULL);
	}
	assert(!success);
	fail_calloc = false;
	assert(sc_map_put_str(&map, &s[21], NULL));
	sc_map_clear_str(&map);

	for (size_t i = 0; i < SC_SIZE_MAX; i++) {
		char *c = str_random(32);
		success = sc_map_put_str(&map, c, NULL);
		if (!success) {
			free(c);
			break;
		}
	}
	assert(!success);
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
	bool success = true;
	for (int i = 0; i < 20; i++) {
		success = sc_map_put_sv(&map, &s[i], NULL);
	}
	assert(!success);
	fail_calloc = false;
	assert(sc_map_put_sv(&map, &s[21], NULL));
	sc_map_clear_sv(&map);

	for (size_t i = 0; i < SC_SIZE_MAX; i++) {
		char *c = str_random(32);
		success = sc_map_put_sv(&map, c, NULL);
		if (!success) {
			free(c);
			break;
		}
	}
	assert(!success);
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
	bool success = true;
	for (int i = 0; i < 20; i++) {
		success = sc_map_put_s64(&map, &s[i], 0);
	}
	assert(!success);
	fail_calloc = false;
	assert(sc_map_put_s64(&map, &s[21], 0));
	sc_map_clear_s64(&map);

	for (size_t i = 0; i < SC_SIZE_MAX; i++) {
		char *c = str_random(32);
		success = sc_map_put_s64(&map, c, 0);
		if (!success) {
			free(c);
			break;
		}
	}
	assert(!success);
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
	fail_test_32();
	fail_test_64();
	fail_test_64v();
	fail_test_64s();
	fail_test_str();
	fail_test_sv();
	fail_test_s64();
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

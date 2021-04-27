#include "sc_map.h"

#include <stdio.h>

void example_str(void)
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

void example_int_to_str(void)
{
	uint32_t key;
	const char *elem;
	const char **value;
	struct sc_map_64s map;

	sc_map_init_64s(&map, 0, 0);

	sc_map_put_64s(&map, 100, "chicago");
	sc_map_put_64s(&map, 200, "new york");
	sc_map_put_64s(&map, 300, "atlanta");

	value = sc_map_del_64s(&map, 100);
	if (value != NULL) {
		printf("Deleted : %s \n", *value);
	}

	sc_map_foreach (&map, key, elem) {
		printf("Key:[%d], Value:[%s] \n", key, elem);
	}

	sc_map_term_64s(&map);
}

void example_errcheck(void)
{
	bool ret;
	uint32_t *val;
	struct sc_map_32 map;

	ret = sc_map_init_32(&map, 0, 0);
	if (!ret) {
		perror("Out of memory!");
		abort();
	}

	ret = sc_map_put_32(&map, 0, 0);
	if (!ret) {
		perror("Out of memory!");
		abort();
	}

	val = sc_map_get_32(&map, 0);
	if (val != NULL) {
		// set current value to 3
		*val = 3;
	}

	val = sc_map_get_32(&map, 0);
	if (val != NULL) {
		printf("Read value : %u \n", *val);
	}

	val = sc_map_del_32(&map, 0);
	if (val != NULL) {
		printf("Deleted : %u \n", *val);
	}

	sc_map_term_32(&map);
}

int main()
{
	example_errcheck();
	example_str();
	example_int_to_str();

	return 0;
}

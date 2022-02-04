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

int main()
{
	example_str();
	example_int_to_str();

	return 0;
}

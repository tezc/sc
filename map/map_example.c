#include "sc_map.h"

#include <stdio.h>

int main(int argc, char *argv[])
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

    return 0;
}

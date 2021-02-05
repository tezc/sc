#include "sc_array.h"

#include <stdio.h>

void example_str()
{
    char **p, *it;

    sc_array_create(p, 0);

    sc_array_add(p, "item0");
    sc_array_add(p, "item1");
    sc_array_add(p, "item2");

    printf("\nDelete first element \n\n");
    sc_array_del(p, 0);

    sc_array_foreach (p, it) {
        printf("Elem = %s \n", it);
    }

    sc_array_destroy(p);
}

void example_int()
{
    int *p;

    sc_array_create(p, 0);

    sc_array_add(p, 0);
    sc_array_add(p, 1);
    sc_array_add(p, 2);

    for (int i = 0; i < sc_array_size(p); i++) {
        printf("Elem = %d \n", p[i]);
    }

    sc_array_destroy(p);
}

int main(int argc, char *argv[])
{
    example_int();
    example_str();

    return 0;
}

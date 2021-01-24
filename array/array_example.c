#include "sc_array.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
    int *p;
    int val;

    sc_array_create(p, 0);

    sc_array_add(p, 0);
    sc_array_add(p, 1);
    sc_array_add(p, 3);

    printf("\nRemoving first element \n\n");
    sc_array_remove(p, 0);

    printf("Capacity %zu \n", sc_array_cap(p));
    printf("Element count %zu \n", sc_array_size(p));


    // Simple loop
    for (int i = 0; i < sc_array_size(p); i++) {
        printf("Elem = %d \n", p[i]);
    }

    sc_array_foreach(p, val) {
        printf("Elem = %d \n", val);
    }

    sc_array_destroy(p);

    return 0;
}

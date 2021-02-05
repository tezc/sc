# Generic array

#### Overview

- Type generic array/vector.
- Index access is possible (e.g float* arr; 'printf("%f", arr[i]')).


##### Usage


```c
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
```

##### Note

Array pointer is not stable. If you pass the array to another function which  
can add items, do it by passing reference of the array pointer :

```c
void some_function_to_add_elems(long **p)
{
    sc_array_add(*p, 500);
}

int main(int argc, char *argv[])
{
    long *p;

    sc_array_create(p, 0);
    sc_array_add(p, 300);
    
    // Pass via address of p
    some_function_to_add_elems(&p);
    sc_array_destroy(p);
}

```
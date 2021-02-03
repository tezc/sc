# Generic array

#### Overview

- Generic array which grows when you add elements.
- Index access is possible (e.g float* arr; 'printf("%f", arr[i]')).
- Lazy allocation. No memory allocation until first 'add'.


##### Usage


```c

    int *p;
    int val;

    sc_array_create(p, 0);

    sc_array_add(p, 0);
    sc_array_add(p, 1);
    sc_array_add(p, 3);
    
    printf("\nDelete first element \n\n");
    sc_array_del(p, 0);

    printf("Capacity %zu \n", sc_array_cap(p));
    printf("Element count %zu \n", sc_array_size(p));


    // Simple loop
    for (int i = 0; i < sc_array_size(p); i++) {
        printf("Elem = %d \n", p[i]);
    }
    
    // Foreach
    sc_array_foreach(p, val) {
        printf("Elem = %d \n", val);
    }

    sc_array_destroy(p);

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
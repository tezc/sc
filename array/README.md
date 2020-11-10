# Generic array

#### Overview

- Type generic array which grows when you add elements.
- Index access is possible (e.g float* arr; 'printf("%f", arr[i]')).
- Just copy <b>sc_array.h</b> and <b>sc_array.c</b> to your project.


##### Usage


```c
    int *p;

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

    sc_array_destroy(p);
```
#### Internals

##### Memory
- Single allocation for all the data.
- Lazy allocation. No memory allocation until first 'add'. 

##### Performance
- As all the items are in a single contiguous memory, it gives the best  
performance you can expect.  

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
    
    some_function_to_add_elems(&p);
    sc_array_destroy(p);
}

```

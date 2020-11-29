#include "sc_array.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

int example()
{
    int *p;

    sc_array_create(p, 0);

    sc_array_add(p, 0);
    sc_array_add(p, 1);
    sc_array_add(p, 3);

    printf("\nRemoving first element \n\n");
    sc_array_remove(p, 0);

    printf("Capacity %zu \n", sc_array_cap(p));
    printf("Element count %zu \n", sc_array_size(p));

    for (int i = 0; i < sc_array_size(p); i++) {
        printf("Elem = %d \n", p[i]);
    }

    sc_array_destroy(p);

    return 0;
}

static int compare(const void *a, const void *b)
{
    const int *x = a;
    const int *y = b;

    return *x - *y;
}

static void test1(void)
{
    int *arr, total = 0;

    sc_array_create(arr, 5);
    sc_array_add(arr, 3);
    sc_array_add(arr, 4);
    sc_array_add(arr, 5);

    assert(sc_array_size(arr) == 3);

    sc_array_remove(arr, 0);
    assert(arr[0] == 4);
    sc_array_remove_last(arr);
    assert(arr[0] == 4);

    sc_array_add(arr, 1);
    sc_array_add(arr, 3);
    sc_array_add(arr, 2);
    sc_array_add(arr, 0);

    assert(sc_array_last(arr) == 0);

    sc_array_sort(arr, compare);

    for (int i = 0; i < sc_array_size(arr); i++) {
        total += arr[i];
    }

    assert(total == 10);

    for (int i = 0; i < sc_array_size(arr); i++) {
        assert(arr[i] == i);
    }

    sc_array_destroy(arr);
}

#ifdef SC_HAVE_WRAP

bool fail_realloc = false;
void *__real_realloc(void *p, size_t size);
void *__wrap_realloc(void *p, size_t n)
{
    if (fail_realloc) {
        return NULL;
    }

    return __real_realloc(p, n);
}

void fail_test()
{
    int tmp;
    int *arr, total = 0;

    assert(sc_array_create(arr, SIZE_MAX) == false);
    assert(arr == NULL);
    assert(sc_array_create(arr, 0) == true);
    assert(arr != NULL);
    sc_array_destroy(arr);
    assert(arr == NULL);
    assert(sc_array_create(arr, 0) == true);

    size_t count = SC_SIZE_MAX / sizeof(*arr);
    bool success = false;

    for (int i = 0; i < count + 5; i++) {
        success = sc_array_add(arr, i);
    }

    assert(!success);

    sc_array_destroy(arr);
    sc_array_create(arr, 0);
    assert(sc_array_size(arr) == 0);

    fail_realloc = true;
    success = sc_array_add(arr, 0);
    assert(!success);

    fail_realloc = false;
    success = sc_array_add(arr, 222);
    assert(success);
    sc_array_destroy(arr);

    fail_realloc = true;
    assert(sc_array_create(arr, 222) == false);
    fail_realloc = false;

    assert(sc_array_create(arr, 0) == true);
    fail_realloc = true;
    success = sc_array_add(arr, 222);
    assert(!success);
    fail_realloc = false;

    sc_array_add(arr, 3);
    sc_array_add(arr, 4);
    sc_array_add(arr, 5);

    assert(sc_array_size(arr) == 3);

    sc_array_remove(arr, 0);
    assert(arr[0] == 4);
    sc_array_remove_last(arr);
    assert(arr[0] == 4);

    sc_array_add(arr, 1);
    sc_array_add(arr, 3);
    sc_array_add(arr, 2);
    sc_array_add(arr, 0);

    sc_array_sort(arr, compare);

    for (int i = 0; i < sc_array_size(arr); i++) {
        total += arr[i];
    }

    assert(total == 10);

    for (int i = 0; i < sc_array_size(arr); i++) {
        assert(arr[i] == i);
    }

    total = 0;
    sc_array_foreach(arr, tmp) {
       total += tmp;
    }
    assert(total == 10);

    sc_array_destroy(arr);
}
#else
void fail_test(void)
{
}
#endif

int main(int argc, char *argv[])
{
    example();
    test1();
    fail_test();
}

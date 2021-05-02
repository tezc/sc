#include "sc_array.h"

#include <stdio.h>

void example_str()
{
	const char *it;
	struct sc_array_str arr;

	sc_array_init(&arr);

	sc_array_add(&arr, "item0");
	sc_array_add(&arr, "item1");
	sc_array_add(&arr, "item2");

	printf("\nDelete first element \n\n");
	sc_array_del(&arr, 0);

	sc_array_foreach (&arr, it) {
		printf("Elem = %s \n", it);
	}

	sc_array_term(&arr);
}

void example_int()
{
	struct sc_array_int arr;

	sc_array_init(&arr);

	sc_array_add(&arr, 0);
	sc_array_add(&arr, 1);
	sc_array_add(&arr, 2);

	for (size_t i = 0; i < sc_array_size(&arr); i++) {
		printf("Elem = %d \n", arr.elems[i]);
	}

	sc_array_term(&arr);
}

int main()
{
	example_int();
	example_str();

	return 0;
}

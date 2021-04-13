#include "sc_str.h"

#include <stdio.h>

int main()
{
	char *s1;

	s1 = sc_str_create("*-hello-*");
	printf("%s \n", s1); // prints **hello**

	sc_str_trim(&s1, "*-");
	printf("%s \n", s1); // prints hello

	sc_str_append_fmt(&s1, "%d", 2);
	printf("%s \n", s1); // prints hello2

	sc_str_replace(&s1, "2", " world!");
	printf("%s \n", s1); // prints hello world!

	sc_str_substring(&s1, 0, 5);
	printf("%s \n", s1); // prints hello

	sc_str_destroy(&s1);

	return 0;
}

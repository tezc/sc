#include "sc_thread.h"

#include <stdio.h>

void *fn(void *arg)
{
	printf("%s \n", (char *) arg);
	return arg;
}

int main()
{
	struct sc_thread thread;

	sc_thread_init(&thread);
	sc_thread_start(&thread, fn, "first");
	sc_thread_term(&thread);

	return 0;
}

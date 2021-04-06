#include "sc_cond.h"

#include <stdio.h>

int main()
{
	struct sc_cond cond;

	sc_cond_init(&cond); // Init once

	sc_cond_signal(&cond, "test"); // Call this on thread-1
	char *p = sc_cond_wait(&cond); // Call this on another thread.

	printf("%s \n", p); // Prints "test"

	sc_cond_term(&cond); // Destroy

	return 0;
}

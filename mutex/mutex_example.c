#include "sc_mutex.h"

int main()
{
	struct sc_mutex mutex;

	sc_mutex_init(&mutex);

	sc_mutex_lock(&mutex);
	sc_mutex_unlock(&mutex);

	sc_mutex_term(&mutex);

	return 0;
}

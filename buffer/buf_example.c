#include "sc_buf.h"

#include <stdio.h>

void basic()
{
	struct sc_buf buf;
	sc_buf_init(&buf, 1024);

	sc_buf_put_32(&buf, 16);
	sc_buf_put_str(&buf, "test");
	sc_buf_put_fmt(&buf, "value is %d", 3);

	printf("%d \n", sc_buf_get_32(&buf));
	printf("%s \n", sc_buf_get_str(&buf));
	printf("%s \n", sc_buf_get_str(&buf));

	sc_buf_term(&buf);
}

void error_check()
{
	uint32_t val, val2;
	struct sc_buf buf;

	sc_buf_init(&buf, 1024);
	sc_buf_put_32(&buf, 16);

	val = sc_buf_get_32(&buf);
	val2 = sc_buf_get_32(&buf); // This will set error flag in buffer;

	if (sc_buf_valid(&buf) == false) {
		printf("buffer is corrupt");
		exit(EXIT_FAILURE);
	}

	printf("%u %u \n", val, val2);
	sc_buf_term(&buf);
}

int main()
{
	basic();
	error_check();

	return 0;
}

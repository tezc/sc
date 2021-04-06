#include "sc_crc32.h"

#include <stdio.h>

int main()
{
	uint32_t crc;
	const uint8_t buf[100] = {0};

	sc_crc32_init();

	// Partial calculation example
	crc = sc_crc32(0, buf, 10);
	crc = sc_crc32(crc, buf + 10, sizeof(buf) - 10);
	printf("crc : %u \n", crc);

	// Calculate at once
	crc = sc_crc32(0, buf, sizeof(buf));
	printf("crc : %u \n", crc);

	return 0;
}

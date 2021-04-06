#include "sc_ini.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

const char *example_ini = "# My configuration"
			  "[Network] \n"
			  "hostname = github.com \n"
			  "port = 443 \n"
			  "protocol = https \n"
			  "repo = any";

int callback(void *arg, int line, const char *section, const char *key,
	     const char *value)
{
	(void) arg;

	printf("Line : %d, Section : %s, Key : %s, Value : %s \n", line,
	       section, key, value);

	return 0;
}

void file_example(void)
{
	int rc;

	FILE *fp = fopen("my_config.ini", "w+");
	fwrite(example_ini, 1, strlen(example_ini), fp);
	fclose(fp);

	printf(" \n Parse file \n");

	rc = sc_ini_parse_file(NULL, callback, "my_config.ini");
	assert(rc == 0);
}

void string_example(void)
{
	int rc;

	printf(" \n Parse string \n");

	rc = sc_ini_parse_string(NULL, callback, example_ini);
	assert(rc == 0);
}

int main()
{
	string_example();
	file_example();
}

#include "sc_option.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>

static struct sc_option_item options[] = {{.letter = 's', .name = NULL},
					  {.letter = 'm', .name = NULL},
					  {.letter = 'k', .name = "key"},
					  {.letter = 'h', .name = "help"}};

void test1()
{
	char *value;
	int argc = 5;
	char *argv[] = {"program", "-m", "-s", "--key=value", "--help"};

	struct sc_option opt = {.argv = argv,
				.count = sizeof(options) /
					 sizeof(struct sc_option_item),
				.options = options};

	for (int i = 1; i < argc; i++) {
		char c = sc_option_at(&opt, i, &value);
		switch (c) {
		case 's':
			assert(*value == '\0');
			break;
		case 'm':
			assert(*value == '\0');
			break;
		case 'k':
			assert(strcmp(value, "value") == 0);
			break;
		case 'h':
			assert(*value == '\0');
			break;
		case '?':
		default:
			assert(false);
			break;
		}
	}
}

void test2()
{
	char *value;
	int argc = 2;
	char *argv[] = {"program", "-j"};

	struct sc_option opt = {.argv = argv,
				.count = sizeof(options) /
					 sizeof(struct sc_option_item),
				.options = options};

	for (int i = 1; i < argc; i++) {
		char c = sc_option_at(&opt, i, &value);
		switch (c) {
		case '?':
			break;
		default:
			assert(false);
			break;
		}
	}
}

void test3()
{
	char *value;
	int argc = 2;
	char *argv[] = {"program", "--key=value"};

	struct sc_option opt = {.argv = argv,
				.count = sizeof(options) /
					 sizeof(struct sc_option_item),
				.options = options};

	for (int i = 1; i < argc; i++) {
		char c = sc_option_at(&opt, i, &value);
		switch (c) {
		case 'k':
			assert(strcmp(value, "value") == 0);
			break;
		default:
			assert(false);
			break;
		}
	}
}

void test4()
{
	char *value;

	int argc = 2;
	char *argv[] = {"program", "key=value"};

	struct sc_option opt = {.argv = argv,
				.count = sizeof(options) /
					 sizeof(struct sc_option_item),
				.options = options};

	for (int i = 1; i < argc; i++) {
		char c = sc_option_at(&opt, i, &value);
		switch (c) {
		case '?':
			break;
		default:
			assert(false);
			break;
		}
	}
}

static struct sc_option_item options2[] = {{.letter = 's', .name = "sadsa"},
					   {.letter = 'm', .name = "sad"},
					   {.letter = 'k', .name = "key3"},
					   {.letter = 'h', .name = "dsadsa"}};
void test5()
{
	char *value;

	int argc = 2;
	char *argv[] = {"program", "--key=value"};

	struct sc_option opt = {.argv = argv,
				.count = sizeof(options2) /
					 sizeof(struct sc_option_item),
				.options = options2};

	for (int i = 1; i < argc; i++) {
		char c = sc_option_at(&opt, i, &value);
		switch (c) {
		case '?':
			break;
		default:
			assert(false);
			break;
		}
	}
}

void test6()
{
	char *value;
	int argc = 2;
	char *argv[] = {"program", "-s"};

	struct sc_option opt = {.argv = argv,
				.count = sizeof(options) /
					 sizeof(struct sc_option_item),
				.options = options};

	for (int i = 1; i < argc; i++) {
		char c = sc_option_at(&opt, i, &value);
		switch (c) {
		case 's':
			break;
		default:
			assert(false);
			break;
		}
	}
}

void test7()
{
	char *value;
	int argc = 2;
	char *argv[] = {"program", "-j"};

	struct sc_option opt = {.argv = argv,
				.count = sizeof(options) /
					 sizeof(struct sc_option_item),
				.options = options};

	for (int i = 1; i < argc; i++) {
		char c = sc_option_at(&opt, i, &value);
		switch (c) {
		case '?':
			break;
		default:
			assert(false);
			break;
		}
	}
}

void test8()
{
	char *value;
	int argc = 2;
	char *argv[] = {"program", "-sx"};

	struct sc_option opt = {.argv = argv,
				.count = sizeof(options) /
					 sizeof(struct sc_option_item),
				.options = options};

	for (int i = 1; i < argc; i++) {
		char c = sc_option_at(&opt, i, &value);
		switch (c) {
		case '?':
			break;
		default:
			assert(false);
			break;
		}
	}
}

int main()
{
	test1();
	test2();
	test3();
	test4();
	test5();
	test6();
	test7();
	test8();

	return 0;
}

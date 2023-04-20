#include "sc_option.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>

static struct sc_option_item options[] = {{.letter = 's', .name = NULL},
					  {.letter = 'm', .name = NULL},
					  {.letter = 'k', .name = "key"},
					  {.letter = 'h', .name = "help"}};

void test1(void)
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

void test2(void)
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

void test3(void)
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

void test4(void)
{
	char *value;
	char *argv[] = {"program", "key=value"};

	struct sc_option opt = {.argv = argv,
				.count = sizeof(options) /
					 sizeof(struct sc_option_item),
				.options = options};

	char c = sc_option_at(&opt, 1, &value);
	assert(c == '?');
}

static struct sc_option_item options2[] = {{.letter = 's', .name = "sadsa"},
					   {.letter = 'm', .name = "sad"},
					   {.letter = 'k', .name = "key3"},
					   {.letter = 'h', .name = "dsadsa"}};
void test5(void)
{
	char *value;
	char *argv[] = {"program", "--key=value"};

	struct sc_option opt = {.argv = argv,
				.count = sizeof(options2) /
					 sizeof(struct sc_option_item),
				.options = options2};

	char c = sc_option_at(&opt, 1, &value);
	assert(c == '?');
}

void test6(void)
{
	char *value;
	char *argv[] = {"program", "-s"};

	struct sc_option opt = {.argv = argv,
				.count = sizeof(options) /
					 sizeof(struct sc_option_item),
				.options = options};

	char c = sc_option_at(&opt, 1, &value);
	assert(c == 's');
}

void test7(void)
{
	char *value;
	char *argv[] = {"program", "-j"};

	struct sc_option opt = {.argv = argv,
				.count = sizeof(options) /
					 sizeof(struct sc_option_item),
				.options = options};

	char c = sc_option_at(&opt, 1, &value);
	assert(c == '?');
}

void test8(void)
{
	char *value;
	char *argv[] = {"program", "-sx"};

	struct sc_option opt = {.argv = argv,
				.count = sizeof(options) /
					 sizeof(struct sc_option_item),
				.options = options};

	char c = sc_option_at(&opt, 1, &value);
	assert(c == '?');
}

int main(void)
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

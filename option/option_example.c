#include "sc_option.h"

static struct sc_option_item options[] = {{.letter = 'm', .name = NULL},
					  {.letter = 'k', .name = "key"},
					  {.letter = 'h', .name = "help"}};

int main(int argc, char *argv[])
{
	char *value;

	struct sc_option opt = {.argv = argv,
				.count = sizeof(options) / sizeof(options[0]),
				.options = options};

	for (int i = 1; i < argc; i++) {
		char c = sc_option_at(&opt, i, &value);
		switch (c) {
		case 'm':
			// If value does not exist, it will point to '\0'
			// character.
			printf("Option 'm', value : %s \n", value);
			break;
		case 'k':
			printf("Option 'k', value : %s \n", value);
			break;
		case 'h':
			printf("Option 'h', value : %s \n", value);
			break;
		case '?':
			printf("Unknown option : %s \n", argv[i]);
			break;
		}
	}

	return 0;
}

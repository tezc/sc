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
                         .count = sizeof(options) / sizeof(struct sc_option_item),
                         .items = options};

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
                         .count = sizeof(options) / sizeof(struct sc_option_item),
                         .items = options};

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
                         .count = sizeof(options) / sizeof(struct sc_option_item),
                         .items = options};

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
                         .count = sizeof(options) / sizeof(struct sc_option_item),
                         .items = options};

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

int main(int argc, char *argv[])
{
    test1();
    test2();
    test3();
    test4();

    return 0;
}

#include "sc_signal.h"

#include <assert.h>
#include <string.h>

void test1()
{
    char tmp[128] = "";

    sc_signal_snprintf(tmp, 0, "%s", "test");
    assert(strcmp(tmp, "") == 0);

    sc_signal_snprintf(tmp, sizeof(tmp), "%s", "test");
    assert(strcmp(tmp, "test") == 0);
    sc_signal_snprintf(tmp, sizeof(tmp), "%s", NULL);
    assert(strcmp(tmp, "(null)") == 0);
    sc_signal_snprintf(tmp, sizeof(tmp), "%d", -3);
    assert(strcmp(tmp, "-3") == 0);
    sc_signal_snprintf(tmp, sizeof(tmp), "%u", 3);
    assert(strcmp(tmp, "3") == 0);
    sc_signal_snprintf(tmp, sizeof(tmp), "%ld", -1000000000l);
    assert(strcmp(tmp, "-1000000000") == 0);
    sc_signal_snprintf(tmp, sizeof(tmp), "%lld", -100000000000ll);
    assert(strcmp(tmp, "-100000000000") == 0);
    sc_signal_snprintf(tmp, sizeof(tmp), "%lu", 1000000000l);
    assert(strcmp(tmp, "1000000000") == 0);
    sc_signal_snprintf(tmp, sizeof(tmp), "%llu", 100000000000ll);
    assert(strcmp(tmp, "100000000000") == 0);

    char *x = (char *) 0xabcdef;
    sc_signal_snprintf(tmp, sizeof(tmp), "%p", x);
    assert(strcmp(tmp, "0xabcdef") == 0);

    sc_signal_snprintf(tmp, sizeof(tmp), "%%p", x);
    assert(strcmp(tmp, "%p") == 0);

    assert(sc_signal_snprintf(tmp, sizeof(tmp), "%c", 3) == -1);
    assert(sc_signal_snprintf(tmp, sizeof(tmp), "%llx", 3) == -1);
    assert(sc_signal_snprintf(tmp, sizeof(tmp), "%lx", 3) == -1);

    sc_signal_log(1, tmp, sizeof(tmp), "%s", "test");
}

void test2()
{
    assert(sc_signal_init() == 0);
}

#ifdef SC_HAVE_WRAP
    #include <stdbool.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <wait.h>

void sig_handler(int signum)
{
}

void test3(int argc, char **argv)
{


    pid_t pid = fork();
    if (pid == -1) {
        assert(true);
    } else if (pid) {
        int status = 0;
        wait(&status);
        if (WIFSIGNALED(status)) {
            return;
        } else {
            assert(true);
        }
    } else {
        execvp(argv[1], argv + 1);
        exit(1);
    }
}

void test4()
{
    assert(sc_signal_init() == 0);
    sc_signal_shutdown_fd = STDOUT_FILENO;
    raise(SIGINT);
    sleep(3);
}

#else
void test3((int argc, char **argv)
{
}
void test4()
{
}
#endif

int main(int argc, char **argv)
{
    test1();
    test2();
    test3(argc, argv);
    test4();

    return 0;
}

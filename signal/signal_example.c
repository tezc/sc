#include "sc_signal.h"

#include <stdio.h>
#include <unistd.h>

int main()
{
    char tmp[1];
    int fds[2];

    sc_signal_init();

    pipe(fds);

    sc_signal_shutdown_fd = fds[1];

    read(fds[0], tmp, sizeof(tmp));
    printf("Received shutdown signal \n");


    return 0;
}


#include "sc_sock.h"

#include <assert.h>
#include <stdlib.h>

void test()
{
    struct sc_sock sock;
    sc_sock_init(&sock, 0, true, AF_INET);
    int rc = sc_sock_connect(&sock, "127.0.0.1", "8080", NULL, NULL);
    assert(rc == -1);
    assert(sc_sock_term(&sock) == 0);

}



int main()
{
    test();
    return 0;
}

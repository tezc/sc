#include "sc_uri.h"

#include <stdio.h>

int main()
{
	struct sc_uri *uri;
	uri = sc_uri_create("http://user:pass@any.com:8042/over/"
			    "there?name=jane#doe");
	printf("%s \n", uri->str);      // prints "http://user:pass@any.com:8042/over/there?name=jane#doe"
	printf("%s \n", uri->scheme);	// prints "http"
	printf("%s \n", uri->host);	// prints "any.com"
	printf("%s \n", uri->userinfo); // prints "user:pass"
	printf("%s \n", uri->port);	// prints "8042"
	printf("%s \n", uri->path);	// prints "/over/there"
	printf("%s \n", uri->query);	// prints "name=jane"
	printf("%s \n", uri->fragment); // prints "doe"

	sc_uri_destroy(&uri);

	return 0;
}

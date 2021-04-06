#include "sc_ini.h"

#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable : 4996)
#endif

int cb(void *arg, int line, const char *section, const char *key,
       const char *value)
{
	(void) arg;
	(void) line;

	printf("%s %s %s \n", section, key, value);
	return 0;
}

void test1()
{
	int rc;
	static const char *ini = "#Sample \n"
				 "[section \n";

	rc = sc_ini_parse_string(NULL, cb, ini);
	assert(rc == 2);
}

int cb2(void *arg, int line, const char *section, const char *key,
	const char *value)
{
	(void) arg;
	(void) line;

	assert(strcmp(section, "section") == 0);
	assert(strcmp(key, "key") == 0);
	assert(strcmp(value, "value") == 0);
	return 0;
}

void test2()
{
	int rc;
	int count = 0;

	static const char *ini = "#Sample \n"
				 "[section] \n"
				 "key = value \n"
				 "key : value ";

	rc = sc_ini_parse_string(&count, cb2, ini);
	assert(rc == 0);
}

int cb3(void *arg, int line, const char *section, const char *key,
	const char *value)
{
	(void) line;

	assert(strcmp(section, "") == 0);
	assert(strcmp(key, "key") == 0);
	assert(strcmp(value, "value") == 0);

	*(int *) arg = *(int *) arg + 1;

	return 0;
}

void test3()
{
	int rc;
	int count = 0;

	static const char *ini = " ;Sample \n"
				 "key = value \n"
				 "key : value ";

	rc = sc_ini_parse_string(&count, cb3, ini);
	assert(rc == 0);
	assert(count == 2);
}

int cb4(void *arg, int line, const char *section, const char *key,
	const char *value)
{
	char tmp[16];
	int count = *(int *) arg;

	(void) line;

	assert(strcmp(section, "section") == 0);
	assert(strcmp(key, "key") == 0);

	snprintf(tmp, 16, "value%d", count);
	assert(strcmp(value, tmp) == 0);

	*(int *) arg = count + 1;

	return 0;
}

void test4()
{
	int rc;
	int count = 0;

	static const char *ini = " ;Sample \n"
				 " [section] \n"
				 "key = value0 \n"
				 "      value1 \n"
				 "      value2 ";

	rc = sc_ini_parse_string(&count, cb4, ini);
	assert(rc == 0);
	assert(count == 3);

	rc = sc_ini_parse_string(&count, cb4, NULL);
	assert(rc == 0);
}

int cb5(int line, void *arg, const char *section, const char *key,
	const char *value)
{
	char tmp[16];
	int count = *(int *) arg;

	(void) line;

	assert(strcmp(section, "section") == 0);
	assert(strcmp(key, "key") == 0);

	snprintf(tmp, 16, "value%d", count);
	assert(strcmp(value, tmp) == 0);

	*(int *) arg = count + 1;

	return 0;
}

void test5()
{
	int rc;
	int count = 0;

	static const char *ini = " ;Sample \n"
				 " [section] \n"
				 "key = value0 \n"
				 "value1 \n"
				 "value2 ";

	rc = sc_ini_parse_string(&count, cb4, ini);
	assert(rc == 4);
	assert(count == 1);

	count = 0;
	static const char *ini2 = " ;Sample \n"
				  " [section] \n"
				  "key = value0 \n"
				  " value1 \n"
				  "value2 ";
	rc = sc_ini_parse_string(&count, cb4, ini2);
	assert(rc == 5);
	assert(count == 2);
}

int cb6(void *arg, int line, const char *section, const char *key,
	const char *value)
{
	(void) arg;
	(void) line;
	(void) section;
	(void) key;
	(void) value;

	return -1;
}

void test6()
{
	int rc;
	int count = 0;

	static const char *ini = " ;Sample \n"
				 " [section] \n"
				 "key = value0 \n"
				 "      value1 \n"
				 "      value2 ";

	rc = sc_ini_parse_string(&count, cb6, ini);
	assert(rc == 3);
	assert(count == 0);
}

int cb7(void *arg, int line, const char *section, const char *key,
	const char *value)
{
	(void) line;

	char tmp[16];
	int count = *(int *) arg;
	if (count == 1) {
		return -1;
	}

	assert(strcmp(section, "section") == 0);
	assert(strcmp(key, "key") == 0);

	snprintf(tmp, 16, "value%d", count);
	assert(strcmp(value, tmp) == 0);

	*(int *) arg = count + 1;

	return 0;
}

void test7()
{
	int rc;
	int count = 0;

	static const char *ini = " ;Sample \n"
				 " [section] \n"
				 "key = value0 \n"
				 "      value1 \n"
				 "      value2 ";

	rc = sc_ini_parse_string(&count, cb7, ini);
	assert(rc == 4);
	assert(count == 1);
}

int cb8(void *arg, int line, const char *section, const char *key,
	const char *value)
{
	(void) line;
	char tmp[16];
	int count = *(int *) arg;

	assert(strcmp(section, "section") == 0);
	assert(strcmp(key, "key") == 0);

	snprintf(tmp, 16, "value%d", count);
	assert(strcmp(value, tmp) == 0);

	*(int *) arg = count + 1;

	return 0;
}

void test8()
{
	int rc;
	int count = 0;
	FILE *fp;

	static const char *ini = " ;Sample \n"
				 " [section] \n"
				 "key = value0 #;comment\n"
				 "      value1 \n"
				 "      value2 ";

	fp = fopen("config.ini", "w+");
	fwrite(ini, 1, strlen(ini), fp);
	fclose(fp);

	rc = sc_ini_parse_file(&count, cb8, "config.ini");
	assert(rc == 0);
	assert(count == 3);
	remove("config.ini");

	count = 0;
	static const char *ini2 = " ;Sample \n"
				  " [section] \n"
				  "key = value0 ;comment\n"
				  "      value1 #comment\n"
				  "      value2 ;#comment\n";

	fp = fopen("config.ini", "w+");
	fwrite(ini2, 1, strlen(ini2), fp);
	fclose(fp);

	rc = sc_ini_parse_file(&count, cb8, "config.ini");
	assert(rc == 0);
	assert(count == 3);
	remove("config.ini");

	count = 0;
	unsigned char bom[3] = {0xEF, 0xBB, 0xBF};
	static const char *ini3 = " ;Sample \n"
				  " [section] \n"
				  "key = value0 \n"
				  "      value1 \n"
				  "      value2 \n";

	fp = fopen("config.ini", "w+");
	fwrite(bom, 1, sizeof(bom), fp);
	fwrite(ini3, 1, strlen(ini3), fp);
	fclose(fp);

	rc = sc_ini_parse_file(&count, cb8, "config.ini");
	assert(rc == 0);
	assert(count == 3);
	remove("config.ini");

	unsigned char boms[2] = {0xE3, 0xBB};
	fp = fopen("config.ini", "w+");
	fwrite(boms, 1, sizeof(boms), fp);
	fclose(fp);
	rc = sc_ini_parse_file(&count, cb8, "config.ini");
	assert(rc != 0);
	remove("config.ini");

	unsigned char bom0[3] = {0xE3, 0xBB, 0xBF};
	fp = fopen("config.ini", "w+");
	fwrite(bom0, 1, sizeof(bom0), fp);
	fwrite(ini3, 1, strlen(ini3), fp);
	fclose(fp);
	rc = sc_ini_parse_file(&count, cb8, "config.ini");
	assert(rc != 0);
	remove("config.ini");

	unsigned char bom2[3] = {0xEF, 0xB3, 0xBF};
	fp = fopen("config.ini", "w+");
	fwrite(bom2, 1, sizeof(bom2), fp);
	fwrite(ini3, 1, strlen(ini3), fp);
	fclose(fp);
	rc = sc_ini_parse_file(&count, cb8, "config.ini");
	assert(rc != 0);
	remove("config.ini");

	unsigned char bom3[3] = {0xEF, 0xBB, 0xB3};
	fp = fopen("config.ini", "w+");
	fwrite(bom3, 1, sizeof(bom3), fp);
	fwrite(ini3, 1, strlen(ini3), fp);
	fclose(fp);
	rc = sc_ini_parse_file(&count, cb8, "config.ini");
	assert(rc != 0);
	remove("config.ini");

	rc = sc_ini_parse_file(&count, cb8, "config.ini");
	assert(rc == -1);
}

int cb9(void *arg, int line, const char *section, const char *key,
	const char *value)
{
	(void) arg;
	(void) line;

	assert(strcmp(section, "section[test") == 0);
	assert(strcmp(key, "key;;") == 0);
	assert(strcmp(value, "value;;") == 0);
	return 0;
}

void test9()
{
	int rc;
	int count = 0;

	static const char *ini = "#Sample \n"
				 "[section[test]] \n"
				 "key;; = value;; ;comment \n"
				 "key;; : value;; #comment ";

	rc = sc_ini_parse_string(&count, cb9, ini);
	assert(rc == 0);
}

void test10()
{
	int rc;
	int count = 0;

	static const char *ini = "#Sample \n"
				 "section[test]] \n"
				 "key = value;; ;comment \n"
				 "key : value# #comment ";

	rc = sc_ini_parse_string(&count, cb9, ini);
	assert(rc == 2);
}

int cb11(void *arg, int line, const char *section, const char *key,
	 const char *value)
{
	(void) arg;
	(void) line;

	assert(strcmp(section, "section") == 0);
	assert(strcmp(key, "key#") == 0);
	assert(strcmp(value, "") == 0);
	return 0;
}

void test11()
{
	int rc;
	int count = 0;

	static const char *ini = "#Sample \n"
				 "[section] \n"
				 "#comment1 \n"
				 ";comment2 \n"
				 "   #comment3 \n"
				 "  ;comment44 \n"
				 "key#  =  ;comment \n"
				 "key#  :  #comment ";

	rc = sc_ini_parse_string(&count, cb11, ini);
	assert(rc == 0);
}

int cb12(void *arg, int line, const char *section, const char *key,
	 const char *value)
{
	(void) arg;
	(void) line;

	assert(strcmp(section, "") == 0);
	assert(strcmp(key, "") == 0);
	assert(strcmp(value, "") == 0);
	return 0;
}

void test12()
{
	int rc;
	int count = 0;

	static const char *ini = "#Sample \n"
				 "#[section] \n"
				 "#comment1 \n"
				 ";comment2 \n"
				 "   #comment3 \n"
				 "  ;comment44 \n"
				 "  =  ;comment \n"
				 "  :  #comment ";

	rc = sc_ini_parse_string(&count, cb12, ini);
	assert(rc == 0);
}

int cb13(void *arg, int line, const char *section, const char *key,
	 const char *value)
{
	(void) line;
	char tmp[16];
	int count = *(int *) arg;

	assert(strcmp(section, "section") == 0);
	assert(strcmp(key, "key") == 0);

	snprintf(tmp, 16, "value%d", count);
	assert(strcmp(value, tmp) == 0);

	*(int *) arg = count + 1;

	return 0;
}

void test13()
{
	int rc;
	int count = 0;

	static const char *ini = " ;Sample \n"
				 " [section] \n"
				 "key = value0 \n"
				 "             \n"
				 "      value1 ";

	rc = sc_ini_parse_string(&count, cb13, ini);
	assert(rc == 0);
	assert(count == 2);
}

int cb14(void *arg, int line, const char *section, const char *key,
	 const char *value)
{
	(void) arg;
	(void) line;

	assert(strcmp(section, "section") == 0);
	assert(strcmp(key, "key") == 0);
	assert(strcmp(value, "value") == 0);
	return -1;
}

void test14()
{
	int rc;
	int count = 0;

	static const char *ini = "#Sample \n"
				 "[section] \n"
				 "key = value \n"
				 "key : value ";

	rc = sc_ini_parse_string(&count, cb14, ini);
	assert(rc != 0);
}

int cb15(void *arg, int line, const char *section, const char *key,
	 const char *value)
{
	(void) line;
	(void) arg;
	(void) section;
	(void) key;
	(void) value;

	return -1;
}

void test15()
{
	int rc;
	int count = 0;
	FILE *fp;

	static const char *ini = " ;Sample \n"
				 " [section] \n"
				 "key = value0 #;comment\n"
				 "      value1 \n"
				 "      value2 ";

	fp = fopen("config.ini", "w+");
	fwrite(ini, 1, strlen(ini), fp);
	fclose(fp);

	rc = sc_ini_parse_file(&count, cb15, "config.ini");
	assert(rc != 0);
	remove("config.ini");
}

int cb16(void *arg, int line, const char *section, const char *key,
	 const char *value)
{
	(void) arg;
	(void) line;
	(void) section;
	(void) key;
	(void) value;

	return 0;
}

void test16()
{
	int rc;

	rc = sc_ini_parse_string(NULL, cb16, "key#  =  ;comment \n");
	assert(rc == 0);
	rc = sc_ini_parse_string(NULL, cb16, "key#  =  ;comment \n\n");
	assert(rc == 0);
	rc = sc_ini_parse_string(NULL, cb16, "key#  =  ;comment \nx=3\n");
	assert(rc == 0);
	rc = sc_ini_parse_string(NULL, cb16, "\n\0");
	assert(rc == 0);
}

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

void example(void)
{
	string_example();
	file_example();
}

#ifdef SC_HAVE_WRAP

int fail_ferror = 0;

int __real_ferror(FILE *stream);
int __wrap_ferror(FILE *stream)
{
	if (fail_ferror) {
		return -1;
	}

	return __real_ferror(stream);
}

int cb_fail(void *arg, int line, const char *section, const char *key,
	    const char *value)
{
	(void) arg;
	(void) line;

	printf("%s %s %s \n", section, key, value);
	return 0;
}

void test_fail()
{
	int rc;
	FILE *fp;
	static const char *ini = " ;Sample \n"
				 " [section] \n"
				 "key = value0 #;comment\n"
				 "      value1 \n"
				 "      value2 ";

	fp = fopen("config.ini", "w+");
	fwrite(ini, 1, strlen(ini), fp);
	fclose(fp);

	fail_ferror = true;
	rc = sc_ini_parse_file(NULL, cb_fail, "config.ini");
	assert(rc == -1);
	fail_ferror = false;
}
#else
void test_fail()
{
}
#endif

int main()
{
	example();
	test1();
	test2();
	test3();
	test4();
	test5();
	test6();
	test7();
	test8();
	test9();
	test10();
	test11();
	test12();
	test13();
	test14();
	test15();
	test16();
	test_fail();

	return 0;
}

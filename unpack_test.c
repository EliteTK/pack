#include <stdio.h>
#include <stdbool.h>

#include "pack.h"

typedef bool test_func(void);

struct test {
	test_func *func;
	char *desc;
};

#define TEST(name) bool test_##name(void)
#define TEST_ENTRY(name, description) { test_##name, description }
#define TEST_ENTRY_END { NULL, NULL }
#define DATA(...) (unsigned char []){ __VA_ARGS__ }, sizeof (unsigned char []){ __VA_ARGS__ }
#define CHECK(test) if (!(test)) { puts("! " #test); return false; }

#define CHECK_UNPACK(data, fmt, ...) do { \
	enum pack_status s = unpack(data, fmt, __VA_ARGS__); \
	if (s != PACK_OK) { \
		printf(__FILE__ ":%d unpack(" #data ", " #fmt ", ...) -> %s (%d)\n", __LINE__, pack_strerror(s), s); \
		return false; \
	} \
} while (0)
#define CHECK_EQUAL(a, b) if (a != b) { printf(__FILE__ ":%d %d != %d\n", __LINE__, a, b); return false; }

TEST(schar)
{
	signed char c = __LINE__;
	fprintf(stderr, "Address of c: %p\n", (void *)&c);
	CHECK_UNPACK(DATA(0), "b", &c);
	CHECK_EQUAL(c, 0);
	CHECK_UNPACK(DATA(1), "b", &c);
	CHECK_EQUAL(c, 1);
	CHECK_UNPACK(DATA(127), "b", &c);
	CHECK_EQUAL(c, 127);
	CHECK_UNPACK(DATA(255), "b", &c);
	CHECK_EQUAL(c, -1);
	CHECK_UNPACK(DATA(128), "b", &c);
	CHECK_EQUAL(c, -128);
	return true;
}

TEST(uchar)
{
	unsigned char c = __LINE__;
	fprintf(stderr, "Address of c: %p\n", (void *)&c);
	CHECK_UNPACK(DATA(0), "B", &c);
	CHECK_EQUAL(c, 0);
	CHECK_UNPACK(DATA(1), "B", &c);
	CHECK_EQUAL(c, 1);
	CHECK_UNPACK(DATA(255), "B", &c);
	CHECK_EQUAL(c, 255);
	return true;
}

TEST(sshort)
{
	short s = __LINE__;
	fprintf(stderr, "Address of s: %p\n", (void *)&s);
	CHECK_UNPACK(DATA(0, 0), "h", &s);
	CHECK_EQUAL(s, 0);
	CHECK_UNPACK(DATA(0, 0), ">h", &s);
	CHECK_EQUAL(s, 0);
	CHECK_UNPACK(DATA(0, 0), "<h", &s);
	CHECK_EQUAL(s, 0);
	CHECK_UNPACK(DATA(0, 1), "h", &s);
	CHECK_EQUAL(s, 1);
	CHECK_UNPACK(DATA(0, 1), ">h", &s);
	CHECK_EQUAL(s, 1);
	CHECK_UNPACK(DATA(1, 0), "<h", &s);
	CHECK_EQUAL(s, 1);
	CHECK_UNPACK(DATA(0x7f, 0xff), "h", &s);
	CHECK_EQUAL(s, 32767);
	CHECK_UNPACK(DATA(0x7f, 0xff), ">h", &s);
	CHECK_EQUAL(s, 32767);
	CHECK_UNPACK(DATA(0xff, 0x7f), "<h", &s);
	CHECK_EQUAL(s, 32767);
	CHECK_UNPACK(DATA(0xff, 0xff), "h", &s);
	CHECK_EQUAL(s, -1);
	CHECK_UNPACK(DATA(0xff, 0xff), ">h", &s);
	CHECK_EQUAL(s, -1);
	CHECK_UNPACK(DATA(0xff, 0xff), "<h", &s);
	CHECK_EQUAL(s, -1);
	CHECK_UNPACK(DATA(0x80, 0x00), "h", &s);
	CHECK_EQUAL(s, -32768);
	CHECK_UNPACK(DATA(0x80, 0x00), ">h", &s);
	CHECK_EQUAL(s, -32768);
	CHECK_UNPACK(DATA(0x00, 0x80), "<h", &s);
	CHECK_EQUAL(s, -32768);
}

TEST(ushort)
{
	unsigned short s;
}

int main(void)
{
	struct test tests[] = {
		TEST_ENTRY(schar, "schar unpacking"),
		TEST_ENTRY(uchar, "uchar unpacking"),
		TEST_ENTRY(sshort, "sshort unpacking"),
		TEST_ENTRY_END
	};

	for (int i = 0; tests[i].func != NULL; i++)
		printf("%s %s\n", tests[i].func() ? " OK " : "FAIL", tests[i].desc);
}

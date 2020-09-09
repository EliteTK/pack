/*
 * Copyright (C) 2020 Tomasz Kramkowski <tk@the-tk.com>
 * SPDX-License-Identifier: MIT
 */
#include <stdint.h>
#include <inttypes.h>
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
	enum pack_status CHECK_UNPACK_s = unpack(data, fmt, __VA_ARGS__); \
	if (CHECK_UNPACK_s != PACK_OK) { \
		printf(__FILE__ ":%d unpack(" #data ", " #fmt ", ...) -> %s (%d)\n", __LINE__, pack_strerror(CHECK_UNPACK_s), CHECK_UNPACK_s); \
		return false; \
	} \
} while (0)
#define CHECK_EQUAL(f, a, b) if (a != b) { printf(__FILE__ ":%d %"f" != %"f"\n", __LINE__, a, b); return false; }

#include "unpack_test.inc"

int main(void)
{
	pack_trace = PACK_TRACE_OFF;

	struct test tests[] = {
		TEST_ENTRY(signed_char, "signed char unpacking"),
		TEST_ENTRY(unsigned_char, "unsigned char unpacking"),
		TEST_ENTRY(signed_short, "signed short unpacking"),
		TEST_ENTRY(unsigned_short, "unsigned short unpacking"),
		TEST_ENTRY(signed_int, "signed int unpacking"),
		TEST_ENTRY(unsigned_int, "unsigned int unpacking"),
		TEST_ENTRY(signed_long, "signed long unpacking"),
		TEST_ENTRY(unsigned_long, "unsigned long unpacking"),
		TEST_ENTRY(signed_long_long, "signed long long unpacking"),
		TEST_ENTRY(unsigned_long_long, "unsigned long long unpacking"),
		TEST_ENTRY_END
	};

	for (int i = 0; tests[i].func != NULL; i++)
		printf("%s %s\n", tests[i].func() ? " OK " : "FAIL", tests[i].desc);
}

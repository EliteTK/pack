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

#define TEST(name, desc) static bool test_##name(void); \
	static struct test testinfo_##name \
	__attribute__((__section__("tests"))) \
	__attribute__((__used__)) = \
		{ test_##name, desc }; \
	static bool test_##name(void)
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

TEST(simple0_float, "simple unpack float")
{
	float v[1] = { __LINE__ };

	CHECK_UNPACK(DATA(0x00, 0x00, 0x00, 0x00), "f", &v);
	CHECK_EQUAL("f", v[0], 0.0f);
	CHECK_UNPACK(DATA(0x3f, 0x80, 0x00, 0x00), "f", &v);
	CHECK_EQUAL("f", v[0], 1.0f);
	CHECK_UNPACK(DATA(0x41, 0x00, 0x00, 0x00), "f", &v);
	CHECK_EQUAL("f", v[0], 8.0f);

	return true;
}

int main(void)
{
	extern struct test __start_tests, __stop_tests;

	pack_trace = PACK_TRACE_OFF;

	for (struct test *t = &__start_tests; t < &__stop_tests; t++) {
		if (t->func()) {
			printf(" OK %s\n", t->desc);
			continue;
		}
		pack_trace = PACK_TRACE_ALL;
		fprintf(stderr, ">>> Test failure trace for '%s'\n", t->desc);
		t->func();
		fprintf(stderr, "<<<\n");
		pack_trace = PACK_TRACE_OFF;
		printf("FAIL %s\n", t->desc);
	}
}

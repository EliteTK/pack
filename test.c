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
#define DATA(...) (unsigned char []){ __VA_ARGS__ }
#define CHECK(test) if (!(test)) { puts("! " #test); return false; }

#define CHECK_UNPACK(data, fmt, ...) do { \
	enum pack_status CHECK_UNPACK_s = unpack(data, sizeof (data), fmt, __VA_ARGS__); \
	if (CHECK_UNPACK_s != PACK_OK) { \
		printf(__FILE__ ":%d unpack(" #data ", " #fmt ", ...) -> %s (%d)\n", __LINE__, pack_strerror(CHECK_UNPACK_s), CHECK_UNPACK_s); \
		return false; \
	} \
} while (0)
#define CHECK_EQUAL(f, a, b) if (a != b) { printf(__FILE__ ":%d %"f" != %"f"\n", __LINE__, a, b); return false; }

#include "test.inc"

TEST(unpack_simple0_float, "unpack simple float")
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

TEST(unpack_simple0_double, "unpack simple double")
{
	double v[1] = { __LINE__ };

	CHECK_UNPACK(DATA(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00), "d", &v);
	CHECK_EQUAL("f", v[0], 0.0f);
	CHECK_UNPACK(DATA(0x3f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00), "d", &v);
	CHECK_EQUAL("f", v[0], 1.0f);
	CHECK_UNPACK(DATA(0x40, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00), "d", &v);
	CHECK_EQUAL("f", v[0], 8.0f);

	return true;
}

TEST(unpack_simple_padding, "unpack simple padding")
{
	struct {
		int i;
		unsigned char c;
		float f;
	} v = { __LINE__, __LINE__ + 1, __LINE__ + 2 };

	CHECK_UNPACK(DATA(0xff, 0x85,
			  0x00, 0x00, 0x00,
			  0x16,
			  0x00, 0x00,
			  0x40, 0x44, 0x58, 0x00),
		     ">i3xBxxf", &v.i, &v.c, &v.f);
	CHECK_EQUAL("d", v.i, -123);
	CHECK_EQUAL("d", v.c, 22);
	CHECK_EQUAL("f", v.f, 0x1.88bp+1);

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

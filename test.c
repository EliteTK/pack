/*
 * Copyright (C) 2020 Tomasz Kramkowski <tk@the-tk.com>
 * SPDX-License-Identifier: MIT
 */
#include <stdint.h>
#include <assert.h>
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

#define CHECK_PACK(dest, fmt, ...) do { \
	enum pack_status CHECK_PACK_s = pack(dest, sizeof (dest), fmt, __VA_ARGS__); \
	if (CHECK_PACK_s != PACK_OK) { \
		printf(__FILE__ ":%d pack(%p, %zu, " #fmt ", ...) -> %s (%d)\n", __LINE__, dest, sizeof (dest), pack_strerror(CHECK_PACK_s), CHECK_PACK_s); \
		return false; \
	} \
} while (0)

#define CHECK_BUFEQUAL(a, b) \
	static_assert(sizeof (a) == sizeof (b), "CHECK_BUFEQUAL - buffer sizes don't match"); \
	if (!do_check_bufequal(__FILE__, __LINE__, (a), (b), sizeof (a))) return false;

static bool do_check_bufequal(const char *file, int line, void *a_, void *b_, size_t s)
{
	unsigned char *a = a_, *b = b_;
	bool mismatch = false;
	size_t start, end;
	for (size_t i = 0; i < s; i++) {
		if (a[i] == b[i]) continue;
		if (!mismatch) {
			start = i;
			mismatch = true;
		}
		end = i + 1;
	}
	if (!mismatch) return true;
	if (start > 0) start--;
	if (end < s) end++;
	printf("%s:%d buffers unequal\n idx", file, line);
	for (size_t i = start; i < end; i++) printf(" %02zu", i);
	printf("\n a =");
	for (size_t i = start; i < end; i++) printf(" %02x", a[i]);
	printf("\n b =");
	for (size_t i = start; i < end; i++) printf(" %02x", b[i]);
	putchar('\n');
	return false;
}

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

TEST(pack_simple_general, "pack simple general")
{
	unsigned char b[12] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

	CHECK_PACK(b, ">ixxxBxxl", -123, 22, -12648430l);
	CHECK_BUFEQUAL(b, DATA(0xff, 0x85,
			       0x00, 0x00, 0x00,
			       0x16,
			       0x00, 0x00,
			       0xff, 0x3f, 0x00, 0x12));

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

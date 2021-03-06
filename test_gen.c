/*
 * Copyright (C) 2020 Tomasz Kramkowski <tk@the-tk.com>
 * SPDX-License-Identifier: MIT
 */
#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

struct {
	enum endian { BIG, LITTLE } e;
	char *prefix;
} endian[] = {
	{ BIG, "" },
	{ BIG, ">" },
	{ LITTLE, "<" },
};

enum fmt {
	FMT_b,
	FMT_B,
	FMT_h,
	FMT_H,
	FMT_i,
	FMT_I,
	FMT_l,
	FMT_L,
	FMT_q,
	FMT_Q,
	FMT_END,
	FMT_BEGIN = FMT_b,
};

struct fmtinfo {
	char fmt;
	char *type;
	intmax_t min;
	uintmax_t max;
	size_t size;
} fmtinfo[] = {
	[FMT_b] = { 'b', "signed char",         INTMAX_C(               -128),     UINTMAX_C(                 127), 1 },
	[FMT_B] = { 'B', "unsigned char",       INTMAX_C(                  0),     UINTMAX_C(                 255), 1 },
	[FMT_h] = { 'h', "signed short",        INTMAX_C(             -32768),     UINTMAX_C(               32767), 2 },
	[FMT_H] = { 'H', "unsigned short",      INTMAX_C(                  0),     UINTMAX_C(               65535), 2 },
	[FMT_i] = { 'i', "signed int",          INTMAX_C(             -32768),     UINTMAX_C(               32767), 2 },
	[FMT_I] = { 'I', "unsigned int",        INTMAX_C(                  0),     UINTMAX_C(               65535), 2 },
	[FMT_l] = { 'l', "signed long",         INTMAX_C(        -2147483648),     UINTMAX_C(          2147483647), 4 },
	[FMT_L] = { 'L', "unsigned long",       INTMAX_C(                  0),     UINTMAX_C(          4294967295), 4 },
	[FMT_q] = { 'q', "signed long long",   -INTMAX_C(9223372036854775807) - 1, UINTMAX_C( 9223372036854775807), 8 },
	[FMT_Q] = { 'Q', "unsigned long long",  INTMAX_C(                  0),     UINTMAX_C(18446744073709551615), 8 },
};

static char cchar(char c)
{
	if (c == '\0') return c;
	if (isalnum(c)) return c;
	return '_';
}

static const char *cname(const char *s)
{
	static char ret[100];
	for (int i = 0; i < 99; i++) {
		ret[i] = cchar(s[i]);
		if (s[i] == '\0') break;
	}
	return ret;
}

static const char *u2bytes(enum endian e, int n, uintmax_t v)
{
	static char b[100];
	char *p = b;

	assert(n > 0 && n <= 8);

	for (int i = 0; i < n; i++) {
		p += snprintf(p, b + sizeof(b) - p, "%s0x%x", i == 0 ? "" : ", ",
			      (int)((v >> (e == LITTLE ? i : n - i - 1) * 8) & 0xff));
		assert(p < b + sizeof(b));
	}

	return b;
}

static uintmax_t i2u(int n, intmax_t v)
{
	if (v >= 0)
		return v;

	return (UINTMAX_MAX >> (sizeof (uintmax_t) * CHAR_BIT - n * 8)) + v + 1;
}

static const char *i2bytes(enum endian e, int n, intmax_t v)
{
	return u2bytes(e, n, i2u(n, v));
}

static void signed_check(FILE *out, const struct fmtinfo *fi, enum endian e, intmax_t testval, int arraysize)
{
	char sizeprefix[11] = "";
	const char *data;

	if (arraysize >= 1)
		snprintf(sizeprefix, sizeof sizeprefix, "%d", arraysize);
	else
		arraysize = 1;

	fprintf(out, "\tCHECK_UNPACK(DATA(");
	data = i2bytes(endian[e].e, fi->size, testval);
	for (int i = 0; i < arraysize; i++)
		fprintf(out, "%s%s", i == 0 ? "" : ", ", data);
	fprintf(out, "), \"%s%s%c\", v);\n", endian[e].prefix, sizeprefix, fi->fmt);
	for (int i = 0; i < arraysize; i++)
		fprintf(out, "\tCHECK_EQUAL(PRIdMAX, (intmax_t)v[%d], -INTMAX_C(%" PRIdMAX ")-1);\n", i, -(testval + 1));
}

static void unsigned_check(FILE *out, const struct fmtinfo *fi, enum endian e, uintmax_t testval, int arraysize)
{
	char sizeprefix[11] = "";
	const char *data;

	if (arraysize >= 1)
		snprintf(sizeprefix, sizeof sizeprefix, "%d", arraysize);
	else
		arraysize = 1;

	fprintf(out, "\tCHECK_UNPACK(DATA(");
	data = u2bytes(endian[e].e, fi->size, testval);
	for (int i = 0; i < arraysize; i++)
		fprintf(out, "%s%s", i == 0 ? "" : ", ", data);
	fprintf(out, "), \"%s%s%c\", v);\n", endian[e].prefix, sizeprefix, fi->fmt);
	for (int i = 0; i < arraysize; i++)
		fprintf(out, "\tCHECK_EQUAL(PRIdMAX, (uintmax_t)v[%d], UINTMAX_C(%" PRIuMAX "));\n", i, testval);
}

static void generate_simple(FILE *out, enum fmt fmt, int arraysize)
{
	int realsize = arraysize == 0 ? 1 : arraysize;
	struct fmtinfo *fi;
	bool sign;

	assert(fmt < FMT_END);

	fi = &fmtinfo[fmt];

	sign = islower(fi->fmt);

	fprintf(out, "TEST(unpack_simple%d_%s, \"unpack simple", arraysize, cname(fi->type));
	if (arraysize != 0)
		fprintf(out, " array[%d]", arraysize);
	fprintf(out, " %s\")\n", fi->type);
	fprintf(out, "{\n");
	fprintf(out, "\t%s v[%d] = { ", fi->type, realsize );
	for (int i = 0; i < realsize; i++)
		fprintf(out, "%s__LINE__ + %d", i == 0 ? "" : ", ", i);
	fprintf(out, " };\n");
	for (size_t e = 0; e < sizeof endian / sizeof endian[0]; e++) {
		for (int i = sign ? -1 : 0; i <= 1; i++)
			signed_check(out, fi, e, i, arraysize);
		if (sign)
			signed_check(out, fi, e, fi->min, arraysize);
		unsigned_check(out, fi, e, fi->max, arraysize);
	}
	fprintf(out, "\treturn true;\n");
	fprintf(out, "}\n");
}

int main(void)
{
	FILE *out = stdout;
	for (enum fmt fmt = FMT_BEGIN; fmt < FMT_END; fmt++) {
		generate_simple(out, fmt, 0);
		for (int i = 2; i < 4; i++)
			generate_simple(out, fmt, i);
	}
}

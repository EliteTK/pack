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

char cchar(char c)
{
	if (c == '\0') return c;
	if (isalnum(c)) return c;
	return '_';
}

const char *cname(const char *s)
{
	static char ret[100];
	for (int i = 0; i < 99; i++) {
		ret[i] = cchar(s[i]);
		if (s[i] == '\0') break;
	}
	return ret;
}

const char *u2bytes(enum endian e, int n, uintmax_t v)
{
	static char b[100];
	char *p = b;

	assert(n > 0 && n <= 8);

	for (int i = 0; i < n; i++) {
		p += snprintf(p, b + sizeof(b) - p, "%s0x%x", i == 0 ? "" : ", ",
			      (v >> (e == LITTLE ? i : n - i - 1) * 8) & 0xff);
		assert(p < b + sizeof(b));
	}

	return b;
}

uintmax_t i2u(int n, intmax_t v)
{
	if (v >= 0)
		return v;

	return (UINTMAX_MAX >> (sizeof (uintmax_t) * CHAR_BIT - n * 8)) + v + 1;
}

const char *i2bytes(enum endian e, int n, intmax_t v)
{
	return u2bytes(e, n, i2u(n, v));
}

void generate_simple(FILE *out, char fmt, const char *type, intmax_t min, uintmax_t max, int size)
{
	unsigned char data[8];
	char *prefix;
	bool sign;

	sign = islower(fmt);
	prefix = sign ? "signed" : "unsigned";

	assert(size <= sizeof data);

	fprintf(out, "TEST(%s_%s)\n", prefix, cname(type));
	fprintf(out, "{\n");
	fprintf(out, "\t%s %s %c = __LINE__;\n", prefix, type, fmt);
	for (size_t e = 0; e < sizeof endian / sizeof endian[0]; e++) {
		for (int i = sign ? -1 : 0; i <= 1; i++) {
			fprintf(out, "\tCHECK_UNPACK(DATA(%s), \"%s%c\", &%c);\n",
				i2bytes(endian[e].e, size, i), endian[e].prefix, fmt, fmt);
			fprintf(out, "\tCHECK_EQUAL(PRIdMAX, (intmax_t)%c, INTMAX_C(%d));\n", fmt, i);
		}
		if (sign) {
			fprintf(out, "\tCHECK_UNPACK(DATA(%s), \"%s%c\", &%c);\n",
				i2bytes(endian[e].e, size, min), endian[e].prefix, fmt, fmt);
			fprintf(out, "\tCHECK_EQUAL(PRIdMAX, (intmax_t)%c, INTMAX_C(%" PRIdMAX "));\n", fmt, min);
		}
		fprintf(out, "\tCHECK_UNPACK(DATA(%s), \"%s%c\", &%c);\n",
			u2bytes(endian[e].e, size, max), endian[e].prefix, fmt, fmt);
		fprintf(out, "\tCHECK_EQUAL(PRIuMAX, (uintmax_t)%c, UINTMAX_C(%" PRIuMAX "));\n", fmt, max);
	}
	fprintf(out, "\treturn true;\n");
	fprintf(out, "}\n");
}

int main(void)
{
	FILE *out = stdout;
	generate_simple(out, 'b', "char",      INTMAX_C(                -128), UINTMAX_C(                 127), 1);
	generate_simple(out, 'B', "char",      INTMAX_C(                   0), UINTMAX_C(                 255), 1);
	generate_simple(out, 'h', "short",     INTMAX_C(              -32768), UINTMAX_C(               32767), 2);
	generate_simple(out, 'H', "short",     INTMAX_C(                   0), UINTMAX_C(               65535), 2);
	generate_simple(out, 'i', "int",       INTMAX_C(              -32768), UINTMAX_C(               32767), 2);
	generate_simple(out, 'I', "int",       INTMAX_C(                   0), UINTMAX_C(               65535), 2);
	generate_simple(out, 'l', "long",      INTMAX_C(         -2147483648), UINTMAX_C(          2147483647), 4);
	generate_simple(out, 'L', "long",      INTMAX_C(                   0), UINTMAX_C(          4294967295), 4);
	generate_simple(out, 'q', "long long", -INTMAX_C(9223372036854775807)-1, UINTMAX_C( 9223372036854775807), 8);
	generate_simple(out, 'Q', "long long", INTMAX_C(                   0), UINTMAX_C(18446744073709551615), 8);
}
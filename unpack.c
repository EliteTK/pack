/*
 * Copyright (C) 2020 Tomasz Kramkowski <tk@the-tk.com>
 * SPDX-License-Identifier: MIT
 */
#include <ctype.h>
#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "common.h"
#include "pack.h"
#include "trace.h"

enum endian { BIG, LITTLE };

static uintmax_t read_val(unsigned char *buf, size_t size, enum endian e)
{
	uintmax_t val = 0;

	for (size_t i = 0; i < size; i++) {
		val |= (uintmax_t)(buf[i] & 0xff) << (e == LITTLE ? i : size - i - 1) * 8;
	}

	return val;
}

enum pack_status unpack(void *buf_, size_t size, const char *fmt, ...)
{
	enum endian endianness = BIG;
	unsigned char *buf = buf_;
	size_t offset = 0;
	va_list ap;

	tr_call("unpack(%p, %zu, %s, ...)", buf, size, fmt);

	va_start(ap, fmt);

	for (int i = 0; fmt[i] != '\0'; i++) {
		bool sign = islower(fmt[i]);
		size_t s;
		union {
			signed   char      *b;
			unsigned char      *B;
			         short     *h;
			unsigned short     *H;
			         int       *i;
			unsigned int       *I;
			         long      *l;
			unsigned long      *L;
			         long long *q;
			unsigned long long *Q;
		} arg;
		union { uintmax_t u; intmax_t s; } val;
		tr_debug("i: %d, fmt[i]: %c, sign: %ssigned", i, fmt[i], sign ? "" : "un");
		switch (fmt[i]) {
		case '>': endianness = BIG; continue;
		case '<': endianness = LITTLE; continue;
		case 'b': arg.b = va_arg(ap, signed   char      *); break;
		case 'B': arg.B = va_arg(ap, unsigned char      *); break;
		case 'h': arg.h = va_arg(ap,          short     *); break;
		case 'H': arg.H = va_arg(ap, unsigned short     *); break;
		case 'i': arg.i = va_arg(ap,          int       *); break;
		case 'I': arg.I = va_arg(ap, unsigned           *); break;
		case 'l': arg.l = va_arg(ap,          long      *); break;
		case 'L': arg.L = va_arg(ap, unsigned long      *); break;
		case 'q': arg.q = va_arg(ap,          long long *); break;
		case 'Q': arg.Q = va_arg(ap, unsigned long long *); break;
		case 'x': break;
		return PACK_FMTINVAL;
		}

		s = getsize(fmt[i]);
		tr_debug("s: %zu", s);
		if (s == (size_t)-1) return PACK_FMTINVAL;

		if (size - offset < s) return PACK_TOOSMALL;

		if (fmt[i] == 'x') goto skip;

		val.u = read_val(buf, s, endianness);
		tr_debug("val.u: %" PRIuMAX, val.u);

		if (sign) {
			intmax_t vals;
			if (!(val.u & (UINTMAX_C(1) << (s * 8 - 1)))) {
				vals = val.u;
			} else {
				uintmax_t offt = UINTMAX_MAX >> (sizeof offt * CHAR_BIT - s * 8);
				vals = val.u - offt - 1;
			}
			val.s = vals;
			tr_debug("val.s: %" PRIdMAX, val.s);
		}

		switch (fmt[i]) {
		case 'b': *arg.b = val.s; break;
		case 'B': *arg.B = val.u; break;
		case 'h': *arg.h = val.s; break;
		case 'H': *arg.H = val.u; break;
		case 'i': *arg.i = val.s; break;
		case 'I': *arg.I = val.u; break;
		case 'l': *arg.l = val.s; break;
		case 'L': *arg.L = val.u; break;
		case 'q': *arg.q = val.s; break;
		case 'Q': *arg.Q = val.u; break;
		}
skip:
		offset += s;
	}

	va_end(ap);

	return PACK_OK;
}

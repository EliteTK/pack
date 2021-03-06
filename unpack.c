/*
 * Copyright (C) 2020 Tomasz Kramkowski <tk@the-tk.com>
 * SPDX-License-Identifier: MIT
 */
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "common.h"
#include "pack.h"
#include "trace.h"

#define GEN_CONV_IEEE754B(type, total, nexp, nfrac) \
static type convert_ieee754b##total(uintmax_t b) \
{ \
	bool isneg; \
	int exp; \
	type n; \
\
	isneg = (b >> (total - 1)) & 0x1; \
	exp = (b >> nfrac) & BITMASK(nexp); \
	n = b & BITMASK(nfrac); \
\
	if (exp == BITMASK(nexp)) { \
		if (n) { \
			return NAN; \
		} else { \
			return isneg ? -INFINITY : INFINITY; \
		} \
	} else if (exp == 0) { \
		if (n == 0) \
			return isneg ? -0.0 : 0.0; \
		exp = -(int)(BITMASK(nexp) / 2 - 1); \
	} else { \
		n += 0x1p##nfrac##f; \
		exp -= BITMASK(nexp) / 2; \
	} \
\
	n = ldexpf(n, exp - nfrac); \
\
	return isneg ? -n : n; \
}

GEN_CONV_IEEE754B(float, 32, 8, 23)
GEN_CONV_IEEE754B(double, 64, 11, 52)

static uintmax_t read_val(const unsigned char *buf, size_t size, enum endian e)
{
	uintmax_t val = 0;

	for (size_t i = 0; i < size; i++) {
		val |= (uintmax_t)(buf[i] & 0xff) << (e == LITTLE ? i : size - i - 1) * 8;
	}

	return val;
}

enum pack_status unpack(const void *buf_, size_t size, const char *fmt, ...)
{
	enum endian endianness = BIG;
	const unsigned char *buf = buf_;
	size_t offset = 0;
	va_list ap;

	tr_call("unpack(%p, %zu, %s, ...)", buf, size, fmt);

	va_start(ap, fmt);

	for (int i = 0; fmt[i] != '\0'; i++) {
		bool sign;
		size_t count = 1, s;
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
			         float     *f;
			         double    *d;
		} arg;
		union { uintmax_t u; intmax_t s; float f; double d; } val;
		tr_debug("i: %d, fmt[i]: %c", i, fmt[i]);
		if (isdigit(fmt[i])) {
			unsigned long long c;
			char *end;

			errno = 0;
			c = strtoull(&fmt[i], &end, 10);
			if ((c == ULLONG_MAX && errno == ERANGE) || c > SIZE_MAX)
				return PACK_FMTINVAL;
			count = c;
			i += end - &fmt[i];
		} else if (fmt[i] == '*') {
			count = va_arg(ap, size_t);
			i++;
		}
		sign = islower(fmt[i]);
		tr_debug("count: %zu, i: %d, fmt[i]: %c, sign: %ssigned",
			 count, i, fmt[i], sign ? "" : "un");
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
		case 'f': arg.f = va_arg(ap,          float     *); break;
		case 'd': arg.d = va_arg(ap,          double    *); break;
		case 'x': break;
		default: return PACK_FMTINVAL;
		}

		s = getsize(fmt[i]);
		tr_debug("s: %zu", s);
		if (s == (size_t)-1) return PACK_FMTINVAL;

		if (size - offset < s * count) return PACK_TOOSMALL;

		if (fmt[i] == 'x') goto skip;

		for (size_t j = 0; j < count; j++) {
			val.u = read_val(buf + offset + s * j, s, endianness);
			tr_debug("val.u: %" PRIuMAX ", at: %zu", val.u, offset + s * j);

			if (fmt[i] == 'f') {
				float f = convert_ieee754b32(val.u);
				val.f = f;
				tr_debug("val.f: %f", val.f);
			} else if (fmt[i] == 'd') {
				double d = convert_ieee754b64(val.u);
				val.d = d;
				tr_debug("val.d: %f", val.d);
			} else if (sign) {
				intmax_t vals;
				if (!(val.u & (UINTMAX_C(1) << (s * 8 - 1)))) {
					vals = val.u;
				} else {
					uintmax_t offt = BITMASK(s * 8);
					vals = val.u - offt - 1;
				}
				val.s = vals;
				tr_debug("val.s: %" PRIdMAX, val.s);
			}

			switch (fmt[i]) {
			case 'b': arg.b[j] = val.s; break;
			case 'B': arg.B[j] = val.u; break;
			case 'h': arg.h[j] = val.s; break;
			case 'H': arg.H[j] = val.u; break;
			case 'i': arg.i[j] = val.s; break;
			case 'I': arg.I[j] = val.u; break;
			case 'l': arg.l[j] = val.s; break;
			case 'L': arg.L[j] = val.u; break;
			case 'q': arg.q[j] = val.s; break;
			case 'Q': arg.Q[j] = val.u; break;
			case 'f': arg.f[j] = val.f; break;
			case 'd': arg.d[j] = val.d; break;
			}
		}
skip:
		offset += s * count;
	}

	va_end(ap);

	return PACK_OK;
}

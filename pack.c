/*
 * Copyright (C) 2020 Tomasz Kramkowski <tk@the-tk.com>
 * SPDX-License-Identifier: MIT
 */
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "common.h"
#include "pack.h"
#include "trace.h"

static void write_val(unsigned char *buf, size_t size, enum endian e, uintmax_t val)
{
	for (size_t i = 0; i < size; i++)
		buf[i] = (val >> (e == LITTLE ? i : size - i - 1) * 8) & 0xff;
}

enum pack_status pack(void *buf_, size_t size, const char *fmt, ...)
{
	enum endian endianness = BIG;
	unsigned char *buf = buf_;
	size_t offset = 0;
	va_list ap;

	tr_call("pack(%p, %zu, %s, ...)", buf, size, fmt);

	va_start(ap, fmt);

	for (int i = 0; fmt[i] != '\0'; i++) {
		bool sign;
		size_t s;
		union { uintmax_t u; intmax_t s; } val;
		tr_debug("i: %d, fmt[i]: %c", i, fmt[i]);
		sign = islower(fmt[i]);
		switch (fmt[i]) {
		case '>': endianness = BIG; continue;
		case '<': endianness = LITTLE; continue;
		case 'b': val.s = va_arg(ap,          int      ); break;
		case 'B': val.u = va_arg(ap, unsigned          ); break;
		case 'h': val.s = va_arg(ap,          int      ); break;
		case 'H': val.u = va_arg(ap, unsigned          ); break;
		case 'i': val.s = va_arg(ap,          int      ); break;
		case 'I': val.u = va_arg(ap, unsigned          ); break;
		case 'l': val.s = va_arg(ap,          long     ); break;
		case 'L': val.u = va_arg(ap, unsigned long     ); break;
		case 'q': val.s = va_arg(ap,          long long); break;
		case 'Q': val.u = va_arg(ap, unsigned long long); break;
		case 'x': val.u = 0; sign = false; break;
		default: return PACK_FMTINVAL;
		}
		tr_debug("i: %d, fmt[i]: %c, sign: %ssigned", i, fmt[i], sign ? "" : "un");

		s = getsize(fmt[i]);
		tr_debug("s: %zu", s);
		if (s == (size_t)-1) return PACK_FMTINVAL;

		if (size - offset < s) return PACK_TOOSMALL;

		if (sign) {
			intmax_t n = val.s;
			tr_debug("val.s: %" PRIdMAX, val.s);
			if (val.s >= 0) {
				val.u = n;
			} else {
				uintmax_t offt = BITMASK(s * 8);
				n += 1;
				val.u = offt + n;
			}
		}
		tr_debug("val.u: %" PRIuMAX, val.u);
		write_val(&buf[offset], s, endianness, val.u);
		offset += s;
	}

	va_end(ap);

	return PACK_OK;
}

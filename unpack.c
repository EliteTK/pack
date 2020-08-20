#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#include <stdio.h>

#include "common.h"
#include "pack.h"

typedef enum pack_status unpacker(void *buf, size_t size, va_list ap);

enum endian { BIG, LITTLE };

uintmax_t read_val(unsigned char *buf, size_t size, enum endian e)
{
	unsigned long long val = 0;

	for (size_t i = 0; i < size; i++)
		val |= (buf[i] & 0xff) << (e == LITTLE ? i : size - i - 1) * 8;

	return val;
}
#define X(M) \
	M(b, signed   char     ) \
	M(B, unsigned char     ) \
	M(h,          short    ) \
	M(H, unsigned short    ) \
	M(i,          int      ) \
	M(I, unsigned          ) \
	M(l,          long     ) \
	M(L, unsigned long     ) \
	M(q,          long long) \
	M(Q, unsigned long long)

enum pack_status unpack(void *buf_, size_t size, const char *fmt, ...)
{
	enum endian endianness = BIG;
	unsigned char *buf = buf_;
	size_t offset = 0;
	va_list ap;

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
		/*void *arg;*/
		union { uintmax_t u; intmax_t s; } val;
		switch (fmt[i]) {
		case '>': endianness = BIG; continue;
		case '<': endianness = LITTLE; continue;
		case 'b': arg.b = va_arg(ap, signed   char      *);
		case 'B': arg.B = va_arg(ap, unsigned char      *);
		case 'h': arg.h = va_arg(ap,          short     *);
		case 'H': arg.H = va_arg(ap, unsigned short     *);
		case 'i': arg.i = va_arg(ap,          int       *);
		case 'I': arg.I = va_arg(ap, unsigned           *);
		case 'l': arg.l = va_arg(ap,          long      *);
		case 'L': arg.L = va_arg(ap, unsigned long      *);
		case 'q': arg.q = va_arg(ap,          long long *);
		case 'Q': arg.Q = va_arg(ap, unsigned long long *);
		case 'x': break;
		return PACK_FMTINVAL;
		}

		s = getsize(fmt[i]);
		if (s == (size_t)-1) return PACK_FMTINVAL;

		if (size - offset < s) return PACK_TOOSMALL;

		if (fmt[i] == 'x') goto skip;

		val.u = read_val(buf, s, endianness);

		fprintf(stderr, "%zu, %llu\n", s, val.u);

		if (sign) {
			intmax_t vals;
			if (!(val.u & (1llu << (s * 8 - 1)))) {
				vals = val.u;
			} else {
				uintmax_t offt = UINTMAX_MAX >> (sizeof offt * CHAR_BIT - s * 8);
				vals = val.u - offt - 1;
			}
			val.s = vals;
			fprintf(stderr, "signed %lld\n", val.s);
		}

		switch (fmt[i]) {
#define D(t, f) fprintf(stderr, "(%p) "#t " = %" #f "\n", (void *)arg.t, *arg.t)
		case 'b': *arg.b = val.s; D(b, d); break;
		case 'B': *arg.B = val.u; D(B, u); break;
		case 'h': *arg.h = val.s; D(h, d); break;
		case 'H': *arg.H = val.u; D(H, u); break;
		case 'i': *arg.i = val.s; D(i, d); break;
		case 'I': *arg.I = val.u; D(I, u); break;
		case 'l': *arg.l = val.s; D(l, ld); break;
		case 'L': *arg.L = val.u; D(L, lu); break;
		case 'q': *arg.q = val.s; D(q, lld); break;
		case 'Q': *arg.Q = val.u; D(Q, llu); break;
		}
skip:
		offset += s;
	}

	va_end(ap);

	return PACK_OK;
}

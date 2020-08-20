#include <stddef.h>

#include "common.h"
#include "pack.h"

size_t getsize(char c)
{
	switch (c) {
	case 'b': case 'B':
	case 'x':
		return 1;
	case 'h': case 'H':
	case 'i': case 'I':
		return 2;
	case 'l': case 'L':
	case 'f':
		return 4;
	case 'q': case 'Q':
	case 'd':
		return 8;
	case 's': default: return (size_t)-1;
	}
}

const char *pack_strerror(enum pack_status status)
{
	switch (status) {
	case PACK_OK: return "Success";
	case PACK_FMTINVAL: return "Invalid format parameter";
	case PACK_TOOSMALL: return "Buffer too small";
	default: return "Invalid Status";
	}
}

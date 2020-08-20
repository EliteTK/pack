#ifndef PACK_H
#define PACK_H

#include <stddef.h>

enum pack_status {
	PACK_OK,
	PACK_FMTINVAL,
	PACK_TOOSMALL,
};

enum pack_status unpack(void *buf, size_t size, const char *fmt, ...);

const char *pack_strerror(enum pack_status status);

#endif // !PACK_H

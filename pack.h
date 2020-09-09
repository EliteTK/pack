/*
 * Copyright (C) 2020 Tomasz Kramkowski <tk@the-tk.com>
 * SPDX-License-Identifier: MIT
 */
#ifndef PACK_H
#define PACK_H

#include <stddef.h>

enum pack_status {
	PACK_OK,
	PACK_FMTINVAL,
	PACK_TOOSMALL,
};

extern enum pack_trace {
	PACK_TRACE_OFF,
	PACK_TRACE_CALL,
	PACK_TRACE_DEBUG,
	PACK_TRACE_ALL,
} pack_trace;

enum pack_status unpack(void *buf, size_t size, const char *fmt, ...);

const char *pack_strerror(enum pack_status status);

#endif // !PACK_H

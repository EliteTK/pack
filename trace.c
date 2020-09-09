/*
 * Copyright (C) 2020 Tomasz Kramkowski <tk@the-tk.com>
 * SPDX-License-Identifier: MIT
 */
#include <stdarg.h>
#include <stdio.h>

#include "pack.h"
#include "trace.h"

enum pack_trace pack_trace = PACK_TRACE_OFF;

void trace(enum pack_trace level, const char *fmt, va_list va)
{
	char *type;

	if (level > pack_trace) return;

	switch (level) {
	case PACK_TRACE_CALL: type = "CALL"; break;
	case PACK_TRACE_DEBUG: type = "DEBUG"; break;
	}

	fprintf(stderr, "PACK_TRACE_%s: ", type);
	vfprintf(stderr, fmt, va);
	fputc('\n', stderr);
}

void tr_call(const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	trace(PACK_TRACE_CALL, fmt, va);
	va_end(va);
}

void tr_debug(const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	trace(PACK_TRACE_DEBUG, fmt, va);
	va_end(va);
}

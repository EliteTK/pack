/*
 * Copyright (C) 2020 Tomasz Kramkowski <tk@the-tk.com>
 * SPDX-License-Identifier: MIT
 */
#ifndef PACK_TRACE_H
#define PACK_TRACE_H

#ifdef DEVEL
#define ATTRIBUTE_FORMAT(type, fmt, args) __attribute__((format(type, fmt, args)))
#else
#define ATTRIBUTE_FORMAT(type, fmt, args)
#endif

ATTRIBUTE_FORMAT(printf, 1, 2)
void tr_call(const char *fmt, ...);
ATTRIBUTE_FORMAT(printf, 1, 2)
void tr_debug(const char *fmt, ...);

#endif // !PACK_TRACE_H

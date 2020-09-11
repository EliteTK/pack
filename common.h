/*
 * Copyright (C) 2020 Tomasz Kramkowski <tk@the-tk.com>
 * SPDX-License-Identifier: MIT
 */
#ifndef PACK_INTERNAL_H
#define PACK_INTERNAL_H

#include <limits.h>
#include <stddef.h>

#define BITMASK(n) (UINTMAX_MAX >> (sizeof (uintmax_t) * CHAR_BIT - n))

enum endian { BIG, LITTLE };

size_t getsize(char c);

#endif // !PACK_INTERNAL_H

# Copyright (C) 2020 Tomasz Kramkowski <tk@the-tk.com>
# SPDX-License-Identifier: MIT
unpack_test: unpack_test.o unpack.o common.o trace.o
unpack_test.o: unpack_test.c pack.h trace.h common.h unpack_test.inc
trace.o: trace.c trace.h
common.o: common.c common.h
unpack_test.inc: test_gen
	./$< >$@
test_gen: test_gen.o

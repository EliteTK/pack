#!/bin/bash
redo-ifchange "unpack_test.inc"
set -- "$1" "${2%.o}" "$3"
. ./default.o.do

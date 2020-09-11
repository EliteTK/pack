#!/bin/bash
redo-ifchange "test.inc"
set -- "$1" "${2%.o}" "$3"
. ./default.o.do

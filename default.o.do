#!/bin/bash
redo-ifchange "$0" "default.o.do" "$2.c"
. loadargs "compile"
gcc -std=c11 -MD -MF "$2.d" -c "${args[@]}" -o "$3" "$2.c" || exit 1
redo-ifchange "$2.d" $(sed ':a;/\\$/{s/[^:]*: //;N;s/ \\\n//;ba}' <"$2.d")

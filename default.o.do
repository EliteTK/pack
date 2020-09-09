#!/bin/bash
redo-ifchange "$0" "default.o.do" "$2.c"
. loadargs "compile"
gcc -std=c11 -MD -MF "$2.d" -c -o "$3" "${args[@]}" "$2.c"
redo-ifchange "$2.d" $(sed ':a;/\\$/{s/[^:]*: //;N;s/ \\\n//;ba}' <"$2.d")

#!/bin/bash
FLAGS="-Wall -Wextra -Werror"
RAYLIB="-I./raylib/include -L./raylib/lib -l:libraylib.a -lm"
FILES="src/main.c src/utils.c src/simulation.c src/simulation_debug.c src/ui/*.c"
gcc $FLAGS -o main $FILES $RAYLIB

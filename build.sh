#!/bin/sh


set -xe
CFLAGS="-O3 -Wall -Wextra `pkg-config --cflags raylib`"
LIBS="`pkg-config --libs raylib` -lm -lglfw -ldl -lpthread"

clang -Wall -Wextra -o twice twice.c -lm
clang -Wall -Wextra -o gates gates.c -lm
clang -Wall -Wextra -o xor xor.c -lm
clang $CFLAGS -o adder adder.c $LIBS

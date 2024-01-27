#!/bin/sh

set -xe
CFLAGS="-O3 -Wall -Wextra `pkg-config --cflags raylib`"
LIBS="`pkg-config --libs raylib` -lm -lglfw -ldl -lpthread"
NN_H="-I /home/tony/workspace/ml/c/Tsoding_MachineLearning_in_C/framework/"
STB_IMAGE="-I /home/tony/workspace/ml/c/Tsoding_MachineLearning_in_C/software/raylib/src/external/"


# clang -Wall -Wextra -o twice twice.c -lm
# clang -Wall -Wextra -o gates gates.c -lm
# clang -Wall -Wextra $NN_H -o xor xor.c -lm -v
# clang $CFLAGS $NN_H -o adder adder.c $LIBS
# clang $CFLAGS $NN_H -o gym gym.c $LIBS
clang $CFLAGS $NN_H -o simple simple.c $LIBS

#!/bin/sh

set -xe
CFLAGS="-O3 -Wall -Wextra `pkg-config --cflags raylib`"
LIBS="`pkg-config --libs raylib` -lm -lglfw -ldl -lpthread"
NN_H="-I /home/tony/workspace/ml/c/Tsoding_MachineLearning_in_C/framework/"
STB_IMAGE="-I /home/tony/workspace/ml/c/Tsoding_MachineLearning_in_C/software/raylib/src/external/"

clang $CFLAGS $NN_H -o layout layout.c $LIBS
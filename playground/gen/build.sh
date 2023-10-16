#!/bin/sh

set -xe
CFLAGS="-O3 -Wall -Wextra `pkg-config --cflags raylib`"
LIBS="`pkg-config --libs raylib` -lm -lglfw -ldl -lpthread"
NN_H="-I /home/tony/workspace/ml/c/MachineLearningInC/framework/"
STB_IMAGE="-I /home/tony/workspace/ml/c/MachineLearningInC/software/raylib/src/external/"

clang $CFLAGS $NN_H -o adder_gen adder_gen.c $LIBS
clang $CFLAGS $NN_H -o xor_gen xor_gen.c $LIBS
clang $CFLAGS $STB_IMAGE $NN_H -o img2mat img2mat.c $LIBS


#!/bin/sh

set -xe
CFLAGS="-O3 -Wall -Wextra `pkg-config --cflags raylib`"
LIBS="`pkg-config --libs raylib` -lm -lglfw -ldl -lpthread"
NN_H="-I /home/tony/workspace/ml/c/MachineLearningInC/framework/"
# NN_H="-I /root/workspace/ml/Tsoding_MachineLearning_in_C/framework/"

clang -Wall -Wextra -o nn nn.c -lm
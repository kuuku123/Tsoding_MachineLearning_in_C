#!/bin/sh

set -xe

OLIVE="-I /home/tony/workspace/ml/c/MachineLearningInC/framework/dump/"
clang $OLIVE -Wall -Wextra -o main main.c
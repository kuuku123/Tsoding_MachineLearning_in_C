#!/bin/sh

set -xe

clang -Wall -Wextra -o dump_nn dump_nn.c -lm
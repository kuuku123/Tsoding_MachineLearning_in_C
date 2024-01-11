#include <stdio.h>

#define NN_IMPLEMENTATION
#include "nn.h"
int main() 
{
    size_t arch[] = {2, 3, 4, 2};
    NN nn = nn_alloc(arch, ARRAY_LEN(arch));
    nn_rand(nn, -1, 1);
    NN_PRINT(nn);

    return 0;
}
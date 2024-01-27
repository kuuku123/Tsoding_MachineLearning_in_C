#include <stdio.h>

#define NN_IMPLEMENTATION
#include "nn.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <math.h>

int main() 
{
    size_t arch[] = {2, 3, 4, 2};
    NN nn = nn_alloc(arch, ARRAY_LEN(arch));
    nn_rand(nn, -1, 1);

    InitWindow(800, 600, "Simple");
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        EndDrawing();
    }
    NN_PRINT(nn);

    return 0;
}
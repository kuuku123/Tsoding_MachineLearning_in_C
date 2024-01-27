#include <stdio.h>

#define NN_IMPLEMENTATION
#define NN_ENABLE_GYM
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

    int w = 800;
    int h = 600;

    InitWindow(w, h, "Simple");
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        gym_render_nn(nn, CLITERAL(Gym_Rect) {0, 0, w, h});
        EndDrawing();
    }
    NN_PRINT(nn);

    return 0;
}
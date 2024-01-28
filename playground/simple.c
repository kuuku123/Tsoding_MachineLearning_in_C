#define NN_IMPLEMENTATION
#define NN_ENABLE_GYM
#include "nn.h"
#include <assert.h>
#include <stdlib.h>
#include <raylib.h>
#include <math.h>
#include <stdio.h>

int main() 
{
    size_t arch[] = {2, 3, 4, 2};
    NN nn = nn_alloc(arch, ARRAY_LEN(arch));
    nn_rand(nn, -1, 1);

    int w = 800;
    int h = 600;

    NN_PRINT(nn);
    InitWindow(w, h, "Simple");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        Gym_Rect r = CLITERAL(Gym_Rect) {0, 0, w, h};
        gym_layout_begin(GLO_HORZ, r, 2, 0);
            gym_render_nn(nn, gym_layout_slot());
            gym_render_nn_as_cake(nn, gym_layout_slot());
        gym_layout_end();
        EndDrawing();
    }

    return 0;
}
#define NN_IMPLEMENTATION
#define NN_ENABLE_GYM
#include "nn.h"
#include <assert.h>
#include <stdlib.h>
#include <raylib.h>
#include <math.h>
#include <stdio.h>


void gym_render_weights(Gym_Rect r, Mat m)
{
    Color low_color  = {0xFF , 0x00, 0xFF, 0xFF};
    Color high_color = {0x00 , 0xFF, 0x00, 0xFF};

    float cell_width = r.w/m.cols;
    float cell_height = r.h/m.rows;

    for (size_t y = 0; y < m.rows; ++y) {
        for (size_t x = 0; x < m.cols; ++x) {
            float alpha = sigmoidf(MAT_AT(m, y, x));
            high_color.a = floorf(255.f * alpha);
            Color color = ColorAlphaBlend(low_color, high_color, WHITE);
            DrawRectangle(
                    ceilf(r.x + x * cell_width),
                    ceilf(r.y + y * cell_height), 
                    ceilf(cell_width), 
                    ceilf(cell_height), 
                    color
                );
        }
    }
}

void gym_render_nn_as_cake(Gym_Rect r, NN nn) 
{
    gym_layout_begin(GLO_VERT, r, nn.count, 20);
    for (size_t i = 0; i < nn.count; ++i) {
        gym_render_weights(gym_layout_slot(), nn.ws[i]);
    }
    gym_layout_end();
}


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
            gym_render_nn_as_cake(gym_layout_slot(), nn);
        gym_layout_end();
        EndDrawing();
    }

    return 0;
}
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <math.h>


#define NN_IMPLEMENTATION
#define NN_ENABLE_GYM
#include "nn.h"

void widget(Gym_Rect r, Color c) 
{
    Rectangle rr = {
        ceilf(r.x), ceilf(r.y), ceilf(r.w), ceilf(r.h)
    };
    Vector2 position = GetMousePosition();
    if (CheckCollisionPointRec(position, rr)) {
        c = ColorBrightness(c, 0.75f);
    }
    DrawRectangleRec(rr, c);
}

int main(void) 
{
    size_t factor = 80;
    size_t width = 16 * factor;
    size_t height = 9 * factor;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(width, height, "Layout");
    SetTargetFPS(60);

    Gym_Layout_Stack ls = {0};

    while (!WindowShouldClose()) {
        float w = GetRenderWidth();
        float h = GetRenderHeight();
        float frame = h * 0.2;
        float gap = 10.0f;
        BeginDrawing();
            ClearBackground(BLACK);
            gym_layout_begin(GLO_HORZ, gym_rect(0, frame, w, h - 2*frame), 3, gap);
                widget(gym_layout_slot(), RED);
                widget(gym_layout_slot(), BLUE);
                gym_layout_begin(GLO_VERT, gym_layout_slot(), 3, gap);
                    gym_layout_begin(GLO_HORZ, gym_layout_slot(), 2, gap);
                        gym_layout_begin(GLO_VERT, gym_layout_slot(), 2, gap);
                           widget(gym_layout_slot(), GREEN);
                           gym_layout_begin(GLO_HORZ, gym_layout_slot(), 2, gap);
                              widget(gym_layout_slot(), GREEN);
                              widget(gym_layout_slot(), GREEN);
                           gym_layout_end();
                        gym_layout_end();
                        widget(gym_layout_slot(), PURPLE);
                    gym_layout_end();
                    gym_layout_begin(GLO_HORZ, gym_layout_slot(), 3, gap);
                        widget(gym_layout_slot(), YELLOW);
                        widget(gym_layout_slot(), YELLOW);
                        widget(gym_layout_slot(), YELLOW);
                    gym_layout_end();
                    widget(gym_layout_slot(), PURPLE);
                gym_layout_end();
            gym_layout_end();
        EndDrawing();

        assert(ls.count == 0);
    }

    CloseWindow();

    return 0;
}
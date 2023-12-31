#include <assert.h>
#include <stdio.h>
#include <raylib.h>

typedef struct {
    float x;
    float y;
    float w;
    float h;
} Layout_Rect;

void widget(Layout_Rect r, Color c) 
{
    DrawRectangle(r.x, r.y, r.w, r.h, c);
    printf("widget(%f, %f,%f,%f)\n", r.x, r.y, r.w, r.h);
}

typedef enum {
    LO_HORZ,
    LO_VERT,
} Layout_Orient;

typedef struct {
    Layout_Orient orient;
    Layout_Rect rect;
    size_t i;
    size_t count;
} Layout;

Layout_Rect layout_slot(Layout *l) 
{
    assert(l-> i < l-> count);
    Layout_Rect r = {0};


    switch (l -> orient) {
        case LO_HORZ:
            r.w = l-> rect.w / l-> count;
            r.h = l-> rect.h;
            r.x = l-> rect.x + l->i * r.w;
            r.y = l-> rect.y;
        break;
        
        case LO_VERT:
            r.w = l-> rect.w;
            r.h = l-> rect.h / l-> count;
            r.x = l-> rect.x;
            r.y = l-> rect.y + l->i * r.h;
        break;

        default:
            assert(0 && "Unreachable");
    }
    l-> i += 1;
    return r;
}

int main(void) 
{
    size_t factor = 80;
    size_t width = 16 * factor;
    size_t height = 9 * factor;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(width, height, "Layout");
    SetTargetFPS(60);


    while (!WindowShouldClose()) {
        int w = GetRenderWidth();
        int h = GetRenderHeight();

        Layout root = {
            .orient = LO_HORZ,
            .rect = {0, 0, w, h},
            .count = 3,
        };

        BeginDrawing();
            ClearBackground(BLACK);
            widget(layout_slot(&root),RED);
            widget(layout_slot(&root),BLUE);
            Layout panel = {
                .orient = LO_VERT,
                .rect = layout_slot(&root),
                .count = 3,
            };
            widget(layout_slot(&panel),GREEN);
            widget(layout_slot(&panel),YELLOW);
            widget(layout_slot(&panel),MAGENTA);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
#include <stdio.h>
#include "raylib.h"

int main()
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "urmom");

    SetTargetFPS(60);

    while (!WindowShouldClose()) 
    {

        BeginDrawing();
        {

            ClearBackground(RAYWHITE);
            DrawCircle(screenWidth/2 , screenHeight/2, 100, RED);
            // DrawText("Ur Mom", screenWidth/2, screenHeight/2, 60, LIGHTGRAY);

        }

        EndDrawing();

    }

    CloseWindow();
    return 0;
}
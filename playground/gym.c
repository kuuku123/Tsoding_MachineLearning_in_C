// Gym is GUI app that trains your NN on the dat you give it
// The idea is that ti will spit out a binary file that can be
// then loaded up with nn.h and used in your application
#include <stdio.h>
#include "raylib.h"
#define SV_IMPLEMENTATION
#include "sv.h"

#define IMG_FACTOR 100
#define IMG_WIDTH (16 * IMG_FACTOR)
#define IMG_HEIGHT (9 * IMG_FACTOR)

int main(int argc, char** argv)
{   
    unsigned int buffer_len = 0;
    unsigned char* buffer = LoadFileData("adder.arch", &buffer_len);

    String_View content = sv_from_parts((const char*) buffer, buffer_len);

    printf("--------------------------------------\n");
    while (content.count > 0) {
        String_View layer = sv_chop_by_delim(&content, ' ');
        printf(SV_Fmt"\n", SV_Arg(layer));
    }
    printf("--------------------------------------\n");

    // InitWindow(IMG_WIDTH, IMG_HEIGHT, "gym");
    // SetTargetFPS(60);

    while (!WindowShouldClose()) {
        
    }
}
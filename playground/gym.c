// Gym is GUI app that trains your NN on the dat you give it
// The idea is that ti will spit out a binary file that can be
// then loaded up with nn.h and used in your application
#include <assert.h>
#include <stdio.h>
#include "raylib.h"
#define SV_IMPLEMENTATION
#include "sv.h"
#define NN_IMPLEMENTATION
#include "nn.h"

#define IMG_FACTOR 100
#define IMG_WIDTH (16 * IMG_FACTOR)
#define IMG_HEIGHT (9 * IMG_FACTOR)

typedef struct {
    size_t* items;
    size_t count;
    size_t capacity;
    
} Arch;

#define DA_INIT_CAP 256
#define da_append(da, item)                                                          \
    do {                                                                             \
        if ((da)->count >= (da)->capacity) {                                         \
            (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;   \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items)); \
            assert((da)->items != NULL && "Buy more RAM lol");                       \
        }                                                                            \
                                                                                     \
        (da)->items[(da)->count++] = (item);                                         \
    } while (0)

int main(int argc, char** argv)
{   
    unsigned int buffer_len = 0;
    unsigned char* buffer = LoadFileData("adder.arch", &buffer_len);

    String_View content = sv_from_parts((const char*) buffer, buffer_len);

    Arch arch = {0};

    content = sv_trim_left(content);
    while (content.count > 0 && isdigit(content.data[0])) {
        int x = sv_chop_u64(&content);
        da_append(&arch, x);
        printf("%d\n", x);
        content = sv_trim_left(content);
    }

    NN nn = nn_alloc(arch.items, arch.count);
    nn_rand(nn, 0, 1);
    NN_PRINT(nn);

    // InitWindow(IMG_WIDTH, IMG_HEIGHT, "gym");
    // SetTargetFPS(60);

}
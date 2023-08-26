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

char *args_shift(int* argc, char*** argv) 
{
    assert(*argc > 0);
    char* result = (**argv);
    (*argc) -= 1;
    (*argv) += 1;
    return result;
}

void nn_render_raylib(NN nn)
{
    Color background_color = { 0x18, 0x18, 0x18, 0xFF};
    Color low_color  = {0xFF , 0x00, 0xFF, 0xFF};
    Color high_color = {0x00 , 0xFF, 0x00, 0xFF};

    ClearBackground(background_color);

    int neuron_radius = 25;
    int layer_border_vpad = 50;
    int layer_border_hpad = 50;
    int nn_width = IMG_WIDTH- 2*layer_border_hpad;
    int nn_height = IMG_HEIGHT - 2 * layer_border_vpad;
    int nn_x = IMG_WIDTH/2 - nn_width/2;
    int nn_y = IMG_HEIGHT/2 - nn_height/2;

    size_t arch_count = nn.count + 1;
    int layer_hpad = nn_width / arch_count;
    for (size_t l = 0; l < arch_count; ++l) {
        int layer_vpad1 = nn_height / nn.as[l].cols;
        for (size_t i = 0; i < nn.as[l].cols; ++i) {
            int cx1 = nn_x + l * layer_hpad + layer_hpad/2;
            int cy1 = nn_y + i * layer_vpad1 + layer_vpad1/2 ;

            if (l+1 < arch_count) {
                int layer_vpad2 = nn_height / nn.as[l+1].cols;
                for (size_t j = 0; j < nn.as[l+1].cols; ++j) {
                    int cx2 = nn_x + (l+1) * layer_hpad + layer_hpad/2;
                    int cy2 = nn_y + j * layer_vpad2 + layer_vpad2/2;
                    float value = sigmoidf(MAT_AT(nn.ws[l], i, j));
                    high_color.a = floorf(255.f * value);
                    float thick = 1.0f * value;
                    Vector2 start = {cx1, cy1};
                    Vector2 end = {cx2, cy2};
                    DrawLineEx(start, end, thick, ColorAlphaBlend(low_color, high_color,  WHITE));
                }
            }

            if  (l > 0) {
                high_color.a = floorf(255.f * sigmoidf(MAT_AT(nn.bs[l-1], 0, i)));
                DrawCircle(cx1, cy1, neuron_radius, ColorAlphaBlend(low_color, high_color, WHITE));
            } else {
                DrawCircle(cx1, cy1, neuron_radius, GRAY); 
            }
        }
    }
}

int main(int argc, char** argv)
{   
    const char* program = args_shift(&argc, &argv);
    if (argc <= 0) {
        fprintf(stderr, "Usage: %s <model.arch> <model.mat>\n", program);
        fprintf(stderr, "ERROR: no archtecture file was provided\n");
        return 1;
    }

    const char *arch_file_path = args_shift(&argc, &argv);
    if (argc <= 0) {
        fprintf(stderr, "Usage: %s <model.arch> <model.mat>\n", program);
        fprintf(stderr, "ERROR: no data file was provided\n");
        return 1;
    }

    const char* data_file_path = args_shift(&argc, &argv);

    unsigned int buffer_len = 0;
    unsigned char* buffer = LoadFileData(arch_file_path, &buffer_len);
    if (buffer == NULL) {
        return 1;
    }

    String_View content = sv_from_parts((const char*) buffer, buffer_len);

    Arch arch = {0};

    content = sv_trim_left(content);
    while (content.count > 0 && isdigit(content.data[0])) {
        int x = sv_chop_u64(&content);
        da_append(&arch, x);
        printf("%d\n", x);
        content = sv_trim_left(content);
    }

    FILE* in = fopen(data_file_path, "rb");
    if (in == NULL) {
        fprintf(stderr, "ERROR: couldn't read file %s\n", data_file_path);
    }
    Mat t = mat_load(in);
    MAT_PRINT(t);
    fclose(in);

    NN_ASSERT(arch.count > 1);
    size_t ins_sz = arch.items[0];
    size_t outs_sz = arch.items[arch.count-1];
    printf("t.cols %zu\n",t.cols);
    printf("ins = %zu\n",ins_sz);
    printf("outs = %zu\n",outs_sz);
    NN_ASSERT(t.cols == ins_sz + outs_sz);

    Mat ti = {
        .rows = t.rows,
        .cols = ins_sz,
        .stride = t.stride,
        .es = &MAT_AT(t,0,0),
    };

    Mat to = {
        .rows = t.rows,
        .cols = outs_sz,
        .stride = t.stride,
        .es = &MAT_AT(t,0,ins_sz),
    };


    NN nn = nn_alloc(arch.items, arch.count);
    NN g = nn_alloc(arch.items, arch.count);
    nn_rand(nn, 0, 1);
    NN_PRINT(nn);

    float rate = 1;

    InitWindow(IMG_WIDTH, IMG_HEIGHT, "gym");
    SetTargetFPS(60);

    size_t i = 0;
    while (!WindowShouldClose()) {
        if (i < 5000) {
            nn_backprop(nn, g, ti, to);
            nn_learn(nn, g, rate);
            i += 1;
            printf("%zu: c = %f\n", i, nn_cost(nn, ti, to));
        }

        BeginDrawing();
        nn_render_raylib(nn);
        EndDrawing();
    }


}
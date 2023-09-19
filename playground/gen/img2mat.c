#include <assert.h>
#include <stdio.h>
#include "raylib.h"
#include <limits.h>
#include <float.h>

#include "stb_image.h"

#define NN_IMPLEMENTATION
#include "nn.h"


typedef struct {
    float *items;
    size_t count;
    size_t capacity;
} Cost_Plot;

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


void nn_render_raylib(NN nn,int rx, int ry, int rw, int rh)
{
    Color low_color  = {0xFF , 0x00, 0xFF, 0xFF};
    Color high_color = {0x00 , 0xFF, 0x00, 0xFF};


    float neuron_radius = rh*0.04;
    int layer_border_vpad = 50;
    int layer_border_hpad = 50;
    int nn_width = rw - 2*layer_border_hpad;
    int nn_height = rh - 2 * layer_border_vpad;
    int nn_x = rx + rw/2 - nn_width/2;
    int nn_y = ry + rh/2 - nn_height/2;

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
                    float thick = rh * 0.004f;
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

void cost_plot_minmax(Cost_Plot plot, float *min, float *max) 
{
    *min = FLT_MAX;
    *max = FLT_MIN;
    for (size_t i = 0; i < plot.count; ++i) {
        if (*max < plot.items[i]) *max = plot.items[i];
        if (*min > plot.items[i]) *min = plot.items[i];
        
    }
}

void plot_cost(Cost_Plot plot, int rx, int ry, int rw, int rh) 
{
    float min, max;
    cost_plot_minmax(plot, &min, &max);
    if (min > 0) min = 0;
    size_t n = plot.count;
    if (n < 1000) n = 1000;
    for (size_t i = 0 ; i+1 < plot.count; ++i) {
        float x1 = rx + (float)rw / n * i;
        float y1 = ry + (1 - (plot.items[i] - min) / (max - min)) * rh;
        float x2 = rx + (float)rw / n * (i+1);
        float y2 = ry + (1 - (plot.items[i+1] - min) / (max - min)) * rh;
        DrawLineEx((Vector2) {x1, y1}, (Vector2){x2 ,y2}, rh*0.007,RED);
    }
}

int main(int argc, char **argv)
{
    const char *program = args_shift(&argc, &argv);

    if (argc <= 0) {
        fprintf(stderr, "Usage: %s <input.png>\n", program);
        fprintf(stderr, "ERROR: no input file is provided\n");
        return 1;
    }

    const char *img_file_path = args_shift(&argc, &argv);

    int img_width, img_height, img_comp; // img_comp = amount of bytes per pixel
    uint8_t *img_pixels = (uint8_t *)stbi_load(img_file_path, &img_width, &img_height, &img_comp, 0);
    if (img_pixels == NULL) {
        fprintf(stderr, "ERROR: could not read image %s\n", img_file_path);
        return 1;
    }
    if (img_comp != 1) {
        fprintf(stderr, "ERROR: %s is %d bits image. Only 8 bit grayscale are supproted\n", img_file_path, img_comp*8);
        return 1;
    }

    printf("%s size %dx%d %d bits\n", img_file_path, img_width, img_height, img_comp*8);

    Mat t = mat_alloc(img_width * img_height, 3); // column is 3 becuase we have x coordinate,y coordinate,b brightness

    for (int y = 0 ; y < img_height; ++y) {
        for (int x = 0;  x < img_width; ++x) {
            size_t i = y * img_width + x;
            float nx = (float) x/(img_width - 1);
            float ny = (float) y/(img_height - 1);
            float nb = img_pixels[i] / 255.f;
            MAT_AT(t, i, 0) = nx;
            MAT_AT(t, i ,1) = ny;
            MAT_AT(t, i, 2) = nb;
        }
    }

    Mat ti = {
        .rows = t.rows,
        .cols = 2,
        .stride = t.stride,
        .es = &MAT_AT(t, 0, 0),
    };

    Mat to = {
        .rows = t.rows,
        .cols = 1,
        .stride = t.stride,
        .es = &MAT_AT(t, 0, ti.cols),
    };

    // MAT_PRINT(ti);
    // MAT_PRINT(to);

    size_t arch[] = {2, 7, 4, 1};
    NN nn = nn_alloc(arch, ARRAY_LEN(arch));
    NN g = nn_alloc(arch, ARRAY_LEN(arch));
    nn_rand(nn, -1, 1);

    size_t WINDOW_FACTOR = 100;
    size_t WINDOW_WIDTH = (16 * WINDOW_FACTOR);
    size_t WINDOW_HEIGHT = (9 * WINDOW_FACTOR);

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "gym");
    SetTargetFPS(60);

    Cost_Plot plot = {0};

    size_t epoch = 0;
    size_t max_epoch = 10 * 10000;
    size_t epochs_per_frame = 120;
    float rate = 1.0f;
    float cost_value = 0;
    bool paused = false;
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) {
            paused = !paused;
        }
        if (IsKeyPressed(KEY_R)) {
            epoch = 0;
            nn_rand(nn, -1, 1);
            plot.count = 0;
        }

        for (size_t i = 0; i < epochs_per_frame && !paused  && epoch < max_epoch; ++i) {
            nn_backprop(nn, g, ti, to);
            nn_learn(nn, g, rate);
            epoch += 1;
            cost_value = nn_cost(nn, ti, to);
            da_append(&plot, cost_value);
        }

        BeginDrawing();
        Color background_color = { 0x18, 0x18, 0x18, 0xFF};
        ClearBackground(background_color);
        {
            int rw, rh, rx, ry;

            int cw = GetRenderWidth();
            int ch = GetRenderHeight();

            rw = cw/3;
            rh = ch*2/3;
            rx = 0;
            ry = ch/2 - rh/2;
            int ceil = ry - 10;
            int floor = ry+rh+10;
            plot_cost(plot,rx, ry, rw, rh);
            DrawLineEx((Vector2) {0, ceil}, (Vector2){rw ,ceil}, rh*0.007,GREEN);
            DrawLineEx((Vector2) {0, floor}, (Vector2){rw ,floor}, rh*0.007,GREEN);

            rx += rw;

            nn_render_raylib(nn, rx, ry, rw, rh);
            char buffer[256];
            snprintf(buffer,sizeof(buffer),"Epoch: %zu/%zu Rate: %f", epoch, max_epoch, rate);
            DrawText(buffer, 0, 0, ch * 0.04, WHITE);
            char costBuffer[256];
            snprintf(costBuffer,sizeof(costBuffer),"Cost: %f", cost_value);
            DrawText(costBuffer, 0, 50, ch * 0.04, WHITE);

        }
        EndDrawing();
    }

    // for (size_t epoch = 0; epoch < max_epoch; ++epoch) {
    //     nn_backprop(nn, g, ti, to);
    //     nn_learn(nn, g, rate);
    //     float cost = nn_cost(nn, ti, to);
    //     if (epoch % 100 == 0) {
    //         printf("%zu: cost = %f\n", epoch, nn_cost(nn, ti, to));
    //     }
    // }

    // print origin
    for (size_t y = 0; y < (size_t)img_height; ++y) {
        for (size_t x = 0; x < (size_t)img_width; ++x) {
            uint8_t pixel = img_pixels[y * img_width + x];
            if (pixel) {
                printf("%3u ", pixel);
            }
            else {
                printf("    ");
            }
        }
        printf("\n");
    }

    // print trained 
    for (size_t y = 0; y < (size_t)img_height; ++y) {
        for (size_t x = 0; x < (size_t)img_width; ++x) {
            MAT_AT(NN_INPUT(nn), 0, 0) = (float) x / (img_width -1);
            MAT_AT(NN_INPUT(nn), 0, 1) = (float) y / (img_height -1);
            nn_forward(nn);
            uint8_t pixel = MAT_AT(NN_OUTPUT(nn), 0, 0) * 255.f;
            if (pixel) {
                printf("%3u ", pixel);
            }
            else {
                printf("    ");
            }
        }
        printf("\n");
    }

    size_t out_width = 512;
    size_t out_height= 512;
    uint8_t *out_pixels = malloc(sizeof(*out_pixels) * out_width * out_height);
    assert(out_pixels != NULL);

    for (size_t y = 0; y < (size_t)out_height; ++y) {
        for (size_t x = 0; x < (size_t)out_width; ++x) {
            MAT_AT(NN_INPUT(nn), 0, 0) = (float) x / (out_width -1);
            MAT_AT(NN_INPUT(nn), 0, 1) = (float) y / (out_height -1);
            nn_forward(nn);
            uint8_t pixel = MAT_AT(NN_OUTPUT(nn), 0, 0) * 255.f;
            out_pixels[y * out_width + x]  = pixel;
        }
    }

    const char *out_file_path = "upscaled.png";

    if (!stbi_write_png(out_file_path, out_width, out_height, 1, out_pixels, out_width * sizeof(*out_pixels))) {
        fprintf(stderr, "ERROR: could not save image %s\n", out_file_path);
        return 1;
    }

    printf("Generated %s from %s\n", out_file_path, img_file_path);

    return 0;
}


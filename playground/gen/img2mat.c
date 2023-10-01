#include <assert.h>
#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
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


size_t arch[] = {3, 10, 10, 4, 1};

int main(int argc, char **argv)
{
    const char *program = args_shift(&argc, &argv);

    if (argc <= 0) {
        fprintf(stderr, "Usage: %s <input.png>\n", program);
        fprintf(stderr, "ERROR: no img1 file is provided\n");
        return 1;
    }

    const char *img1_file_path = args_shift(&argc, &argv);

    int img1_width, img1_height, img1_comp; // img1_comp = amount of bytes per pixel
    uint8_t *img1_pixels = (uint8_t *)stbi_load(img1_file_path, &img1_width, &img1_height, &img1_comp, 0);
    if (img1_pixels == NULL) {
        fprintf(stderr, "ERROR: could not read image %s\n", img1_file_path);
        return 1;
    }
    if (img1_comp != 1) {
        fprintf(stderr, "ERROR: %s is %d bits image. Only 8 bit grayscale are supproted\n", img1_file_path, img1_comp*8);
        return 1;
    }

    const char *img2_file_path = args_shift(&argc, &argv);

    int img2_width, img2_height, img2_comp; // img2_comp = amount of bytes per pixel
    uint8_t *img2_pixels = (uint8_t *)stbi_load(img2_file_path, &img2_width, &img2_height, &img2_comp, 0);
    if (img2_pixels == NULL) {
        fprintf(stderr, "ERROR: could not read image %s\n", img2_file_path);
        return 1;
    }
    if (img2_comp != 1) {
        fprintf(stderr, "ERROR: %s is %d bits image. Only 8 bit grayscale are supproted\n", img2_file_path, img2_comp*8);
        return 1;
    }


    printf("%s size %dx%d %d bits\n", img1_file_path, img1_width, img1_height, img1_comp*8);
    printf("%s size %dx%d %d bits\n", img2_file_path, img2_width, img2_height, img2_comp*8);

    NN nn = nn_alloc(arch, ARRAY_LEN(arch));
    NN g = nn_alloc(arch, ARRAY_LEN(arch));

    Mat t = mat_alloc(img1_width * img1_height + img2_width * img2_height, NN_INPUT(nn).cols + NN_OUTPUT(nn).cols); // column is 3 + 1 becuase we have x coordinate,y coordinate, index (0 or 1),b brightness

    for (int y = 0 ; y < img1_height; ++y) {
        for (int x = 0;  x < img1_width; ++x) {
            size_t i = y * img1_width + x;
            float nx = (float) x/(img1_width - 1);
            float ny = (float) y/(img1_height - 1);
            float nb = img1_pixels[i] / 255.f;
            MAT_AT(t, i, 0) = nx;
            MAT_AT(t, i ,1) = ny;
            MAT_AT(t, i, 2) = 0.0f; // indicating index of image (0 or 1)
            MAT_AT(t, i, 3) = nb;
        }
    }

    for (int y = 0 ; y < img2_height; ++y) {
        for (int x = 0;  x < img2_width; ++x) {
            size_t i = img1_width * img1_height + y * img2_width + x; // index need to start after img1 rows
            float nx = (float) x/(img2_width - 1);
            float ny = (float) y/(img2_height - 1);
            float nb = img2_pixels[y * img2_width + x] / 255.f;
            MAT_AT(t, i, 0) = nx;
            MAT_AT(t, i ,1) = ny;
            MAT_AT(t, i, 2) = 1.0f; // indicating index of image (0 or 1)
            MAT_AT(t, i, 3) = nb;
        }
    }

    nn_rand(nn, -1, 1);

    size_t WINDOW_FACTOR = 100;
    size_t WINDOW_WIDTH = (16 * WINDOW_FACTOR);
    size_t WINDOW_HEIGHT = (9 * WINDOW_FACTOR);

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "gym");
    SetTargetFPS(60);

    Cost_Plot plot = {0};

    size_t preview_width = 28;
    size_t preview_height = 28;

    Image preview_image1 = GenImageColor(preview_width, preview_height, BLACK);
    Texture2D preview_texture1 = LoadTextureFromImage(preview_image1);

    Image preview_image2 = GenImageColor(preview_width, preview_height, BLACK);
    Texture2D preview_texture2 = LoadTextureFromImage(preview_image2);

    Image preview_image3 = GenImageColor(preview_width, preview_height, BLACK);
    Texture2D preview_texture3 = LoadTextureFromImage(preview_image3);

    Image original_image1 = GenImageColor(img1_width, img1_height, BLACK);
    for (size_t y = 0; y < (size_t) img1_height; ++y) {
        for (size_t x = 0; x < (size_t) img1_width; ++x) {
            uint8_t pixel = img1_pixels[y * img1_width + x];
            ImageDrawPixel(&original_image1 , x, y, CLITERAL(Color){ pixel, pixel, pixel, 255});
        }
    }
    Texture2D original_texture1 = LoadTextureFromImage(original_image1);

    Image original_image2 = GenImageColor(img2_width, img2_height, BLACK);
    for (size_t y = 0; y < (size_t) img2_height; ++y) {
        for (size_t x = 0; x < (size_t) img2_width; ++x) {
            uint8_t pixel = img2_pixels[y * img2_width + x];
            ImageDrawPixel(&original_image2 , x, y, CLITERAL(Color){ pixel, pixel, pixel, 255});
        }
    }
    Texture2D original_texture2 = LoadTextureFromImage(original_image2);

    size_t epoch = 0;
    size_t max_epoch = 10 * 10000;
    size_t batches_per_frame = 200;
    size_t batch_size = 28;
    size_t batch_count = (t.rows + batch_size - 1) / batch_size;
    size_t batch_begin = 0;
    float average_cost = 0.0f;
    float rate = 1.0f;
    bool paused = true;

    float scroll = 0.5f;
    bool scroll_dragging = false;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) {
            paused = !paused;
        }
        if (IsKeyPressed(KEY_R)) {
            epoch = 0;
            nn_rand(nn, -1, 1);
            plot.count = 0;
        }

        for (size_t i = 0; i < batches_per_frame && !paused  && epoch < max_epoch; ++i) {
            size_t size = batch_size;
            if (batch_begin + batch_size >= t.rows) {
                size = t.rows - batch_begin;
            }

            Mat batch_ti = {
                .rows = batch_size,
                .cols = 3,
                .stride = t.stride,
                .es = &MAT_AT(t, batch_begin, 0),
            };

            Mat batch_to = {
                .rows = batch_size,
                .cols = 1,
                .stride = t.stride,
                .es = &MAT_AT(t, batch_begin, batch_ti.cols),
            };

            nn_backprop(nn, g, batch_ti, batch_to);
            nn_learn(nn, g, rate);
            average_cost += nn_cost(nn, batch_ti, batch_to);
            batch_begin += batch_size;

            if (batch_begin >= t.rows) {
                epoch += 1;
                da_append(&plot, average_cost / batch_count);
                average_cost = 0.0f;
                batch_begin = 0;
                mat_shuffle_rows(t);
            }
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

            rx += rw; // update x coordinate so we can render stuff accordingly

            nn_render_raylib(nn, rx, ry, rw, rh);

            rx += rw;
            
            float scale = rh * 0.01;


            for (size_t y = 0; y < (size_t)preview_height; ++y) {
                for (size_t x = 0; x < (size_t)preview_width; ++x) {
                    MAT_AT(NN_INPUT(nn), 0, 0) = (float) x / (preview_width - 1);
                    MAT_AT(NN_INPUT(nn), 0, 1) = (float) y / (preview_height - 1);
                    MAT_AT(NN_INPUT(nn), 0, 2) = 0.0f;
                    nn_forward(nn);
                    uint8_t pixel = MAT_AT(NN_OUTPUT(nn), 0, 0) * 255.f;
                    ImageDrawPixel(&preview_image1, x, y, CLITERAL(Color) { pixel, pixel, pixel, 255});
                }
            }

            for (size_t y = 0; y < (size_t)preview_height; ++y) {
                for (size_t x = 0; x < (size_t)preview_width; ++x) {
                    MAT_AT(NN_INPUT(nn), 0, 0) = (float) x / (preview_width - 1);
                    MAT_AT(NN_INPUT(nn), 0, 1) = (float) y / (preview_height - 1);
                    MAT_AT(NN_INPUT(nn), 0, 2) = 1.0f;
                    nn_forward(nn);
                    uint8_t pixel = MAT_AT(NN_OUTPUT(nn), 0, 0) * 255.f;
                    ImageDrawPixel(&preview_image2, x, y, CLITERAL(Color) { pixel, pixel, pixel, 255});
                }
            }

            for (size_t y = 0; y < (size_t)preview_height; ++y) {
                for (size_t x = 0; x < (size_t)preview_width; ++x) {
                    MAT_AT(NN_INPUT(nn), 0, 0) = (float) x / (preview_width - 1);
                    MAT_AT(NN_INPUT(nn), 0, 1) = (float) y / (preview_height - 1);
                    MAT_AT(NN_INPUT(nn), 0, 2) = scroll;
                    nn_forward(nn);
                    uint8_t pixel = MAT_AT(NN_OUTPUT(nn), 0, 0) * 255.f;
                    ImageDrawPixel(&preview_image3, x, y, CLITERAL(Color) { pixel, pixel, pixel, 255});
                }
            }

            UpdateTexture(preview_texture1, preview_image1.data);
            DrawTextureEx(preview_texture1, CLITERAL(Vector2) { rx, ry }, 0 , scale, WHITE);
            DrawTextureEx(original_texture1, CLITERAL(Vector2) { rx, ry + img1_height * scale }, 0 , scale, WHITE);

            UpdateTexture(preview_texture2, preview_image2.data);
            DrawTextureEx(preview_texture2, CLITERAL(Vector2) { rx + img1_width * scale, ry }, 0 , scale, WHITE);
            DrawTextureEx(original_texture2, CLITERAL(Vector2) { rx + img1_width * scale, ry + img2_height * scale }, 0 , scale, WHITE);
            
            UpdateTexture(preview_texture3, preview_image3.data);
            DrawTextureEx(preview_texture3, CLITERAL(Vector2) { rx , ry + img2_height * scale * 2 }, 0 , scale, WHITE);

            {
                float pad = rh * 0.05;
                Vector2 size = { img1_width * scale * 2 , rh * 0.02 };
                Vector2 position = { rx, ry + img2_height * scale * 3  + pad };
                DrawRectangleV(position , size, WHITE);
                
                float knob_radius = rh * 0.02;
                Vector2 knob_position = { rx + size.x * scroll, position.y + size.y * 0.5f };
                DrawCircleV(knob_position , knob_radius,RED );

                if (scroll_dragging) {
                    float x = GetMousePosition().x;
                    if (x < position.x) x = position.x;
                    if (x > position.x + size.x) x = position.x + size.x;
                    scroll = (x - position.x) / size.x;
                }

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    Vector2 mouse_position = GetMousePosition();
                    if (Vector2Distance(mouse_position, knob_position) <= knob_radius) {
                        scroll_dragging = true;
                    }
                }

                if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                    scroll_dragging = false;
                }
            }

            char buffer[256];
            snprintf(buffer,sizeof(buffer),"Epoch: %zu/%zu Rate: %f", epoch, max_epoch, rate);
            DrawText(buffer, 0, 0, ch * 0.04, WHITE);
            char costBuffer[256];
            snprintf(costBuffer,sizeof(costBuffer),"Cost: %f", plot.count > 0 ? plot.items[plot.count-1] : 0);
            DrawText(costBuffer, 0, 50, ch * 0.04, WHITE);

        }
        EndDrawing();
    }

    for (size_t y = 0; y < (size_t)img1_height; ++y) {
        for (size_t x = 0; x < (size_t)img1_width; ++x) {
            uint8_t pixel = img1_pixels[y * img1_width + x];
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
    for (size_t y = 0; y < (size_t)img1_height; ++y) {
        for (size_t x = 0; x < (size_t)img1_width; ++x) {
            MAT_AT(NN_INPUT(nn), 0, 0) = (float) x / (img1_width -1);
            MAT_AT(NN_INPUT(nn), 0, 1) = (float) y / (img1_height -1);
            MAT_AT(NN_INPUT(nn), 0, 2) = 0.5f;
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
            MAT_AT(NN_INPUT(nn), 0, 2) = 0.5f;
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

    printf("Generated %s from %s\n", out_file_path, img1_file_path);

    return 0;
}


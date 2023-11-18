#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "raylib.h"
#include "raymath.h"
#include <limits.h>
#include <float.h>

#include "stb_image.h"

#define NN_IMPLEMENTATION
#define NN_ENABLE_GYM
#include "nn.h"

size_t arch[] = {3, 10, 10, 4, 1};
size_t max_epoch = 10 * 10000;
size_t batches_per_frame = 200;
size_t batch_size = 28;
float rate = 1.0f;
bool paused = true;
float scroll = 0.5f;

char *args_shift(int* argc, char*** argv) 
{
    assert(*argc > 0);
    char* result = (**argv);
    (*argc) -= 1;
    (*argv) += 1;
    return result;
}

#define out_width 512
#define out_height 512
uint32_t out_pixels[out_width*out_height];
#define READ_END 0
#define WRITE_END 1
#define FPS 30
#define STR2(x) #x
#define STR(x) STR2(x)

void render_single_out_image(NN nn, float a)
{
    for (size_t y = 0; y < out_height; ++y) {
        for (size_t x = 0; x < out_width; ++x) {
            MAT_AT(NN_INPUT(nn), 0, 0) = (float) x / (out_width- 1);
            MAT_AT(NN_INPUT(nn), 0, 1) = (float) y / (out_height - 1);
            MAT_AT(NN_INPUT(nn), 0, 2) = a;
            nn_forward(nn);
            float activation = MAT_AT(NN_OUTPUT(nn), 0, 0); // now we are using sigmoid as activation , but it might change and 0~1 output will break
            if (activation < 0)  activation = 0;
            if (activation > 1) activation = 1;
            uint32_t bright = activation * 255.f;
            uint32_t pixel = 0xFF000000|bright|(bright<<8)|(bright<<16);
            out_pixels[y*out_width + x] = pixel;
        }
    }
}

int render_upscaled_video(NN nn, float duration, const char *out_file_path)
{
    int pipefd[2];

    if (pipe(pipefd) < 0) {
        fprintf(stderr, "Error: could not create a pipe: %s\n", strerror(errno));
        return 1;
    }

    pid_t child = fork();
    if (child < 0) {
        fprintf(stderr, "Error: could not fork a child: %s\n", strerror(errno));
        return 1;
    }
    if (child == 0) {
        if (dup2(pipefd[READ_END], STDIN_FILENO) < 0) {
            fprintf(stderr, "Error: could not reopen read end of pipe as stdin: %s\n", strerror(errno));
            return 1;
        }
        close(pipefd[WRITE_END]);

        int ret = execlp("ffmpeg", "ffmpeg", 
        "-y",
        "-f", "rawvideo",
        "-pix_fmt", "rgba",
        "-s:v", STR(out_width) "x" STR(out_height),
        "-r", STR(FPS),
        "-an",
        "-i", "-",
        "-c:v", "libx264",
        out_file_path,
        NULL);
        if (ret < 0) {
            fprintf(stderr, "ERROR: could not run ffpmpeg as a child process: %s\n", strerror(errno));
            return 1;
        }
        assert(0 && "unreachable");
    }

    close(pipefd[READ_END]);

    typedef struct {
        float start;
        float end;
    } Segment;

    Segment segments[] = {
        {0, 0},
        {0, 1},
        {1, 1},
        {1, 0},
    };

    size_t segments_count = ARRAY_LEN(segments);
    float segment_length = 1.0f/segments_count;

    size_t frame_count = FPS * duration;
    for (size_t i = 0; i < frame_count; ++i) {
        float a = (float)i / frame_count;
        size_t segment_index = floorf(a / segment_length);
        float segment_progress = a /segment_length - segment_index;
        if (segment_index > segments_count) 
            segment_index = segment_length -1;
        Segment segment = segments[segment_index];
        float b = segment.start + (segment.end - segment.start) * segment_progress;
        render_single_out_image(nn,b);
        write(pipefd[WRITE_END], out_pixels, sizeof(*out_pixels)*out_width*out_height);
        printf("a = %f , index = %zu , progress = %zu, b = %f\n", a,segment_index, segment_progress);
    }

    close(pipefd[WRITE_END]);
    wait(NULL);
    printf("Generateds %s!\n",out_file_path);

    return 0;
}


int render_upscaled_screentshot(NN nn, const char *out_file_path)
{

    render_single_out_image(nn, scroll);

    if (!stbi_write_png(out_file_path, out_width, out_height, 4, out_pixels, out_width * sizeof(*out_pixels))) {
        fprintf(stderr, "ERROR: could not save image %s\n", out_file_path);
        return 1;
    }

    printf("Generated %s\n", out_file_path);
    return 0;
}

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
    
    Gym_Plot plot = {0};

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

    Gym_Batch gb = {0};
    size_t epoch = 0;
    size_t batch_begin = 0;
    float average_cost = 0.0f;

    bool scroll_dragging = false;
    bool rate_dragging = false;


    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) {
            paused = !paused;
        }
        if (IsKeyPressed(KEY_R)) {
            epoch = 0;
            nn_rand(nn, -1, 1);
            plot.count = 0;
        }
        if (IsKeyPressed(KEY_S)) {
            render_upscaled_screentshot(nn,"upscaled.png");
        }
        if (IsKeyPressed(KEY_X)) {
            render_upscaled_video(nn, 5, "upscaled.mp4");
        }

        for (size_t i = 0; i < batches_per_frame && !paused  && epoch < max_epoch; ++i) {
            gym_process_batch(&gb, batch_size, nn, g, t, rate);
            if (gb.finished) {
                epoch += 1;
                da_append(&plot, gb.cost);
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
            gym_plot(plot,rx, ry, rw, rh);
            DrawLineEx((Vector2) {0, ceil}, (Vector2){rw ,ceil}, rh*0.007,GREEN);
            DrawLineEx((Vector2) {0, floor}, (Vector2){rw ,floor}, rh*0.007,GREEN);

            rx += rw; // update x coordinate so we can render stuff accordingly

            gym_render_nn(nn, rx, ry, rw, rh);

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
            DrawTextureEx(preview_texture1, CLITERAL(Vector2) { rx, ry + img1_height * scale}, 0 , scale, WHITE);
            DrawTextureEx(original_texture1, CLITERAL(Vector2) { rx, ry  }, 0 , scale, WHITE);

            UpdateTexture(preview_texture2, preview_image2.data);
            DrawTextureEx(preview_texture2, CLITERAL(Vector2) { rx + img1_width * scale, ry + img2_height * scale }, 0 , scale, WHITE);
            DrawTextureEx(original_texture2, CLITERAL(Vector2) { rx + img1_width * scale, ry }, 0 , scale, WHITE);
            
            UpdateTexture(preview_texture3, preview_image3.data);
            DrawTextureEx(preview_texture3, CLITERAL(Vector2) { rx , ry + img2_height * scale * 2 }, 0 , 2 * scale, WHITE);

            {
                float pad = rh * 0.05;
                ry = ry + img2_height * scale * 4 + pad;
                rw = img1_width * scale *2;
                rh = rh * 0.02;
                gym_slider(&scroll, &scroll_dragging, rx ,ry, rw, rh);
            }

            char buffer[256];
            snprintf(buffer,sizeof(buffer),"Epoch: %zu/%zu Rate: %f", epoch, max_epoch, rate);
            DrawText(buffer, 0, 0, ch * 0.04, WHITE);
            char costBuffer[256];
            snprintf(costBuffer,sizeof(costBuffer),"Cost: %f", plot.count > 0 ? plot.items[plot.count-1] : 0);
            DrawText(costBuffer, 0, 50, ch * 0.04, WHITE);
            gym_slider(&rate, &rate_dragging, 0, ch*0.08, cw, ch*0.02);

        }
        EndDrawing();
    }

    // for (size_t y = 0; y < (size_t)img1_height; ++y) {
    //     for (size_t x = 0; x < (size_t)img1_width; ++x) {
    //         uint8_t pixel = img1_pixels[y * img1_width + x];
    //         if (pixel) {
    //             printf("%3u ", pixel);
    //         }
    //         else {
    //             printf("    ");
    //         }
    //     }
    //     printf("\n");
    // }

    // // print trained 
    // for (size_t y = 0; y < (size_t)img1_height; ++y) {
    //     for (size_t x = 0; x < (size_t)img1_width; ++x) {
    //         MAT_AT(NN_INPUT(nn), 0, 0) = (float) x / (img1_width -1);
    //         MAT_AT(NN_INPUT(nn), 0, 1) = (float) y / (img1_height -1);
    //         MAT_AT(NN_INPUT(nn), 0, 2) = 0.5f;
    //         nn_forward(nn);
    //         uint8_t pixel = MAT_AT(NN_OUTPUT(nn), 0, 0) * 255.f;
    //         if (pixel) {
    //             printf("%3u ", pixel);
    //         }
    //         else {
    //             printf("    ");
    //         }
    //     }
    //     printf("\n");
    // }


    // for (size_t y = 0; y < (size_t)out_height; ++y) {
    //     for (size_t x = 0; x < (size_t)out_width; ++x) {
    //         MAT_AT(NN_INPUT(nn), 0, 0) = (float) x / (out_width -1);
    //         MAT_AT(NN_INPUT(nn), 0, 1) = (float) y / (out_height -1);
    //         MAT_AT(NN_INPUT(nn), 0, 2) = 0.5f;
    //         nn_forward(nn);
    //         uint8_t pixel = MAT_AT(NN_OUTPUT(nn), 0, 0) * 255.f;
    //         out_pixels[y * out_width + x]  = pixel;
    //     }
    // }
    
    return 0;
}





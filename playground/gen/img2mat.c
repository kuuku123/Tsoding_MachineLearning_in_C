#include <assert.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define NN_IMPLEMENTATION
#include "nn.h"


char *args_shift(int* argc, char*** argv) 
{
    assert(*argc > 0);
    char* result = (**argv);
    (*argc) -= 1;
    (*argv) += 1;
    return result;
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
    MAT_PRINT(t);

    const char *out_file_path = "img.mat";
    FILE *out = fopen(out_file_path, "wb");
    if (out == NULL) {
        fprintf(stderr, "ERROR: could not open file %s\n", out_file_path);
        return 1;
    }

    mat_save(out, t);
    printf("Generated %s from %s\n", out_file_path, img_file_path);

    return 0;
}


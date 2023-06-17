#define NN_IMPLEMENTATION
#include "framework/nn.h"  

#define BITS 2

int main(void) 
{
    size_t n = (1 << BITS);
    size_t rows= (n) * (n);
    Mat ti = mat_alloc(rows, 2*BITS);
    Mat to = mat_alloc(rows, BITS + 1);

    for (size_t i = 0; i < ti.rows; ++i) {
        size_t x = i/n;
        size_t y = i%n;
        size_t z = x + y;
        size_t overflow = z >= n;
        for (size_t j = 0; j < BITS; ++j) {
            MAT_AT(ti, i, BITS - 1 -j)        = (x>>j)&1;
            MAT_AT(ti, i, ti.cols - 1 - j) = (y>>j)&1;
            MAT_AT(to, i, BITS - 1 - j)        = (z>>j)&1;
        }
        MAT_AT(to, i, BITS) = overflow;
    }

    MAT_PRINT(ti);
    MAT_PRINT(to);


    size_t arch[] = {2*BITS, 2 * BITS, BITS + 1};
    NN nn = nn_alloc(arch, ARRAY_LEN(arch));
    NN g = nn_alloc(arch, ARRAY_LEN(arch));
    nn_rand(nn, 0, 1);
    // NN_PRINT(nn);

    float rate = 1;

    // printf("c = %f\n", nn_cost(nn, ti, to));
    for (size_t i = 0; i < 10 *1000; ++i) {
        nn_backprop(nn, g, ti, to);
        nn_learn(nn, g, rate);
        // printf("%zu: c = %f\n", i, nn_cost(nn, ti, to));
    }

    for (size_t x = 0; x < n; ++x) {
        for (size_t y = 0; y < n; ++y) {
            printf("%zu + %zu = ", x, y);
            for (size_t j = 0; j < BITS; ++j) {
                MAT_AT(NN_INPUT(nn), 0, BITS -1 -j)    = (x>>j)&1;
                MAT_AT(NN_INPUT(nn), 0, ti.cols -1 -j) = (y>>j)&1;
            }
            nn_forward(nn);
            if (MAT_AT(NN_OUTPUT(nn), 0, BITS) > 0.5f) {
                printf("OVERFLOW\n");
            } else {
                size_t z = 0;
                for (size_t j = 0; j < BITS; ++j) {
                    size_t bit = MAT_AT(NN_OUTPUT(nn), 0, j) > 0.5f;
                    z |= bit << j;
                }
                printf("%zu\n", z);
            }

        }
    }




}
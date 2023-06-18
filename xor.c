#define NN_IMPLEMENTATION
#include <time.h>
#include "framework/nn.h"


typedef struct
{
    float or_w1;
    float or_w2;
    float or_b;

    float nand_w1;
    float nand_w2;
    float nand_b;

    float and_w1;
    float and_w2;
    float and_b;
} Xor;



typedef float sample[3];

sample td_or[] = {
	0, 0 ,0,
	1, 0 ,1,
	0, 1 ,1,
	1, 1 ,1,
};

sample td_and[] = {
	0, 0 ,0,
	1, 0 ,0,
	0, 1 ,0,
	1, 1 ,1,
};

sample td_nand[] = {
	0, 0 ,1,
	1, 0 ,1,
	0, 1 ,1,
	1, 1 ,0,
};
sample td_xor[] = {
	0, 0 ,0,
	1, 0 ,1,
	0, 1 ,1,
	1, 1 ,0,
};


int main(void)
{
    // srand(time(0));
    srand(69);
    float* td = td_or;
    size_t stride = 3;
    size_t n = 4;
    Mat ti = {
        .rows = n,
        .cols = 2,
        .stride = stride,
        .es = td
    };

    Mat to = {
        .rows = n,
        .cols = 1,
        .stride = stride,
        .es = td + 2,
    };

    size_t arch[] = {2,2,1}; // first index is input layer and last index is output layer, all of the others are hidden layer
    NN nn = nn_alloc(arch , ARRAY_LEN(arch));
    NN g = nn_alloc(arch,ARRAY_LEN(arch));
    nn_rand(nn,0,1);

    NN_PRINT(nn);
    return 0;

    float rate = 1;

    printf("cost = %f\n", nn_cost(nn,ti,to));
    for (size_t i = 0; i < 5000; ++i) {
        nn_backprop(nn, g, ti, to);
        nn_learn(nn, g, rate);
        printf("%zu: cost = %f\n", i, nn_cost(nn,ti,to));
    }

    NN_PRINT(nn);

    for (size_t i = 0; i< 2; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            MAT_AT(NN_INPUT(nn), 0, 0) = i;
            MAT_AT(NN_INPUT(nn), 0, 1) = j;
            nn_forward(nn);
            printf("%zu ^ %zu = %f\n", i, j, MAT_AT(NN_OUTPUT(nn), 0, 0));
        }
    }

    return 0;
}
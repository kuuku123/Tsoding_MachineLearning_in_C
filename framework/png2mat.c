#include <assert.h>
#include <stdio.h>


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
}
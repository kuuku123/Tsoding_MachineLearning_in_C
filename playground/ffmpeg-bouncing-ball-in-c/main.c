#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <unistd.h>

int main (void) 
{
    pid_t child = fork();
    if (child < 0) {
        fprintf(stderr, "Error: could not fork a child: %s\n", strerror(errno));
        return 1;
    }
    if (child == 0) {
        printf("Hello from Child\n");
        return 0;
    }

    printf("Hello from Parent. The child's pid is %d\n", child);

    return 0;
}
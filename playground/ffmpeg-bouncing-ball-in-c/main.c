#include <stdio.h>
#include <assert.h>
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
        int ret = execlp("ffmpeg", "ffmpeg", 
        // "-i", "-",
        // "-f", "rawvideo",
        
        NULL);
        if (ret < 0) {
            fprintf(stderr, "ERROR: could not run ffpmpeg as a child process: %s\n", strerror(errno));
            return 1;
        }
        assert(0 && "unreachable");
    }


    printf("Hello from Parent. The child's pid is %d\n", child);

    return 0;
}
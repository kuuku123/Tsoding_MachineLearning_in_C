#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define READ_END 0
#define WRITE_END 1

#define WIDTH 800
#define HEIGHT 600
uint32_t pixels[WIDTH*HEIGHT];  

int main (void) 
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
        "-s:v", "800x600",
        "-r", "60",
        "-an",
        "-i", "-",
        "-c:v", "libx264",
        "output.mp4",
        NULL);
        if (ret < 0) {
            fprintf(stderr, "ERROR: could not run ffpmpeg as a child process: %s\n", strerror(errno));
            return 1;
        }
        assert(0 && "unreachable");
    }

    close(pipefd[READ_END]);

    for (size_t i = 0; i < WIDTH*HEIGHT; ++i) {
        pixels[i] = 0xFF0000FF;

    }
    
    for (size_t i = 0; i < 60; ++i) {
        write(pipefd[WRITE_END], &pixels, sizeof(pixels)*WIDTH*HEIGHT);
    }

    close(pipefd[WRITE_END]);

    wait(NULL);


    printf("Done rendering the video!");

    return 0;
}
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>


int main()
{
    char input_1[30];
    char input_2[30];
    int old_stdout = dup(1);
    int old_stdin = dup(0);

    printf("Input: \n");
    scanf("%s", input_1);
    printf("Input: %s\n", input_1);

    int fd[2];
    pipe(fd);

    printf("STDOUT: %d\n", old_stdout);

    dup2(fd[1], 1);
    printf("Pipe\n");

    dup2(fd[0], 0);
    scanf("%s", input_2);


    dup2(old_stdout, 1);
    dup2(old_stdin, 0);


    printf("Input2: %s\n", input_2);
    return 0;
}
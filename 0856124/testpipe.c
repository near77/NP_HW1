#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>


int main()
{
    int fd = open("test.txt", O_WRONLY);
    dup2(fd, STDOUT_FILENO);
    printf("123\n");
    return 0;
}
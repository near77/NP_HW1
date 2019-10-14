#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#define DELI " "

char **parse_line(char *line, char * delimeters)// parse line with CMD_DELIMETERS
{
    int bufsize = 1024, position = 0;
    char **tokens = (char **)malloc(sizeof(char*) * bufsize);
    char *token;
    if(!tokens)
    {
        printf("Allocation Fail.\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, delimeters);
    while(token != NULL)
    {
        tokens[position] = token;
        position++;
        if(position >= bufsize){
            bufsize += 128;
            tokens = (char **)realloc(tokens, sizeof(char*) * bufsize);
            if(!tokens)
            {
                printf("Allocation Fail.\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, DELI);
    }
    tokens[position] = NULL;
    return tokens;
}

int main()
{
    char * string = "setenv PATH bin";
    char ** args = parse_line(string, DELI);
    int tmp_idx = 0;
    while(args[tmp_idx])
    {
        printf("%s\n", args[tmp_idx]);
        tmp_idx++;
    }
    
    return 0;
}
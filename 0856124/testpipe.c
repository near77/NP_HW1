#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#define DELI " "

char **parse_line(char *line, char * delimeters)// parse line with CMD_DELIMETERS
{
    int bufsize = TOKENBUF_SIZE, position = 0;
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
            bufsize += TOKENBUF_SIZE;
            tokens = (char **)realloc(tokens, sizeof(char*) * bufsize);
            if(!tokens)
            {
                printf("Allocation Fail.\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, CMD_DELIMITERS);
    }
    tokens[position] = NULL;
    return tokens;
}

int main()
{
    char * string = "setenv PATH bin";
    char * args = strtok(string, DELI);
    int tmp_idx = 0;
    while(args[tmp_idx])
    {
        printf("%s\n", args[tmp_idx]);
        tmp_idx++;
    }
    
    return 0;
}
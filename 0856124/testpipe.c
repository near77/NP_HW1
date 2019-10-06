#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#define RL_BUFSIZE 1024
#define TOK_BUFSIZE 64
#define TOK_DELIMITERS " \t\r\n\a"

static inline void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}


char **parse_line(char *line)// parse line with TOK_DELIMETERS
{
    int bufsize = TOK_BUFSIZE, position = 0;
    char **tokens = malloc(sizeof(char*) * bufsize);
    char *token;
    if(!tokens)
    {
        printf("Allocation Fail.\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, TOK_DELIMITERS);
    while(token != NULL)
    {
        tokens[position] = token;
        position++;
        if(position >= bufsize){
            bufsize += TOK_BUFSIZE;
            tokens = realloc(tokens, sizeof(char*) * bufsize);
            if(!tokens)
            {
                printf("Allocation Fail.\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, TOK_DELIMITERS);
    }
    tokens[position] = NULL;
    for (int i = 0;i < sizeof(tokens); i++)
    {
        printf("Tokens: %s\n", tokens[i]);
    }
    
    return tokens;
}

char *read_line(char *line)
{
    int bufsize = RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char)*bufsize);
    int c;
    if(!buffer)
    {
        printf("Allocation Fail.\n");
        exit(EXIT_FAILURE);
    }
    for(int i=0;i<sizeof(line);i++)
    {
        c = line[i];
        if(c == EOF || c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
        {
            buffer[position] = c;
        }
        position++;

        if(position >= bufsize)
        {
            bufsize += RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if(!buffer)
            {
                printf("Allocation Fail.\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

static void
cisshPipe(char **command1, char **command2)
{
    int fd[2];
    pid_t childPid;
    if (pipe(fd) != 0)
        error("failed to create pipe");

    if ((childPid = fork()) == -1)
        error("failed to fork");

    if (childPid == 0)
    {
        dup2(fd[1], 1);
        close(fd[0]);
        close(fd[1]);
        execvp(command1[0], command1);
        error("failed to exec command 1");
    }
    else
    {
        dup2(fd[0], 0);
        close(fd[0]);
        close(fd[1]);
        execvp(command2[0], command2);
        error("failed to exec command 2");
    }
}

int main(void)
{
    char * l = "ls > sort";
    char *line = read_line(l);
    printf("Token: %s\n", line);
    //char **token = parse_line(line);
    //printf("Token: %s\n", *token);
    char *ls[] = { "ls", 0 };
    char *sort[] = { "sort", 0 };
    //cisshPipe(ls, sort);
    return 0;
}
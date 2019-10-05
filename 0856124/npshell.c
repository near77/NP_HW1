#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define RL_BUFSIZE 1024
#define TOK_BUFSIZE 64
#define TOK_DELIMITERS " \t\r\n\a"

//--------built in fuction-------------
char *builtin_str[] = {
    "exit",
    "printenv",
    "setenv"
};

int sh_exit(char **args)
{
    return 0;
}

int sh_printenv(char **args)
{
    printf("%s\n", getenv(args[1]));
}

int sh_setenv(char **args)
{
    setenv("PATH", args[1], 1);
    printf("env had been set.\n");
}

int (*builtin_func[]) (char **) = {
    &sh_exit,
    &sh_printenv,
    &sh_setenv
};

int num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}
//-------------------------------------

int sh_launch(char **args)// execute command
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid==0){

        if(execvp(args[0], args) == -1)
        {
            perror("Shell");
            printf("Unknown command: [%s].\n", args[0]);
        }
        else if(pid < 0)
        {
            perror("Shell");
        }
        else
        {
            do
            {
                wpid = waitpid(pid, &status, WUNTRACED);
            }while(!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }
    return 1;
}

int sh_execute(char **args)// execute built in command
{
    int i;
    if (args[0] == NULL) 
    {
        // An empty command was entered.
        return 1;
    }

    for (i = 0; i < num_builtins(); i++) 
    {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return sh_launch(args);
}

char *read_line(void)
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
    while(1)
    {
        c = getchar();
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
    return tokens;
}

void shell_loop(void)
{
    char *line;
    char **args;
    int status;
    do 
    {
        printf("%% ");
        line = read_line();
        args = parse_line(line);
        status = sh_execute(args);
        free(line);
        free(args);
    }while (status);
}

int main(int argc, char **argv, char *envp[])
{
    //--------initialize---------
    setenv("PATH", "bin:.", 1);
    //---------------------------
    shell_loop();
    return 0;
}
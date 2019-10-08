#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define RL_BUFSIZE 1024
#define TOK_BUFSIZE 64
#define TOK_DELIMITERS " \t\r\n\a"
#define CMD_DELIMITERS "|"

//--------built in function-------------
int sh_exit(char **args)
{
    exit(0);
}

int sh_printenv(char **args)
{
    char * env = getenv(args[1]);
    if(env)
    {
        printf("%s\n", getenv(args[1]));
    }
    return 1;
}

int sh_setenv(char **args)
{
    setenv("PATH", args[1], 1);
    printf("env had been set.\n");
    return 0;
}

char *builtin_str[] = {
    "exit",
    "printenv",
    "setenv"
};

int (*builtin_func[]) (char **) = {
    &sh_exit,
    &sh_printenv,
    &sh_setenv
};

int num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}
//-------------------------------------

static inline void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

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
    else
    {
        wait(NULL);    
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
    //if it's not built in command ->launched by exec 
    return sh_launch(args);
}

char *read_line(void)
{
    int bufsize = RL_BUFSIZE;
    int position = 0;
    char *buffer = (char *)malloc(sizeof(char)*bufsize);
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
            buffer = (char *)realloc(buffer, bufsize);
            if(!buffer)
            {
                printf("Allocation Fail.\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char **parse_cmd(char *line)// parse line with CMD_DELIMETERS
{
    int bufsize = TOK_BUFSIZE, position = 0;
    char **tokens = (char **)malloc(sizeof(char*) * bufsize);
    char *token;
    if(!tokens)
    {
        printf("Allocation Fail.\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, CMD_DELIMITERS);
    while(token != NULL)
    {
        tokens[position] = token;
        position++;
        if(position >= bufsize){
            bufsize += TOK_BUFSIZE;
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

char **parse_line(char *cmd)// parse line with TOK_DELIMETERS
{
    int bufsize = TOK_BUFSIZE, position = 0;
    char **tokens = (char **)malloc(sizeof(char*) * bufsize);
    char *token;
    if(!tokens)
    {
        printf("Allocation Fail.\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(cmd, TOK_DELIMITERS);
    while(token != NULL)
    {
        tokens[position] = token;
        position++;
        if(position >= bufsize){
            bufsize += TOK_BUFSIZE;
            tokens = (char **)realloc(tokens, sizeof(char*) * bufsize);
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
    char **cmd;
    char **args;
    char *cmd_list[256];
    int status;
    pid_t child_pid;
    do 
    {
        int i = 0;
        printf("%% ");
        line = read_line();
        cmd = parse_cmd(line);
        while(cmd[i])// calculate number of command
        {
            args = parse_line(cmd[i]);
            cmd_list[i] = args;
            i++;
        }
        if(i > 1)
        {
            int fd[i][2];
            int j = 0;
            while(cmd_list[j])
            {
                pipe(fd[j]);
                if ((child_pid = fork()) == -1)
                    printf("failed to fork\n");
                    exit(EXIT_FAILURE);
                if(child_pid == 0)
                {
                    if(j == 0)
                    {
                        dup2(fd[j][1], STDOUT_FILENO);
                        close(fd[j][1]);
                    }
                    else if(j == i-1)
                    {
                        dup2(fd[j-1][0], STDIN_FILENO);
                        close(fd[j-1][0]);
                    }
                    else
                    {
                        dup2(fd[j-1][0], STDIN_FILENO);
                        dup2(fd[j][1], STDOUT_FILENO);
                        close(fd[j-1][0]);
                        close(fd[j][1]);
                    }
                    execvp(cmd_list[j][0], cmd_list[j]);
                    exit(0);
                }
                else
                {
                    close(fd[j][1]);
                    int status_child;
                    waitpid(child_pid, &status_child, 0);
                }
                j++;
            }
        }
        else if(i == 1)// input single command
        {
            status = sh_execute(args);
        }
        else// input nothing
        {
            continue;
        }
        free(line);
        free(cmd);
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
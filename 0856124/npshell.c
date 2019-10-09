#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#define READBUF_SIZE 1024
#define TOKENBUF_SIZE 128
#define CMD_DELIMITERS "|"
#define TOK_DELIMITERS " "

//----------------built in functions------------------
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
//---------------------------------------------------


struct number_pipe
{
    int target_cmd_num;
    int *fd;
};

char *read_line()
{
    int bufsize = READBUF_SIZE;
    int line_position = 0;
    int c;
    char *buffer = malloc(sizeof(char)*bufsize);
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
            buffer[line_position] = '\0';
            return buffer;
        }
        else
        {
            buffer[line_position] = c;
        }
        line_position++;
        if(line_position >= bufsize)
        {
            bufsize += READBUF_SIZE;
            buffer = realloc(buffer, bufsize);
            if(!buffer)
            {
                printf("Allocation Fail.\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

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


void shell_loop()
{
    char *line;// input
    char **cmd;// cmd
    char **args;// cmd arguments
    char **exe_args;// exec cmd
    struct number_pipe pipe_num[1000];// record numbered pipe
    int numpipe_idx = 0;// record numbered pipe list idx
    int cmd_no = 1; //record ?th cmd is executing
    int status = 1;
    pid_t pid;
    do
    {
        printf("<o> ");
        line = read_line();
        cmd = parse_line(line, CMD_DELIMITERS);
        int num_of_cmd = 0;
        while(cmd[num_of_cmd])// calculate number of command
        {
            num_of_cmd++;
        }
        int fd[num_of_cmd][2];// Fd for pipe
        int cmd_idx = 0;
        while(cmd_idx < num_of_cmd)
        {
            args = parse_line(cmd[cmd_idx], TOK_DELIMITERS);
            if(isdigit(cmd[cmd_idx][0]))
            {
                exe_args = args + 1;
            }
            else
            {
                exe_args = args;
            }
            //----------------Check if cmd is built in function----------------------
            int builtin_flag = 0;
            if (args[0] == NULL) 
            {
                // An empty command was entered.
                continue;
            }
            for (int i = 0; i < num_builtins(); i++) 
            {
                if (strcmp(args[0], builtin_str[i]) == 0) {
                    builtin_flag = 1;
                    (*builtin_func[i])(args);// Execute built in command
                    break;
                }
            }
            if(builtin_flag == 1)
            {
                cmd_no++;
                cmd_idx++;
                continue;
            }
            //----------------------------------------------------------------------
            int stdin_fd = dup(0);
            int stdout_fd = dup(1);
            //----------------Check if there are num pipe---------------------------
            if(cmd[cmd_idx+1])
            {
                if(isdigit(cmd[cmd_idx+1][0]))// record every num cmd
                {
                    pipe(fd[cmd_idx]);
                    printf("ls pipe: <in> %d <out> %d\n", fd[cmd_idx][0], fd[cmd_idx][1]);
                    stdin_fd = fd[cmd_idx][0];
                    stdout_fd = fd[cmd_idx][1];
                    pipe_num[numpipe_idx].target_cmd_num = cmd_no + ((int)cmd[cmd_idx+1][0]-48);
                    // start from previous cmd
                    pipe_num[numpipe_idx].fd = fd[cmd_idx];// current cmd's fd
                    printf("numpipe recorded| target: %d | fd: %d %d\n", \
                        pipe_num[numpipe_idx].target_cmd_num, pipe_num[numpipe_idx].fd[0], pipe_num[numpipe_idx].fd[1]);
                }
                else
                {
                    //Need to check if next cmd is someone's target.
                    int next_cmd_is_target = 0;
                    int i;
                    for(i = 0; i<numpipe_idx; i++)
                    {
                        if(cmd_no+1 == pipe_num[i].target_cmd_num)
                        {
                            stdout_fd = pipe_num[i].fd[1];
                            next_cmd_is_target = 1;
                            break;
                        }
                    }
                    pipe_num[numpipe_idx].target_cmd_num = cmd_no + 1;
                    if(next_cmd_is_target == 0)// next cmd is not someone's target
                    {
                        pipe(fd[cmd_idx]);
                        stdout_fd = fd[cmd_idx][1];
                        pipe_num[numpipe_idx].fd = fd[cmd_idx];// current cmd's fd
                    }
                    else// next cmd is someone's target
                    {
                        // redirect pipe to previous source of next cmd.
                        stdin_fd = pipe_num[i].fd[0];
                        stdout_fd = pipe_num[i].fd[1];
                        pipe_num[numpipe_idx].fd = pipe_num[i].fd;
                    }
                }
                numpipe_idx ++;
                numpipe_idx %= 1000;
            }
            
            
            //---------------------------------------------------------------------

            //----------------Check if current cmd is target-----------------------
            for(int i = 0; i<numpipe_idx; i++)
            {
                if(cmd_no == pipe_num[i].target_cmd_num)
                {
                    stdin_fd = pipe_num[i].fd[0];
                    printf("exec pipe\n");
                    break;
                }
            }
            //---------------------------------------------------------------------

            //-----------------Start forking child---------------------------------
            if ((pid = fork()) == -1)
            {
                printf("failed to fork\n");
                exit(EXIT_FAILURE);
            }
            if(pid == 0)
            {
                // -----dup close issue--------------
                dup2(stdin_fd, STDIN_FILENO);
                dup2(stdout_fd, STDOUT_FILENO);
                printf("used pipe: <in> %d <out> %d\n", stdin_fd, stdout_fd);
                close(stdin_fd);
                close(stdout_fd);
                int idx = 0;
                while(exe_args[idx])
                {
                    printf("exec_arg[0]: %s\n", exe_args[idx]);
                    idx++;
                }
                execvp(exe_args[0], exe_args);
                exit(0);
            }
            else
            {
                close(stdout_fd);
                close(stdin_fd);
                int status_child;
                waitpid(pid, &status_child, 0);
            }

            //---------------------------------------------------------------------
            cmd_idx++;
            cmd_no++;
        }
    }while(status);
}

int main()
{
    //--------initialize---------
    setenv("PATH", "bin:.", 1);
    //---------------------------
    shell_loop();
    return 0;
}
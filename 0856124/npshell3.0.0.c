#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#define READBUF_SIZE 1024
#define TOKENBUF_SIZE 128
#define CMD_DELIMITERS "|!"
#define TOK_DELIMITERS " "
#define ERR_DEL1 "|"
#define ERR_DEL2 "!"

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

int num_builtins() 
{
    return sizeof(builtin_str) / sizeof(char *);
}
//---------------------------------------------------


struct number_pipe
{
    int target_cmd_num;
    int in_fd;
    int out_fd;
};

void remove_spaces(char* s) {
    const char* d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while (*s++ = *d++);
}

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
        char *err_line = malloc(sizeof(line));
        strcpy(err_line, line);
        cmd = parse_line(line, CMD_DELIMITERS);

        int num_of_cmd = 0;
        while(cmd[num_of_cmd])// calculate number of command
        {
            num_of_cmd++;
        }
        int fd[num_of_cmd][2];// Fd for pipe

        //--------Check error pipe-------------------
        char **errpipe_check0; 
        char **errpipe_check1;
        int err_idx[60]={99}; //Check err pipe initialize all to be 99
        int err_idx_cnt = 0; // record how many indexes did err_idx used
        int errpipe_idx = 0; // record the cmd idx that currently checking if it's err pipe
        errpipe_check0 = parse_line(err_line, ERR_DEL1);
        while(errpipe_check0[errpipe_idx])
        {
            errpipe_check1 = parse_line(errpipe_check0[errpipe_idx], ERR_DEL2);
            if(errpipe_check1[1])
            {
                err_idx[err_idx_cnt] = errpipe_idx + err_idx_cnt;
                err_idx_cnt++;
            }
            errpipe_idx++;
        }
        //--------------------------------------------
        int cmd_idx = 0;
        while(cmd_idx < num_of_cmd)
        {
            // ----------Check if current cmd has err pipe----------
            int is_errpipe = 0;
            for(int i = 0; i < err_idx_cnt; i++)
            {
                if(cmd_idx==err_idx[i])
                {
                    is_errpipe = 1;
                    break;
                }
            }
            // ------------------------------------------------------

            if(cmd_idx==num_of_cmd-1 && isdigit(cmd[cmd_idx][0]) && !cmd[cmd_idx][1])// skip ls |1/2/3 ...
            {
                break;
            }
            args = parse_line(cmd[cmd_idx], TOK_DELIMITERS);
            if(isdigit(cmd[cmd_idx][0]))
            {
                exe_args = args + 1;
            }
            else
            {
                exe_args = args;
            }

            //-------Remove spaces in every tokens----------
            int tmp_idx = 0;
            while(exe_args[tmp_idx])
            {
                remove_spaces(exe_args[tmp_idx]);
                tmp_idx++;
            }
            //----------------------------------------------

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
            int stdin_fd = STDIN_FILENO;
            int stdout_fd = STDOUT_FILENO;
            int stdin_pipe[2];
            int stdout_pipe[2];

            //----------------Check if there are num pipe---------------------------
            if(cmd[cmd_idx+1])
            {
                if(isdigit(cmd[cmd_idx+1][0])&&cmd[cmd_idx+1][0]!='1')// record every num cmd
                {
                    pipe(fd[cmd_idx]);
                    printf("ls pipe: <in> %d <out> %d\n", fd[cmd_idx][0], fd[cmd_idx][1]);
                    stdout_pipe[0] = fd[cmd_idx][0]; 
                    stdout_pipe[1] = fd[cmd_idx][1];
                    stdout_fd = fd[cmd_idx][1];
                    pipe_num[numpipe_idx].target_cmd_num = cmd_no + ((int)cmd[cmd_idx+1][0]-48);
                    // start from previous cmd
                    pipe_num[numpipe_idx].in_fd = fd[cmd_idx][0];// current cmd's fd
                    pipe_num[numpipe_idx].out_fd = fd[cmd_idx][1];
                    printf("numpipe recorded| target: %d | fd: %d %d\n", \
                        pipe_num[numpipe_idx].target_cmd_num, pipe_num[numpipe_idx].in_fd, pipe_num[numpipe_idx].out_fd);
                    numpipe_idx++;
                }
                else
                {
                    //Need to check if next cmd is someone's target.
                    int next_cmd_is_target = 0;
                    int i;
                    for(i = 0; i < numpipe_idx; i++)
                    {
                        if(cmd_no + 1 == pipe_num[i].target_cmd_num)
                        {
                            // redirect pipe to previous source of next cmd.
                            stdout_pipe[0] = pipe_num[i].in_fd;
                            stdout_pipe[1] = pipe_num[i].out_fd;
                            stdout_fd = pipe_num[i].out_fd;
                            next_cmd_is_target = 1;
                            break;
                        }
                    }
                    if(next_cmd_is_target == 0)// next cmd is not someone's target
                    {
                        pipe(fd[cmd_idx]);
                        stdout_pipe[0] = fd[cmd_idx][0];
                        stdout_pipe[1] = fd[cmd_idx][1];
                        stdout_fd = fd[cmd_idx][1];
                        pipe_num[numpipe_idx].in_fd = fd[cmd_idx][0];// current cmd's fd
                        pipe_num[numpipe_idx].out_fd = fd[cmd_idx][1];
                        pipe_num[numpipe_idx].target_cmd_num = cmd_no + 1;
                        numpipe_idx++;
                    }
                }
                numpipe_idx %= 1000;
            }
            
            
            //---------------------------------------------------------------------

            //----------------Check if current cmd is target-----------------------
            int is_target = 0;
            for(int i = 0; i<numpipe_idx; i++)
            {
                if(cmd_no == pipe_num[i].target_cmd_num)
                {
                    is_target = 1;
                    stdin_pipe[0] = pipe_num[i].in_fd;
                    stdin_pipe[1] = pipe_num[i].out_fd;
                    stdin_fd = pipe_num[i].in_fd;
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
                if(cmd_idx == 0)
                {
                    if(is_target)
                    {  
                        dup2(stdin_fd, STDIN_FILENO);
                        if(is_errpipe)
                        {
                            dup2(stdout_fd, STDERR_FILENO);
                        }
                        dup2(stdout_fd, STDOUT_FILENO);
                        close(stdin_pipe[1]);
                    }
                    else
                    {
                        if(stdout_fd != STDOUT_FILENO)
                        {
                            if(is_errpipe)
                            {
                                dup2(stdout_fd, STDERR_FILENO);
                            }
                            dup2(stdout_fd, STDOUT_FILENO);
                            close(stdout_fd);
                        }
                    }
                }
                else if(cmd_idx == num_of_cmd-1)
                {
                    dup2(stdin_fd, STDIN_FILENO);
                    close(stdin_pipe[1]);
                    if(stdout_fd != STDOUT_FILENO)
                    {
                        if(is_errpipe)
                        {
                            dup2(stdout_fd, STDERR_FILENO);
                        }
                        dup2(stdout_fd, STDOUT_FILENO);
                        close(stdout_fd);
                    }
                }
                else
                {
                    close(stdin_pipe[1]);
                    dup2(stdin_fd, STDIN_FILENO);
                    if(is_errpipe)
                    {
                        dup2(stdout_fd, STDERR_FILENO);
                    }
                    dup2(stdout_fd, STDOUT_FILENO);
                    close(stdout_fd);
                }
                
                execvp(exe_args[0], exe_args);
                exit(0);
            }
            else
            {
                if(stdout_fd != STDOUT_FILENO)
                    close(stdout_fd);
                printf("stdin: %d, stdout: %d\n", stdin_fd, stdout_fd);
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
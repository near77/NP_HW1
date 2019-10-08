#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define READBUF_SIZE 1024
#define TOKENBUF_SIZE 128
#define CMD_DELIMITERS "|"
#define TOK_DELIMITERS " "



struct number_pipe
{
    int target_cmd_num;
    int fd;
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
    int status = 1;
    char *line;// input
    char **cmd;// cmd
    char **args;// cmd arguments
    struct number_pipe pipe_num[1000];// record numbered pipe
    int numpipe_idx = 0;// record numbered pipe list idx
    int cmd_no = 1; //record ?th cmd is executing

    do
    {
        printf("<o> ");
        line = read_line();
        cmd = parse_line(line, CMD_DELIMITERS);
        int cmd_idx = 0;
        while(cmd[cmd_idx])
        {
            if(isdigit(cmd[cmd_idx][0]))
            {
                if(cmd[cmd_idx][0]!='1')// record every num cmd
                {
                    pipe_num[numpipe_idx].target_cmd_num = cmd_no + ((int)cmd[cmd_idx][0]-48) -1;
                    // start from previous cmd
                    pipe_num[numpipe_idx].fd = 1;// previous cmd's fd
                    printf("numpipe recorded| target: %d | fd: %d\n", \
                        pipe_num[numpipe_idx].target_cmd_num, pipe_num[numpipe_idx].fd);
                    numpipe_idx ++;
                    numpipe_idx %= 1000;
                }
            }
            for(int i = 0; i<numpipe_idx; i++)
            {
                if(cmd_no == pipe_num[i].target_cmd_num)
                {
                    printf("exec num pipe.\n");
                    break;
                }
            }
            args = parse_line(cmd[cmd_idx], TOK_DELIMITERS);
            // int arg_idx = 0;
            // while(args[arg_idx])
            // {
            //     printf("Tok: %s \n", args[arg_idx]);
            //     arg_idx++;
            // }
            cmd_idx++;
            cmd_no++;
        }
    }while(status);
}

int main()
{
    shell_loop();
    return 0;
}
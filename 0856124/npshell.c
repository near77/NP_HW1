//https://www.youtube.com/watch?v=z4LEuxMGGs8

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

void read_command(char cmd[], char *parameters[]){
    char line[1024];
    int count=0, i=0, j=0;
    char *array[100], *pch;

    for(;;){
        int c = fgetc(stdin);
        line[count++] = (char) c;
        if(c == '\n'){
            break;
        }
    }//read user input
    if(count == 1){
        return;
    }//just enter
    pch = strtok(line, " \n");//split line with space and store to pch

    while(pch != NULL){
        array[i++] = strdup(pch);//copy tokens in pch to array
        pch = strtok(NULL, " \n");//set pch to NULL
    }
    strcpy(cmd, array[0]);//copy first token to cmd which is command
    for (int j=0;j<i;j++){
        parameters[j] = array[j];//parameters store every tokens of pch including cmd
    }
    parameters[i] = NULL;
}


void type_prompt(){
    /*
    static int first_time = 1;
    if(first_time){
        const char* CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";//e means escape
        write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
        first_time = 0;
    }
    */
    printf("%c ", '%');
}


int main(){
    char cmd[100], command[100], *parameters[20];
    char *envp[] = {(char *) "PATH=/home/t0856124/Desktop/NP_HW1/0856124/bin", 0};

    while(1){
        printf("%c ", '%');
        read_command(command, parameters);
        if(fork()!=0){
            wait(NULL);
        }else{
            strcpy(cmd, "/home/t0856124/Desktop/NP_HW1/0856124/bin/");
            strcat(cmd, command);
            execve(cmd, parameters, envp);
        }
        
        if(strcmp(command, "exit")==0){
            break;
        }else if(strcmp(command, "printenv")==0){
            printf("PATH = %s\n", getenv("PATH"));
        }else if(strcmp(command, "setenv")==0){
            setenv("PATH", *parameters, 0);
            printf("env had been set.\n");
        }
        cmd[0] = 0;
        command[0] = 0;
        *parameters[0] = 0;
    }
    return 0;
}
#include <stdio.h>
#include <stdlib.h>

void loop(void){
    char* line;
    char** args;
    int status;

    do{
        printf("0 ");
        line = read();
        args = parse(line);
        status = execute(args);
        free(line);
        free(args);
    }while(status);
}


int main(){
    
    loop();

    return 0;
}


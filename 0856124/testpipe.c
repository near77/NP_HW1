#include <stdio.h>
int main()
{
    char test[4] = {'0','1', '2', '3'};
    int i = 0;
    while(test[i])
    {
        printf("test: %d\n", test[i]);
        i++;
    }

    
    char *test2 = test + 1;
    int j = 0;
    while(test2[j])
    {
        printf("test2: %d\n", test[j]);
        j++;
    }
    
    return 0;
}
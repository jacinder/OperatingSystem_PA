#include<stdio.h>
#include<string.h>
int main(){
    char message[256];
    char buffer[32];
    int length = 10;
    int best_child_path[5]={1,2,3,4,5};

    sprintf(message,"%d ",length);
    printf("1: %s\n",message);
    for(int i = 0; i < 5; ++i){
        sprintf(buffer,"%d ",best_child_path[i]);
        //printf("best_child_path[%d] : %d",i,best_child_path[i]);
        strcat(message,buffer);
    }
    printf("2: %s\n",message);

    return 0;

}
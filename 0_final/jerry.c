#include<stdlib.h>
#include<stdio.h>
void block_open_file();
void prevent_process_kill();
int main(){
        int input=0;
        while(1){
            printf("Which fucntion do you want to use?\n");
            printf("0. Terminate this program")
            printf("1. block a certain user from opening a specified files\n");
            printf("2. prevent a killing of processes created by a specific user\n");
            scanf("%d"&input);
            if(input==0) break;
            else if(input==1) {
                system("sudo insmod openx.ko");
                block_open_file();
            }
            else if(input==2) {
                system("sudo insmod killx.ko");
                prevent_process_kill();
            }
            else;
        }
        return -1;
}
void block_open_file(){
        char fname[64];
        char uname[64];
        printf("Enter a file name : ");
        scanf("%s",fname);
        printf("Entr a user name : ");
        scanf("%s",uname);
        ///CODE HERE

        system();
}
void prevent_process_kill(){
        char uname[64];
        char buffer[256];
        uid_t uid=-1;

        printf("Enter a user name : ");
        scanf("%s",uname);

        sprintf(buffer,"uid=id -ur %s",uname);
        system(buffer)
        sprintf(buffer, "echo %s > /proc/killx",uid);
        system(buffer);
}
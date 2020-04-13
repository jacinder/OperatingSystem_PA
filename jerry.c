#include<stdlib.h>
#include<stdio.h>
#include <pwd.h> 
void block_open_file();
void prevent_process_kill();
int main(){
        int input=0;
        int returnV = -1;
        while(1){
            printf("Which fucntion do you want to use?\n");
            printf("\t0. Terminate this program")
            printf("\t1. block a certain user from opening a specified files\n");
            printf("\t2. prevent a killing of processes created by a specific user\n");
            scanf("%d"&input);
            if(input==0) break;
            else if(input==1) {
                returnV = system("sudo insmod openx.ko");
                returnV == 0 ? block_open_file() : printf("openx Error\n");
            }
            else if(input==2) {
                returnV=system("sudo insmod killx.ko");
                returnV == 0 ? prevent_process_kill(); : printf("killx Error\n");
            }
            else;
        }
        return -1;
}

void block_open_file(){
    char fname[64];
    char uname[64];
    char buffer[256];
    uid_t target_uid = -1;
    FILE *fp = fopen("/proc/openx", "w");

    printf("Enter a file name : ");
    scanf("%s",fname);
    printf("Entr a user name : ");
    scanf("%s",uname);
    target_uid=get_uid(uname);
    if(target_uid == -1){
        printf("Failed to get uid\n");
        exit(-1);
    }
    sscanf(buffer,"%d,%s",target_uid,fname);
    fputs(buffer, fp);
}

void prevent_process_kill(){
    char uname[64];
    char buffer[256];
    uid_t target_uid = -1;
    FILE *fp = fopen("/proc/killx", "w");

    printf("Enter a user name : ");
    scanf("%s",uname);
    target_uid = get_uid(uname);
    if(target_uid == -1){
        printf("Failed to get uid\n");
        exit(-1);
    }

    if (fp == NULL) {
        printf("Failed to open /proc/killx\n");
        exit(-1);
    }
    sscanf(buffer,"%d",target_uid);
    fputs(buffer, fp);
}

uid_t get_uid(char* username){
    struct passwd *user_pw;
    user_pw  = getpwnam(uname);
    return user_pw->pw_uid
}
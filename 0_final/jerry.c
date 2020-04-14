#include<stdlib.h>
#include<stdio.h>
void block_open_file(FILE * fp);
void prevent_process_kill(FILE* fp);
int main(){
        int input=0;
        while(1){
            printf("Which fucntion do you want to use?\n");
            printf("0. Terminate this program")
            printf("1. block a certain user from opening a specified files\n");
            printf("2. prevent a killing of processes created by a specific user\n");
            scanf("%d"&input);
            if(input==0) break;
            else if(input==1 || input==2){
                system("sudo insmod mousehole.ko");
                FILE *fp = fopen("/proc/mousehole", "w");
                if (fp == NULL) {
                    printf("Failed to open /proc/mousehole\n");
                    exit(-1);
                }
                input == 1? block_open_file():prevent_process_kill();
            }
            else;
        }
        return -1;
}
void block_open_file(FILE* fp){
    char fname[64];
    char uname[64];
    char buffer[256];
    uid_t target_uid = -1;
    FILE *fp = fopen("/proc/mousehole", "w");

    printf("Enter a file name : ");
    scanf("%s",fname);
    printf("Entr a user name : ");
    scanf("%s",uname);
    target_uid=get_uid(uname);
    if(target_uid == -1){
        printf("Failed to get uid\n");
        exit(-1);
    }
    sscanf(buffer,"1 %d%s",target_uid,fname);
    fputs(buffer, fp);
}

void prevent_process_kill(FILE* fp){
    char uname[64];
    char buffer[256];
    uid_t target_uid = -1;

    printf("Enter a user name : ");
    scanf("%s",uname);
    target_uid = get_uid(uname);
    if(target_uid == -1){
        printf("Failed to get uid\n");
        exit(-1);
    }
    sscanf(buffer,"2 %d",target_uid);
    fputs(buffer, fp);
}

uid_t get_uid(char* username){
    struct passwd *user_pw;
    user_pw  = getpwnam(uname);
    return user_pw->pw_uid
}
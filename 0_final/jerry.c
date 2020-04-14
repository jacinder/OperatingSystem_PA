#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include <pwd.h> 
void block_open_file(FILE * );
void prevent_process_kill(FILE* );
void check_current_info(FILE* )
int release_module();
int main(){
        int option=0;
        printf("Which fucntion do you want to use?\n");
        printf("1. Block a certain user from opening a specified files\n");
        printf("2. Prevent a killing of processes created by a specific user\n");
        printf("3. Check current mousehole module info\n");
        scanf("%d",&option);
        if(!(option==1 || option==2 || option==3 ){
            printf("Wrong input\n");
            return 0;
        }
        int release = release_module();
        if(option ==3 && release != 1){
            printf("Mousehole module is not running now\n");
            return 0;
        }
        if(option==1 || option==2){
            if (release == 1)
                system("sudo rmmod mousehole");
            system("sudo insmod mousehole.ko");
            printf("Start mousehole module\n");
        }
        FILE *fp = fopen("/proc/mousehole", "wt");
        if (fp == NULL){
            printf("Failed to open /proc/mousehole\n");
            return -1;
        }
        if(option == 1) block_open_file(fp)
        else if(option == 2) prevent_process_kill(fp);
        else check_current_info(fp);
        fclose(fp);
        return 0;
}
void block_open_file(FILE* fp){
    char fname[64];
    char uname[64];
    //char buffer[256];
    struct passwd*  user_pw;
    //int target_uid;

    printf("Enter a file name : ");
    scanf("%s",fname);
    printf("Entr a user name : ");
    scanf("%s",uname);

    user_pw = getpwnam(uname);
    //target_uid = (int)(user_pw->pw_uid);
    fprintf(fp, "1 %d %s", user_pw->pw_uid,fname);
}
void prevent_process_kill(FILE* fp){
    char uname[64];
    //char buffer[256];
    struct passwd* user_pw;
    //int target_uid;

    printf("Enter a user name : ");
    scanf("%s",uname);

    user_pw = getpwnam(uname);
    //target_uid = (int)(user_pw->pw_uid);
    fprintf(fp, "2 %d", user_pw->pw_uid);
}

int release_module(){
    char buffer[128];
    FILE* fp = fopen("/proc/modules","r");
    if(fp==NULL){
        printf("Failed to open /proc/modules\n");
        return -1;
    }
    while(fgets(buffer, sizeof(buffer), fp)){
        if(strstr(buffer,"mousehole")){
            return 1;
        }
    }
    fclose(fp);
    return 0;
    printf("No mousehole module running");
}
void check_current_info(FILE* ){
    char buffer[128];
    while (fgets(buf, sizeof(buffer), fp)){
        printf("%s\n", buf);
    }
}
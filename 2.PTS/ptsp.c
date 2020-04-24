#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <error.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int ROW = 0
int COL = 0;
int pipes[2];
int** m;
int* path;
int* used;
int length = 0;
int min = -1;

void travel(int start){
    path[0] = start;
    used[start] = 1;
    _travel(1);
    used[start] = 0;
}

void _travel(int idx){
    if (idx == COL){
        length += m[path[16]][path[0]];
        if (min == -1 || min > length){
            min = length ;
            printf("%d (", length);
            for (i=0 ; i<ROW ; i++) 
                printf("%d ", path[i]);
            printf("%d)\n", path[0]);
        }
        length -= m[path[16]][path[0]];
    }
    else{
        for (i=0 ; i<ROW ; i++){
            if (used[i] == 0){
                path[idx] = i;
                used[i] = 1;
                length += m[path[idx-1]][i];
                _travel(idx+1);
                length -= m[path[idx-1]][i];
                used[i] = 0;
            }
        }
    }
}
//최대 연산량이 12!라는건 하나의 child_proc에게 주어지는 선택지를 12개 이하로 하라는 말이지
//마지막 12개는 child process에게, 나머지는 parent에서 반복문으로 처리하라는 건가..;

void child_proc(){
    //writer
    close(pipes[0]);
    //
    //
    //
    exit(0);
}
void parent_proc(){
    //reader
    close(pipes[1]);
    //
    //
    //
    close(pipes[0];
}
void handler (int sig){
    if (sig == SIGINT) {
        //best solution upto the point
        for (i=0;i<ROW;i++) {
            printf("0 ~ %d : %d\n",i,path[i]);
        }
        //total number of checked/covered routes upto the point
        exit(0);
    }
}
int done(){
    for(int i=0;i<ROW;i++){
        if(used[i]==0)
            return 0;
    }
    return 1;
}

int main(int argc, char argv[]){
    if(argc != 3){
        printf("Wrong input: Enter filename and limit of children processes");
        return 0;
    }
    char filename[32];
    char str[128];
    int limit=atoi(argv[2]);
    pid_t child_pid;
    int exit_code ;

    strcpy(filename,argv[1]);
    signal(SIGINT, handler);

    FILE * fp = fopen(filename, "r") ;
    FILE * fp2 = fp;
    if(fp == NULL){
        printf("Failed to open %s\n",filename);
        exit(-1);
    }
    while(fgets (str, 128, fp)!=NULL){
            ROW++;
    }
    COL = ROW;
    m = (int **)malloc(sizeof(int *) * ROW);
    m[0] = (int *)malloc(sizeof(int) * ROW * COL );
    for(int i=1; i<ROW; i++){
        m[i] = m[i-1] + COL;
    }
    path = (int*)malloc(sizeof(int) * ROW);
    used = (int*)malloc(sizeof(int) * ROW);

    //Initialize
    for (i = 0 ; i < ROW ; i++){
        for (j = 0 ; j < COL ; j++){
            fscanf(fp2, "%d", &t) ;
            m[i][j] = t ;
        }
    }
    fclose(fp);
    for (i=1;i<=n;i++) { 
        used[i] = 0;
        path[i] = 99;
    }

    //Start travel
    for (i = 0;i<ROW;i++){
        travel(i);
    }

    //make a pipe between parent - child
    if (pipe(pipes) != 0){
        perror("Error");
        exit(1);
    }
    //만약 전부 탐색하지 못했다면 child proc를 만든다.
    while(done()==0)
        child_pid = fork();
        if (child_pid == 0){
            child_proc();
        }
        else{
            parent_proc();
        }
    }
    wait(&exit_code);

    exit(0);
}
/*
TSP 알고리즘 참고 : https://shoark7.github.io/programming/algorithm/introduction-to-tsp-and-solve-with-exhasutive-search
permutation 참고 : https://twpower.github.io/62-permutation-by-recursion
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
//#include <error.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>

#define N 5
#define cal_limit 2

int pipes[2]; //result 전달을 위한 파이프
int dist[N][N]; //도시간의 거리 저장하는 매트릭스
int best_path[N];
int ans = 2000000000;
int** prefix;
int** path;
int prefixLen = N - cal_limit;
int prefixCase = 1;
int temp_count = 0;
int running_limit = 0;
int running_proc = 0;
int process_count = -1;
int parent_pid;

void final_print(){
    //best solution upto the point
    printf("\n\n-------------[FINAL]-------------\n");
    printf("best solution :\n");
    for(int i=0;i<N;i++){
        printf("%d ",best_path[i]);
    }
    //total number of checked/covered routes upto the point
    printf("\nchecked routes :\n");
    for(int i=0;i<prefixCase;i++){
        for (int j=0;j<N;j++) {
            if(path[i][j]!=-1)
                printf("%d ",path[i][j]);
        }
        printf("\n");
    }
    printf("\nlength = %d\n",ans);
}

void terminate_handler (int sig){
    if ((sig == SIGINT) && (getpid() == parent_pid)) { //parent
        final_print();
        exit(0);
    }
    else{//child
        exit(0);
    }
}

void openFile(char filename[]){
    FILE * fp = fopen(filename, "r");
    if(fp == NULL){
        printf("Failed to open %s\n",filename);
        exit(1);
    }
    int t;
    for (int i = 0 ; i < N ; i++){
        for (int j = 0 ; j < N ; j++){
            fscanf(fp, "%d", &t) ;
            dist[i][j] = t ;
        }
    }
    fclose(fp);
}

void permutation(int n, int r, int depth,int* arr){
    int tmp;
	if(r == depth){
		for(int i=0; i < depth; i++)
		    prefix[temp_count][i]=arr[i];
        temp_count++;
		return;
	}
	for(int i=depth; i<n; i++){
        tmp = arr[i];
        arr[i] = arr[depth];
        arr[depth] = tmp;
		permutation(n, r, depth+1, arr);
		tmp = arr[i];
        arr[i] = arr[depth];
        arr[depth] = tmp;
	}

}

void makePrefix(){

    for(int i=N ; i>cal_limit ; i--){
        prefixCase *= i;
    }

    prefix = (int**)malloc(sizeof(int*)* prefixCase);
    for(int i=0;i<prefixCase;i++){
        prefix[i]=(int*)malloc(sizeof(int)*prefixLen);
    }

    path=(int**)malloc(sizeof(int*)*prefixCase);
    for(int i=0;i<prefixCase;i++){
        path[i]=(int*)malloc(sizeof(int)*N);
        for(int j=0;j<N;j++){
            path[i][j]=-1;
        }
    }

    int arr[N];
    for(int i=0;i<N;i++) arr[i]=i;
	permutation(N, prefixLen, 0, arr);
}

void find_path(int start, int last, int tmp_dist, int curr, int* length, int used[], int child_path[], int* best_child_path){

    int FLAG = 1;
    for(int i=0;i<N;i++){
        if(used[i] == 0) FLAG = 0;
    }
    if (FLAG==1){ 
        int return_home_dist = dist[last][start];
        if(*length > (tmp_dist + return_home_dist)){
            *length = tmp_dist + return_home_dist;
            for(int i=0;i<N;i++){
                best_child_path[i]=child_path[i];
            }
        }
    }
    else{
        for(int left=0;left<N;left++){
            if(used[left]==0){ //loop for left cities

                used[left]=1;
                child_path[curr]=left;
                curr++;

                find_path(start,left,tmp_dist+dist[last][left], curr, length, used, child_path, best_child_path);
                
                used[left]=0;
                child_path[curr]=0;
                curr--;
            }
        }
    }
    
}

void child_proc(){
    close(pipes[0]);//write mode
    int this_process_count = process_count;
    int start = prefix[process_count][0];
    int last = prefix[process_count][prefixLen-1];
    int curr = prefixLen;
    int used[N];
    int child_path[N];
    int best_child_path[N];
    for(int i=0;i<N;i++){
        used[i]=child_path[i]=0;
    }
    for(int i=0;i<prefixLen;i++){
        child_path[i]=prefix[process_count][i];
        used[child_path[i]]=1;
    }

    int tmp_dist = 0;
    for(int i=0;i<prefixLen-1;i++){
        tmp_dist += dist[child_path[i]][child_path[i+1]];
    }
    int length = 2000000000;
    find_path(start, last, tmp_dist, curr, &length, used, child_path, &best_child_path[0]);

    if(length < ans){
        for(int i=0;i<N;i++){
            best_path[i] = child_path[i];
        }
    }

    char message[256];
    char buffer[32];
    /*DEBUGGING*/
    //printf("length: %d\n",length);
    //
    sprintf(message,"%d %d ",this_process_count,length);
    for(int i = 0; i < N; ++i){
        sprintf(buffer,"%d ",best_child_path[i]);
        strcat(message,buffer);
    }
    write(pipes[1], message, 256);

    for(int i=0;i<N;i++){
        used[i]=child_path[i]=0;
    }
    tmp_dist = 0;
    length = 1000000000;
    close(pipes[1]);
    exit(0);
}

void parent_proc(){
    int exitcode;
    int this_process_count;
    int length;
    int i = 0;
    char message[256];

    pid_t child = wait(&exitcode) ;
    printf("> child process %d is terminated with exitcode %d\n", child, WEXITSTATUS(exitcode));

    close(pipes[1]);//read mode
    read(pipes[0], message, 256);
    //message에서 length, path를 읽어오기
    char *ptr = strtok(message, " ");
    sscanf(ptr,"%d",&this_process_count);          // 자른 문자열 출력
    ptr = strtok(NULL, " ");
    sscanf(ptr,"%d",&length);          // 자른 문자열 출력
    ptr = strtok(NULL, " ");

    while (ptr != NULL && i<N){
        sscanf(ptr,"%d",&path[this_process_count][i]);
        ptr = strtok(NULL, " ");
        i++;
    }

    if(length < ans){
        ans = length;
        for(int i=0;i<N;i++){
            best_path[i] = path[this_process_count][i];
        }
    }

    // printf("\nans : %d\nbest path : \n",ans);
    // for(int i=0;i<N;i++){
    //     printf("%d ",best_path[i]);
    // }
    // printf("\n");
    close(pipes[0]);
    running_proc--;
}

int main(int argc, char* argv[]){
    running_limit = atoi(argv[2]);
    openFile(argv[1]);
    signal(SIGINT, terminate_handler);

    //prefix 만들기
    makePrefix();
    
    for(int q=0;q<prefixCase;q++){
        pid_t child_pid;
        if(pipe(pipes) != 0){
            perror("Error") ;
            exit(1) ;
        }
        if(running_proc == running_limit){
            wait(0);
        }

        if(running_proc < running_limit){
            running_proc++;
            process_count++;
            child_pid = fork();
        
            if (child_pid < 0){
                printf("Failed to make child process\n");
                exit(1);
            }
            else if(child_pid > 0){
                printf("Child %d is forked\n", child_pid);
                parent_proc();
            }
            if (child_pid == 0) {
                child_proc();
            }
        }
    }
    final_print();
    exit(0);
}
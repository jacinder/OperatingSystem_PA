#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <error.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>

#define N 17
#define cal_limit 12

int** pipes; //for transfer result
int dist[N][N]; //distances between cities
int best_path[N];
int ans = 2000000000;
int** prefix;
int** path;
int* process_pipe_table;
int prefixLen = N - cal_limit;
int prefixCase = 1;
int temp_count = 0;
int running_limit = 0;
int running_proc = 0;
int process_count = 0;
int parent_pid;

void final_print(){
    //best solution upto the point
    printf("best solution :\n");
    for(int i=0;i<N;i++){
        printf("%d ",best_path[i]);
    }
    //total number of checked/covered routes upto the point
    printf("\nchecked routes :\n");
    for(int i=0;i<process_count;i++){
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
    else{
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
            path[i][j]=-1; //initialize path with -1
        }
    }

    pipes = (int**)malloc(sizeof(int*)* prefixCase);
    for(int i=0;i<prefixCase;i++){
        pipes[i]=(int*)malloc(sizeof(int)*2);
    }

    process_pipe_table = (int*)malloc(sizeof(int*)* prefixCase);
    for(int i=0;i<prefixCase;i++){
        process_pipe_table[i] = -1; //initialize process_pipe_table with -1
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

void child_proc(int this_process_count){
    close(pipes[this_process_count][0]);
    
    int start = prefix[this_process_count][0];
    int last = prefix[this_process_count][prefixLen-1];
    int curr = prefixLen;
    int used[N];
    int child_path[N];
    int best_child_path[N];
    for(int i=0;i<N;i++){
        used[i]=child_path[i]=0;
    }
    for(int i=0;i<prefixLen;i++){
        child_path[i]=prefix[this_process_count][i];
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
    sprintf(message,"%d %d ",this_process_count,length);
    for(int i = 0; i < N; ++i){
        sprintf(buffer,"%d ",best_child_path[i]);
        strcat(message,buffer);
    }
    write(pipes[this_process_count][1], message, 256);

    for(int i=0;i<N;i++){
        used[i]=child_path[i]=0;
    }
    tmp_dist = 0;
    length = 2000000000;
    close(pipes[this_process_count][1]);
    exit(0);
}
void sigchld_handler(int sig){
    int exitcode;
    int this_process_count = -1;
    int this_pipe_count;
    int length;
    int i = 0;
    char message[256];

    pid_t child = wait(&exitcode) ;
    for(int j=0;j<prefixCase;j++){
        if(process_pipe_table[j]==child){
            this_pipe_count = j;
            break;
        }
    }
    close(pipes[this_pipe_count][1]);//read mode
    read(pipes[this_pipe_count][0], message, 256);

    char *ptr = strtok(message, " ");
    sscanf(ptr,"%d",&this_process_count);
    ptr = strtok(NULL, " ");
    sscanf(ptr,"%d",&length);
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

    close(pipes[this_pipe_count][0]);
    running_proc--;
}

int main(int argc, char* argv[]){
    running_limit = atoi(argv[2]);
    openFile(argv[1]);
    parent_pid=getpid();
    signal(SIGCHLD, sigchld_handler);
    signal(SIGINT, terminate_handler);

    makePrefix();
    
    while(process_count<prefixCase){
        pid_t child_pid;
        if(pipe(pipes[process_count]) != 0){
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
                process_pipe_table[process_count-1]=child_pid;
            }
            else{
                child_proc(process_count-1);
            }
        }
    }
    sleep(10);
    final_print();
    exit(0);
}

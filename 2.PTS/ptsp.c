/*
TSP 알고리즘 참고 : https://shoark7.github.io/programming/algorithm/introduction-to-tsp-and-solve-with-exhasutive-search
permutation 참고 : https://twpower.github.io/62-permutation-by-recursion
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <error.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>

#define N 5
#define callimit 2

typedef struct {
    // int N;
    // int dist[N][N];
    int prefix[N-callimit];
    int prefixLen;
    int path[N];
} Buffer;

int pipes[2]; //result 전달을 위한 파이프
int dist[N][N]; //도시간의 거리 저장하는 매트릭스
int ANS = INFINITY;
int** prefix;
int count = 0;

void handler(int sig);
void swap(int *a, int *b );
void print_arr(int size, int* arr);
void permutation(int n, int r, int depth,int* arr);
int find_pid(int* total_pid, int limit);

void child_proc(int shm_id){
    sleep(1); // wait for the parent to initialize the shared buffer
    close(pipes[0]);
    Buffer *buffer = (Buffer*)mmap(0,sizeof(Buffer),PROT_READ,MAP_SHARED,shm_id,0);

    //get info from shared memory
    int prefix[N-callimit];
    for(int i=0;i<N-callimit;i++){
        prefix[i] = buffer->prefix[i];
    }
    int prefixLen = buffer->prefixLen;

    float ans = INFINITY;
    int visited = 0;
    int used[N] = {0};
    int path[N] = { [0 ... N-1] = -1 };
    int last = prefix[prefixLen-1];
    int start = prefix[0];
    int tmp_dist = 0;

    int done(){
        for(int i=0;i<N;i++)
            if(used[i]==0)
                return 0;
        return 1;
    }

    void find_path(){
        if (done()==1){ //모든 도시를 모두 거쳤을 경우
            int return_home_dist = dist[last][start];
            ans = (ans < tmp_dist + return_home_dist) ? ans : tmp_dist+return_home_dist;
            //최단거리를 ans에 저장
            //메인에 pipe로 ans, path리턴
        }
        else{
            for(int left=0;left<N;left++){
                if(used[left]==0){ //loop for left cities
                    used[left]=1;
                    path[visited]=left;
                    visited++;
                    find_path(start,left,path,tmp_dist+dist[last][left])
                }
            }
        }
    }

    //find tmp_dist of prefix
    for(int i=0;i<prefixLen-2;i++){
        tmp_dist += dist[i][i+1];
    }
    //initialize array for find_path
    for(int i=0;i<prefixLen-1;i++){
        used[prefix[i]]=1;
        path[i]=prefix[i];
    }

    find_path();
    //send ans using pipe to parent
    char buf[32];
    sprintf(buf,"%f",ans);
    write(pipes[1], buf, 31);

    for(int i=0;i<N;i++){
        buffer->path[i] = path[i];
    }

    munmap(buffer,sizeof(Buffer));
    exit(0);
}

void parent_proc(int shm_id, int** path, char name[],int prefixNum, int* total_pid, int curr){
    int exit_code;
    close(pipes[1]);
    Buffer* buffer = (Buffer*)mmap(0,sizeof(Buffer),PROT_READ | PROT_WRITE,MAP_SHARED,shm_id,0);

    buffer->prefixLen = N-callimit;
    for(int i=0;i<N-callimit;i++){
        buffer->prefix[i] = prefix[prefixNum][i];
    }
    
    //wait til child process is terminated
    wait(&exit_code);

    //get info from child
    char buf[32];
    read(pipes[0], buf, 31);
    int ans = atoi(buf);
    if(ans < ANS) ANS = ans;
    close(pipes[0]);

    for(int i=0;i<N;i++){
        path[prefixNum][i] = buffer->path[i];
    }  
    shm_unlink(name);

    total_pid[curr] = 0;

}

int main(int argc, char argv[]){
    if(argc != 3){
        printf("Wrong input: Enter filename and limit of children processes");
        return 0;
    }
    char filename[32];
    char str[128];
    int limit = atoi(argv[2]);
    pid_t child_pid[limit];
    

    strcpy(filename,argv[1]);
    signal(SIGINT, handler);

    FILE * fp = fopen(filename, "r");
    if(fp == NULL){
        printf("Failed to open %s\n",filename);
        exit(-1);
    }

    //Initialize
    int t;
    for (int i = 0 ; i < N ; i++){
        for (int j = 0 ; j < N ; j++){
            fscanf(fp, "%d", &t) ;
            dist[i][j] = t ;
        }
    }
    fclose(fp);

    int prefixLen = N - callimit;
    int prefixCase = 1;

    //calculate nPr to get to know number of all kind of prefix.
    for(int i=0;i<r;i++){ 
        prefixCase *= n;
        n--;
    }
    int prefixNum = prefixCase;
    prefix = (int**)malloc(sizeof(int*)*prefixCase);
    for(int i=0;i<prefixCase;i++){
        prefix[i]=(int*)malloc(sizeof(int)*prefixLen);
    }
    int path[prefixCase][N];

    int curr_pid = 0;
    int running_pid = 0;
    int total_pid[limit] = {0};
    char buf[256];
    
    //making prefix
    int arr[N];
    for(int i=0;i<N;i++){
        arr[i]=i;
    }
	permutation(N, prefixLen, 0, arr);
    if(count != prefixCase){
        printf("Failed to make prefix properly\n");
        exit(-1);
    }

    while(prefixNum != 0){ // if still prefix list remain
        if(running_pid < limit){ //if there is remaining child_pid
            if (pipe(pipes) != 0) {
                perror("Error") ;
                exit(1) ;
            }
            curr_pid = find_pid(total_pid, limit);
            if(curr_pid == -1) continue;

            //make a shared memory
            char name[32];
            sprintf(name,"/buffer[%d]",curr_pid);
            int shm_id = shm_open(name, O_CREAT | O_RDWR, 0666);
            ftruncate(shm_id,sizeof(Buffer));
            
            child_pid[curr_pid] = fork();
            total_pid[curr_pid] = 1;
            prefixNum --;

            if(child_pid < 0){
                printf("Failed to create child process\n");
                exit(-1);
            } else if(child_pid > 0){
                parent_proc(shm_id, path, name, prefixNum, total_pid, curr_pid);
            } else {
                child_proc(shm_id);
            }
        }
    }
    exit(0);
}

void handler (int sig){
    if (sig == SIGINT) {
        //best solution upto the point
        printf("best solution :\n");
        for (i=0;i<N;i++) {
            printf("%d ",path[i]);
        }
        printf("\nlength = %d\n",ans);
        //total number of checked/covered routes upto the point
        //패쓰들을 다 저장해야 하는건가???
        exit(0);
    }
}

void swap(int *a, int *b ){
	int tmp;
	tmp = *a;
	*a = *b;
	*b = tmp;
}

void print_arr(int size, int* arr){
	for(int i=0; i < size; i++)
		prefix[count][i]=arr[i];
    count++;
}

void permutation(int n, int r, int depth,int* arr){

	if(r == depth){
		print_arr(depth, arr);
		return;
	}
	for(int i=depth; i<n; i++){
		swap(&arr[i], &arr[depth]);
		permutation(n, r, depth+1, arr);
		swap(&arr[i], &arr[depth]);
	}

}

int find_pid(int* total_pid, int limit){
    for(int i=0;i<limit;i++){
        if(total_pid[i]==0) return i;
    }
    return -1;
}
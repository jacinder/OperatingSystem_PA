#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
// #include <error.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#define N 5
#define callimit 2

typedef struct {
    int child_path[N];
} Buffer;

int pipes[2]; //result 전달을 위한 파이프
int dist[N][N]; //도시간의 거리 저장하는 매트릭스
float ANS = INFINITY;
int** prefix;
int** path;
int* best_path;
int* prefixCase;
int count = 0;

void child_proc(int shm_id,int prefixNum, int prefixLen){
    sleep(1); // wait for the parent to initialize the shared buffer
    close(pipes[0]);
    Buffer *buffer = (Buffer*)mmap(0,sizeof(Buffer),PROT_READ,MAP_SHARED,shm_id,0);

    int child_prefix[prefixLen];
    for(int i=0;i<prefixLen;i++){
        child_prefix[i] = prefix[prefixNum][i];
    }

    float ans = INFINITY;
    int visited = 0;
    int used[N] = {0};
    int child_path[N] = { [0 ... N-1] = -1 };
    int last = prefix[prefixNum][prefixLen-1];
    int start = prefix[prefixNum][0];
    int tmp_dist = 0;
    int FALG = 1;

    void find_path(){
        for(int i=0;i<N;i++)
            if(used[i]==0)
                FLAG = 0;
        if (FLAG==1){ //모든 도시를 모두 거쳤을 경우
            int return_home_dist = dist[last][start];
            ans = (ans < tmp_dist + return_home_dist) ? ans : tmp_dist+return_home_dist;
            //최단거리를 ans에 저장
            //메인에 pipe로 ans, path리턴
        }
        else{
            for(int left=0;left<N;left++){
                if(used[left]==0){ //loop for left cities
                    used[left]=1;
                    child_path[visited]=left;
                    visited++;
                    find_path(start,left,child_path,tmp_dist+dist[last][left]);
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
        used[prefix[prefixNum][i]]=1;
        child_path[i]=prefix[prefixNum][i];
    }

    find_path();
    //send ans using pipe to parent
    char buf[32];
    sprintf(buf,"%f",ans);
    write(pipes[1], buf, 31);

    //send this path using shm
    for(int i=0;i<N;i++){
        buffer->child_path[i] = child_path[i];
    }

    for(int i=0;i<N;i++){
        path[prefixNum][i] = child_path[i];
    }

    munmap(buffer,sizeof(Buffer));
    exit(0);
}

void parent_proc(int shm_id, char name[],int prefixNum, int* total_pid, int curr){
    int exit_code;
    close(pipes[1]);
    Buffer* buffer = (Buffer*)mmap(0,sizeof(Buffer),PROT_READ | PROT_WRITE,MAP_SHARED,shm_id,0);
    
    //wait til child process is terminated
    wait(&exit_code);
    printf("count : %d\n",count);

    //get info from child
    char buf[32];
    read(pipes[0], buf, 31);
    int ans = atof(buf);
    if(ans < ANS){
        ANS = ans;
        for(int i=0;i<N;i++){
            best_path[i] = buffer->child_path[i];
        }
    }
    close(pipes[0]);

    shm_unlink(name);

    total_pid[curr] = 0;

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

void handler (int sig){
    if (sig == SIGINT) {
        //best solution upto the point
        printf("best solution :\n");
        for(int i=0;i<N;i++){
            printf("%d ",best_path[i]);
        }
        //total number of checked/covered routes upto the point
        printf("\nchecked routes :\n");
        for(int i=0;i<(*prefixCase);i++){
            for (int j=0;j<N;j++) {
                if(path[i][j]!=-1)
                    printf("%d ",path[i][j]);
            }
        }
        printf("\nlength = %f\n",ANS);
        exit(0);
    }
}

int main(int argc, char *argv[]){
    if(argc != 3){
        printf("Wrong input: Enter filename and limit of children processes");
        return 0;
    }
    char filename[32];
    int limit = atoi(argv[2]);
    pid_t child_pid[limit];
    

    strcpy(filename,argv[1]);
    // signal(SIGINT, handler);

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
    for (int i = 0 ; i < N ; i++){
        for (int j = 0 ; j < N ; j++){
            printf("%d ", dist[i][j]);
        }
        printf("\n");
    }

    int prefixLen = N - callimit;
    int _prefixCase = 1;

    //calculate nPr to get to know number of all kind of prefix.
    int n = N;
    for(int i=0;i<prefixLen;i++){ 
        _prefixCase *= n;
        n--;
    }
    int prefixNum = _prefixCase;
    prefixCase = (int*)malloc(sizeof(int));
    *prefixCase = _prefixCase;
    prefix = (int**)malloc(sizeof(int*)*_prefixCase);
    for(int i=0;i<_prefixCase;i++){
        prefix[i]=(int*)malloc(sizeof(int)*prefixLen);
    }
    path=(int**)malloc(sizeof(int*)*_prefixCase);
    for(int i=0;i<_prefixCase;i++){
        path[i]=(int*)malloc(sizeof(int)*N);
    }
    for(int i=0;i<_prefixCase;i++){
        for(int j=0;j<N;j++){
            path[i][j]=-1;
        }
    }
    best_path = (int*)malloc(sizeof(int)*N);

    int curr_pid = 0;
    int running_pid = 0;
    int total_pid[limit];
    for(int i=0;i<limit;i++) total_pid[i]=0;
    char buf[256];
    
    //making prefix
    int arr[N];
    for(int i=0;i<N;i++){
        arr[i]=i;
    }
	permutation(N, prefixLen, 0, arr);
    if(count != _prefixCase){
        printf("Failed to make prefix properly\n");
        exit(-1);
    }
//print for check
    printf("prefix : \n");
    for(int i=0;i<_prefixCase;i++){
        for(int j=0;j<prefixLen;j++){
            printf("%d ",prefix[i][j]);
        }
        printf("\n");
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
                parent_proc(shm_id, name, prefixNum, total_pid, curr_pid);
            } else {
                child_proc(shm_id, prefixNum, prefixLen);
            }
        }
    }
    return 0;
}
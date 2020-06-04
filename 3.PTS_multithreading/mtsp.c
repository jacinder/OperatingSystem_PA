#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
//#include <error.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <pthread.h> 
#define non_DEBUG

#define N 17
#define cal_limit 11

void *producerRunner(void *param);
void find_path(int THIS, int start, int last, int tmp_dist, int curr, int* length, int used[], int child_path[], int* best_child_path);
void *consumerRunner(void* _THIS);
void changeNumConsumer(void );
void assignTask(void* _THIS);
void printThreads(void);
void printStat(void);
void openFile(char* filename);
void makePrefix();
void permutation(int n, int r, int depth,int* arr);
void terminate_handler (int sig);

int** path;
int** prefix;
int dist[N][N];
int best_path[N];
int temp_count = 0;
int prefixCase = 1;
int prefixLen =  N - cal_limit;
int numOfConsumer = 8;
int ans = 99999;
int p=0;
pthread_t prod_tid = 0;
pthread_t cons_tid[8]={0};
int booking_cons_tid[8]={0};
int currNum_cons_tid[8]={-1,-1,-1,-1,-1,-1,-1,-1};
//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // initialize thread
int cons_srch_cnt[8]={0};

void terminate_handler (int sig){
    printStat();
    printThreads();
    exit(0);
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

    int arr[N]; //temporary array to make prefix
    for(int i=0;i<N;i++) arr[i]=i;
	permutation(N, prefixLen, 0, arr);
}

void openFile(char* filename){
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

void printStat(void){
    printf("checked routes :\n");
    for(int i=0;i<N;i++){
        for (int j=0;j<N;j++) {
            if(path[i][j]!=-1)
                printf("%d ",path[i][j]);
        }
        printf("\n");
    }
    printf("current best ans : %d\n", ans);
    printf("current best solution :\n");
    for(int i=0;i<N;i++){
        printf("%d ",best_path[i]);
    }
    printf("\n");
}

void printThreads(void){
    printf("Index\tTID\t\tsearched route\n");
    for(int i = 0; i < numOfConsumer; i++){
        printf("%d\t%lu\t\t%d\n", i+1, cons_tid[i], cons_srch_cnt[i]);
    }
}

void assignTask(void* _THIS){
	int THIS = atoi(_THIS);
    currNum_cons_tid[THIS % numOfConsumer] = THIS;
}



void changeNumConsumer(void ){
    int num;
    printf("Enter a number to set number of threads : ");
    scanf("%d",&num);
    if (1 > num || num > 8){
        printf("You need to enter number between 1 to 8\n");
        return ;
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    int THIS_buffer[8]={-1,-1,-1,-1,-1,-1,-1,-1};
    int buffer = 0;

    for (int i = 0; i < numOfConsumer; i++){ //기존의 consumer
        pthread_cancel(cons_tid[i]);
        booking_cons_tid[i] = 0;
        if(currNum_cons_tid[i]!=-1){ //if the work has not terminated
            THIS_buffer[buffer] = currNum_cons_tid[i];
            buffer++;
            currNum_cons_tid[i] = -1;
        }
    }
    numOfConsumer = num;
    //works that has not done yet
    for (int i = 0; i < buffer; i++){
        int THIS = THIS_buffer[i];
		char buffer[32];
		sprintf(buffer,"%d",THIS);
        pthread_create(&cons_tid[THIS % num], &attr, consumerRunner,buffer);
    }
}

void *producerRunner(void *param){
	printf("producer start\n");
    makePrefix(); //assign task to consumer
    while (1){
        int option = 0;
        printf("Enter option\n");
        printf("1. Print current status\n");
        printf("2. print current threads information\n");
        printf("3. Change Num of Threads\n");
        printf("4. Quit this program\n");
        scanf("%d",&option);

        if (option == 1){
            printStat();
        }

        else if (option == 2){
            printThreads();
        }

        else if (option == 3){
            changeNumConsumer();
        }
        else if(option == 4){
            break;
        }
        else;
    }
    
    pthread_exit(0);
}

void find_path(int THIS, int start, int last, int tmp_dist, int curr, int* length, int used[], int child_path[], int* best_child_path){

    int FLAG = 1;
    #ifdef DEBUG
    printf("start : %d, last : %d\n",start,last);
    printf("tmp_dist till now : %d\n",tmp_dist);
    printf("curr : %d\n\n",curr);
    #endif
    for(int i=0;i<N;i++){
        //if there is any city that you haven't visited
        if(used[i] == 0) FLAG = 0; 
        cons_srch_cnt[THIS % numOfConsumer]++;
	    #ifdef DEBUG
        printf("used[%d]:%d\n",i,used[i]);
        #endif
    }
    //if you visited all the cities
    if (FLAG==1){
        int return_home_dist = dist[last][start];
	    #ifdef DEBUG
        printf("visited all the cities\n");
	    printf("length : %d\n",*length);
	    printf("new length : %d\n",tmp_dist+return_home_dist);
	    #endif
        if(*length > (tmp_dist + return_home_dist)){ //if length has to be updated
            *length = tmp_dist + return_home_dist;
            for(int i=0;i<N;i++){
                best_child_path[i]=child_path[i];
		    }
            #ifdef DEBUG //print the path
            for(int i=0;i<N;i++){
                printf("%d-",best_child_path[i]);
            }
	        printf("%d\n",best_child_path[0]);
	        #endif
        }
    } else 
    for(int left=0;left<N;left++)
        if(used[left]==0){ //loop for left cities
            #ifdef DEBUG
            printf("left : %d\n",left);
            #endif
            used[left]=1;
            child_path[curr]=left;
            curr++;

            find_path(THIS, start,left,tmp_dist+dist[last][left], curr, length, used, child_path, best_child_path);
    
            used[left]=0;
            child_path[curr]=0;
            curr--;
        }

    
}

void *consumerRunner(void * _THIS){
    #ifdef DEBUG
	printf("consumer runner starts\n");
    #endif
	int THIS =atoi(_THIS)-1;
	printf("THIS : %d\n",THIS);
    int terminated=1;
	
    pthread_cleanup_push(assignTask,_THIS);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    
    int start = prefix[THIS][0]; //first of prefix
    int last = prefix[THIS][prefixLen-1]; //last of prefix
    int curr = prefixLen; //cursor of this path
    int used[N]; //array to show whether some citiy visited
    int child_path[N]; //buffer variable to search
    int best_child_path[N]; //best path of this prefix

    for(int i=0;i<N;i++){
        used[i]=child_path[i]=0; //Initialization
    }

    for(int i=0;i<prefixLen;i++){
        child_path[i]=prefix[THIS][i]; //update prefix's visit
        used[child_path[i]]=1;
        #ifdef DEBUG
        printf("used[%d]=1\n",child_path[i]);
        printf("child_path[%d]=%d\n",i,prefix[THIS][i]);
        #endif
    }

    int tmp_dist = 0;
    for(int i=0;i<prefixLen-1;i++){
        tmp_dist += dist[child_path[i]][child_path[i+1]]; //update temp distance
        #ifdef DEBUG
        printf("tmp_dist : %d\n",tmp_dist);
        #endif
    }
    int length = 99999; //maximum of length
    #ifdef DEBUG
	printf("start finding path\n");
    #endif

    find_path(THIS, start, last, tmp_dist, curr, &length, used, child_path, &best_child_path[0]);
	
    #ifdef DEBUG
    printf("finished finding path\n");
    printf("%d's best path\n",THIS);
    #endif

    for(int i=0;i<N;i++)
        path[THIS][i]=best_child_path[i];
    
    if(ans > length){
	    ans = length;
	    for(int i=0;i<N;i++){
	        best_path[i]=best_child_path[i];
	    }
    }
    #ifdef DEBUG
    printf("best path:\n");
    for(int i=0;i<N;i++){
	    printf("%d-",best_path[i]);
    }
    printf("%d\n",best_path[0]);
    printf("ans : %d\n\n",ans);
    #endif
    
    //RE-Initialization
    for(int i=0;i<N;i++){
        used[i]=child_path[i]=0;
    }
    tmp_dist = 0;
    length = 99999;
    terminated = 0;
    booking_cons_tid[THIS % numOfConsumer]=0;
    cons_srch_cnt[THIS%numOfConsumer]=0;
    
    pthread_cleanup_pop(terminated);
    //if work is done, just terminate the thread
    //if work has not done, record THIS on currNum_cons_tid array
    pthread_exit(0);
}

int main(int argc, char* argv[]){
    if(argc == 3){
        openFile(argv[1]);
        numOfConsumer = atoi(argv[2]);
    }
    else exit(0);
    if(numOfConsumer < 1 || numOfConsumer > 8){
        numOfConsumer = 8; //default of numOfConsumer is 8
    }
	for(int i=N ; i>cal_limit ; i--){
		prefixCase *= i;
	}
    
    signal(SIGINT, terminate_handler);
    pthread_attr_t attr;
    int curr = 0;
	char buffer[32];
	int i;
    pthread_attr_init(&attr);
    pthread_create(&prod_tid, &attr, producerRunner, NULL);
    //pthread_join(prod_tid,NULL);
    #ifdef DEBUG
    printf("create producer\n");
    #endif
    while(curr < prefixCase){
        //until all the works are finished
        #ifdef DEBUG
		printf("curr : %d, prefixCase : %d\n",curr,prefixCase);
        #endif
        if(curr <= prefixCase){ //WORK!
            curr++;
            i = curr % numOfConsumer;
			//printf("%d consumer\n",i);
            while(booking_cons_tid[i]==1);
			sprintf(buffer,"%d",curr);
            #ifdef DEBUG
            printf("curr : %d\n",curr);
            #endif
            booking_cons_tid[i]=1;
            pthread_create(&cons_tid[i], &attr, consumerRunner, buffer);
            printf("create consumer\n");
			//pthread_join(cons_tid[i],NULL);
        }
    }
    if(curr > prefixCase)
        exit(0);
}

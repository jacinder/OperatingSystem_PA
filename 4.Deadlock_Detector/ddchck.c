#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#define DEBUG
pid_t command;

void FindIndex(unsigned long tid, int rsc, int* tid_index, int* rsc_index, unsigned long * process_index, int * mutex_index, int* mutex_cur){
	int new = -1;
	int temp_tid_index = -1;
	int temp_rsc_index = -1;
	//process_index
	for(int i=0;i<10;i++){
		//아직 프로세스 인덱스가 채워지지 않았는데 new도 초기화 상태라면 new는 i.
		if((process_index[i] == 0)&&(new == -1)){
			new = i;
		}
		//만약 해당 tid가 process_index안에 있다면 그게 i
		if (tid == process_index[i]){
			temp_tid_index = i;
			#ifdef DEBUG
				printf("tid is found in process_index[%d]\n",i);
			#endif
		}
	}
	//for문을 돌았는데 temp_tid_index가 -1이라면 프로세스 인덱스 중 없었다는 뜻.
	//새로 추가를 해줘야함.
	if(temp_tid_index == -1){
		temp_tid_index = new;
		process_index[new] = tid;
		#ifdef DEBUG
			printf("tid is new one : %d\n",new);
		#endif
	}
	//main의 tid_index에 temp_tid_index내용을 저장해주기.
	*tid_index = temp_tid_index;
	#ifdef DEBUG
		printf("FindIndex process_index[%d]:%lu\n",temp_tid_index,process_index[temp_tid_index]);
	#endif

	new = -1;
	//mutex_index
	for(int i=0;i<10;i++){
		//아직 뮤텍스 인덱스가 채워지지 않았는데 new도 초기화 상태라면 new는 i.
		if((mutex_index[i] == 0)&&(new == -1)){
			new = i;
		}
		//만약 해당 rsc가 mutex_index안에 있다면 그게 i
		if (rsc == mutex_index[i]){
			temp_rsc_index = i;
			#ifdef DEBUG
				printf("rsc is found in process_index[%d]\n",i);
			#endif
		}
	}
	//for문을 돌았는데 temp_tid_index가 -1이라면 프로세스 인덱스 중 없었다는 뜻.
	//새로 추가를 해줘야함.
	if(temp_rsc_index == -1){
		temp_rsc_index = new;
		mutex_index[new] = rsc;
		#ifdef DEBUG
			printf("rsc is new one : %d\n",new);
		#endif
	}
	*rsc_index = temp_rsc_index;
	#ifdef DEBUG
		printf("FindIndex mutex_index[%d]:%p\n",temp_rsc_index,mutex_index[temp_rsc_index]);
	#endif
}
int getMin(int* array){
	//0이 아닌 수 중에서 min
	int min;
	for(int i=0;i<10;i++){
		if((array[i]>0)&&(array[i]<min)){
			min = array[i];
		}
	}
	return min;
}
void setDPmatrix(int** P, int** D, int lock, int tid, int rsc){
	if(lock == 1){ //lock
		//P[m,]에 10이 있으면 0이 아닌 수 중 min의 -1
		for(int i=0;i<10;i++){
			if(P[rsc][i]==10){
				if(i == tid){ //self - deadlock
					printf("***self deadlock detected***\n");
					return ;
				}
				else{
					P[rsc][tid]=getMin(P[rsc])-1;
					break;
				}
			}
		}
		//만약 10이 없었다면 내가 10!(오너)
		if(P[rsc][tid]==0) P[rsc][tid] = 10;
		//col에 10이 있으면서 10이 아닌 수가 있다면 D를 채워줘야함
		for(int i=0;i<10;i++){
			if(P[i][tid]==10 && P[rsc][tid]!=10){
				D[i][rsc] = 1;
			}
		}
	}
	if(lock == 0){ //unlock
		//일단 P[rsc][tid]를 0으로 만들어주기
		P[rsc][tid] = 0;
		int x = -1;
		//P[rsc]에 10보다 작은 waiting하는 친구들은 우선순위 하나씩++
		for(int i=0;i<10;i++){
			if(P[rsc][i]!=0){
				P[rsc][i] += 1;
				if(P[rsc][i] == 10){
					x = i;
				}
			}
		}
		//D matrix update
		//D[rsc]중에 1이 있었다면 이번에 10된 D[rsc][x]=0
		if(x != -1) D[rsc][x] = 0;
	}
}
void printDeadlock(int* mutex_index, unsigned long * process_index, int** P,int i){
	//i에 해당하는 인덱스의 mutex를 찾는다.
	int deadlock_mutex = 0, deadlock_tid_index = -1;
	unsigned long deadlock_tid = 0;
	deadlock_mutex = mutex_index[i];
	//해당 mutex를 소유한 tid를 찾는다.
	for(int m=0;m<10;m++){
		if(P[i][m] == 10){
			deadlock_tid_index = m;
		}
	}
	deadlock_tid = process_index[deadlock_tid_index];
	//출력한다.
	printf("Dead ocurred with %lu thread, %p mutex\n",deadlock_tid, deadlock_mutex);
}
void DFS_D_matrix(int **D, int* visited, int curr, int ** P, int* mutex_index, unsigned long * process_index){
	int i;
	visited[curr] = 1;
	for(int i=0;i<10;i++){
		if(D[curr][i]==1){
			if(visited[i] == 0){
				DFS_D_matrix(D, visited, i, P, mutex_index, process_index);
			}
			else{
				//tid와 mutex id출력해야함
				printDeadlock(mutex_index, process_index, P, i);
				printf("***Deadlock detected***\n");
				exit(0);
			}
		}
	}
}
int main (int argc, char* argv[]){
	if(argc != 2){
		printf("Enter a file name\n");
		exit(0);
	}
	char cmd[256] = "LE_PRELOAD=\"./ddmon.so\" ./";
	strcat(cmd, argv[1]);
	printf("%s\n",cmd);
	command = fork();
	if(command == 0){
		system(cmd);
		return 0;
	}
    unsigned long tid = 0;
	int rsc;
	int lock = 0, mutex_cur = 0;
	//lock : 1, unlock : 0
	int fd = open(".ddtrace", O_RDONLY | O_SYNC) ;
	unsigned long * process_index = (unsigned long *)malloc(sizeof(unsigned long)*10);
	int* mutex_index = (int *)malloc(sizeof(int*)*10);
	int* visited = (int *)malloc(sizeof(int*)*10);
	int**D, **P;

	for(int i=0;i<10;i++){
		mutex_index[i] = 0;
		process_index[i] = 0;
		visited[i] = 0;
	}
	P = (int**)malloc(sizeof(int**)*10); //process * mutex
	for(int i=0;i<10;i++){
		P[i] = (int*)malloc(sizeof(int*)*10);
		for(int j=0;j<10;j++) P[i][j] = 0;
	}
	D = (int**)malloc(sizeof(int**)*10); //mutex * mutex
	for(int i=0;i<10;i++){
		D[i] = (int*)malloc(sizeof(int*)*10);
		for(int j=0;j<10;j++) D[i][j] = 0;
	}
	
	
	while (1) {
		char msg[128] ;
		int len ;
		if ((len = read(fd, msg, 128)) == -1)
			break ;
		if (len > 0){
            printf("%s\n", msg);
			sscanf(msg,"%lu/%p/%d",&tid,&rsc,&lock);
			//tid, rsc의 인덱스 찾기
			int tid_index=-1, rsc_index=-1;
			FindIndex(tid, rsc, &tid_index, &rsc_index, process_index, mutex_index, &mutex_cur);
			#ifdef DEBUG
				printf("tid index : %d\n",tid_index);
				printf("rsc index : %d\n",rsc_index);
			#endif
			setDPmatrix(P,D,lock,tid_index,rsc_index);
			#ifdef DEBUG
				printf("P : \n");
				for(int i=0;i<10;i++){
					for(int j=0;j<10;j++){
						printf("%d\t",P[i][j]);
					}
					printf("\n");
				}
				printf("D : \n");
				for(int i=0;i<10;i++){
					for(int j=0;j<10;j++){
						printf("%d\t",D[i][j]);
					}
					printf("\n");
				}
			#endif
			for(int start = 0;start<10;start++){
				for(int i=0;i<10;i++) visited[i] = 0;
				DFS_D_matrix(D, visited, start, P, mutex_index, process_index);
			}
        }
	}
	close(fd) ;

	return 0 ;
}

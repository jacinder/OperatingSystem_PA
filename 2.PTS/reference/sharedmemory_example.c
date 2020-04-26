#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define name "/buffer"

// struct for message buffer
typedef struct {
	char message[512];			// message buffer
	int filled;					// 1 if the message is filled. 0 otherwise.
} Buffer;
int size = sizeof(Buffer);
Buffer global_buffer;

void parent(int shm_id);		// function for the parent
void child(int shm_id);			// function for the child

int main(int argc, char argv[0])
{
	int shm_id = 0;

	// hint: allocate shared memory block for the shared buffer

	shm_id = shm_open(name, O_CREAT | O_RDWR, 0666);
	//쉐어드 메모리를 만들겠다
	//name : 이름
	//O_CREAT : 없으면 만들겠다
	//O_RDWR : 읽고 쓰기가 가능
	//0666 : 권한
	//리턴값은 쉐어드 메모리 아이디

	ftruncate(shm_id,sizeof(Buffer));
	//쉐어드 메모리의 크기
	//쉐어드 메모리의 형태는 구조체이다. : 사람들이 가장 많이 쓰는 구조

	pid_t child_pid = fork();
	//포크!

	if(child_pid < 0){
		printf("Failed to create child process\n");
		exit(-1);
	} else if(child_pid > 0){
		//부모
		parent(shm_id);
	} else {
		//자식
		child(shm_id);
	}

	return 0;
}

void parent(int shm_id)
{
	Buffer *buffer = (Buffer*)mmap(0,size,PROT_WRITE,MAP_SHARED,shm_id,0);		// hint: modify this line
	//매핑을 시켜주는 mmap
	//PROT_WRITE : 쓰는 기능
	//부모에서 접근하는 쉐어드 메모리의 주솟값 -> buffer

	buffer->filled = 0;
	//filled는 글을 썼는지/ 안썼는지

	sleep(2);			// wait for the child to start
	printf("[parent] Input a message and I'll send to the child.\n");

	while(1){			// DO NOT print any message in this loop.
		char message[512] = "";
		message[511]
        0	// for safety

		fgets(message, 512, stdin);
		message[strlen(message) - 1] = 0;	// trim '\n'
		//맨 마지막을 \0로
		
		while(buffer->filled)
			usleep(50000);

		strcpy(buffer->message, message);
		buffer->filled = 1;

		if(strcmp(message, "quit") == 0)
			break;
	}

	// hint: put some code here
	munmap(buffer,);
	
	printf("[parent] Terminating.\n");
	fflush(stdout);
}

void child(int shm_id)
{
	sleep(1);			// wait for the parent to initialize the shared buffer
	//쉐어드가 먼저 매핑

	printf("[child] Started\n");
	fflush(stdout);

	Buffer *buffer = (Buffer*)mmap(0,size,PROT_READ | PROT_WRITE,MAP_SHARED,shm_id,0);
	
	while(1){
		
		while(!buffer->filled)
			usleep(50000);
		if(buffer->message[0]){
			printf("[child] %s\n", buffer->message);
			fflush(stdout);
			buffer->filled = 0;
		}

		if(strcmp(buffer->message, "quit") == 0)
			break;
	}

	// hint: put some code here
	munmap(buffer,size);
	//매핑이 사라짐
	shm_unlink(name);
	//쉐어드 메모리 삭제
    
	printf("[child] Terminating.\n");
	fflush(stdout);
}


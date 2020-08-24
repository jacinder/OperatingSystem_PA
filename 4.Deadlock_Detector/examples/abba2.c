#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <dlfcn.h>
#include <execinfo.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex4 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex5 = PTHREAD_MUTEX_INITIALIZER;

void 
noise()
{
    usleep(rand() % 1000) ;
}

void *
thread1(void *arg)
{
    pthread_mutex_lock(&mutex5); noise() ;
    pthread_mutex_lock(&mutex2);
    noise() ;

    pthread_mutex_unlock(&mutex5); noise() ;
    pthread_mutex_unlock(&mutex2); noise() ;
}

void *
thread2(void *arg)
{
    pthread_mutex_lock(&mutex4); noise() ;
    pthread_mutex_lock(&mutex5); noise() ;

    pthread_mutex_unlock(&mutex4); noise() ;
    pthread_mutex_unlock(&mutex5); noise() ;
}

void *
thread3(void *arg)
{
    pthread_mutex_lock(&mutex3); noise() ;
    pthread_mutex_lock(&mutex4); noise() ;

    pthread_mutex_unlock(&mutex3); noise() ;
    pthread_mutex_unlock(&mutex4); noise() ;
}

void *
thread4(void *arg)
{
    pthread_mutex_lock(&mutex2); noise() ;
    pthread_mutex_lock(&mutex3); noise() ;

    pthread_mutex_unlock(&mutex2); noise() ;
    pthread_mutex_unlock(&mutex3); noise() ;
}

void *
thread5(void *arg)
{
    pthread_mutex_lock(&mutex1); noise() ;
    pthread_mutex_lock(&mutex2); noise() ;

    pthread_mutex_unlock(&mutex1); noise() ;
    pthread_mutex_unlock(&mutex2); noise() ;

    return NULL;
}

int 
main(int argc, char *argv[]) 
{
	pthread_t tid1, tid2, tid3, tid4, tid5;
	srand(time(0x0)) ;

	pthread_create(&tid1, NULL, thread1, NULL);
	pthread_create(&tid2, NULL, thread2, NULL);
	pthread_create(&tid3, NULL, thread3, NULL);
	pthread_create(&tid4, NULL, thread4, NULL);
	pthread_create(&tid5, NULL, thread5, NULL);


	pthread_join(tid5, NULL);
	pthread_join(tid4, NULL);
	pthread_join(tid3, NULL);
	pthread_join(tid2, NULL);
	pthread_join(tid1, NULL);
	

	return 0;
}


#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>

// gcc -shared -fPIC -o ddmon.so ddmon.c -ldl
//LD_PRELOAD=./ddmon.so ./dinning_deadlock

int pthread_mutex_lock(pthread_mutex_t * mutex){

    int (*normal_lock)(pthread_mutex_t * mutex);
    char * error;
    
    //normal pthread_mutex_lock
    normal_lock = dlsym(RTLD_NEXT, "pthread_mutex_lock");
    if ((error = dlerror()) != 0x0){
		  exit(1);
    }
    
    //write info in named pipe
    char msg[128];
    int i = 0;

    int fd = open(".ddtrace", O_WRONLY | O_SYNC) ;
    //thread id / resource id / 1
    sprintf(msg,"%lu/%p/1",pthread_self(), mutex);
    for (int i = 0 ; i < 128; ){
      i += write(fd, msg + i, 128);
    } 
    close(fd);
    printf("lock : %s\n",msg);

    int lock_result = normal_lock(mutex);
    return lock_result;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex){
    int (*normal_unlock)(pthread_mutex_t *mutex);
    char * error;
    
    //normal pthread_mutex_lock
    normal_unlock = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
    if ((error = dlerror()) != 0x0){
		  exit(1);
    }
    
    //write info in pipe
    int fd = open(".ddtrace", O_WRONLY | O_SYNC) ;

    char msg[128];
    int i = 0;
    //thread id / resource id / 0
    sprintf(msg,"%lu/%p/0",pthread_self(), mutex);
    for (int i = 0 ; i < 128; ){
      i += write(fd, msg + i, 128);
    } 
    close(fd);
    printf("unlock : %s\n",msg);

    int unlock_result = normal_unlock(mutex);
    return unlock_result;
}

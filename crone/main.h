
#ifndef HEADER_FILE
#define HEADER_FILE
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <syslog.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdatomic.h>
#include <string.h> 
#include <semaphore.h> 
#include<semaphore.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/mman.h>
struct queue {
sem_t main_file;

};

#endif

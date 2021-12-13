#if !defined(_COMMON_H_)
#define _COMMON_H_
#include<stdio.h>
#include<semaphore.h>
#include <pthread.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<unistd.h>
#include<time.h>

#define SIZE_OF_ARRAY_X 31
#define SIZE_OF_ARRAY_Y 42

static void err(int c, const char* msg) {
if (!c)
return;
perror(msg);
exit(1);
}

struct queue {
char PID[3];
sem_t queue;
sem_t wait_for_data;
sem_t wait_for_receive_data;
int full;
};


struct player {
	char under;
	int is_init; 	//
	int id;		//
	char PID[3];//
	int number;//
	char type[6];
	int x;
	int y;
	int x_spawn;
	int y_spawn;
	int death;//
	int coinsf;//
	int coinsb;	//
	char map[5][5];
	sem_t lock;
	int move;	
};
struct drop{
	int is_init;
	int value;
	int x;
	int y;
};


struct serwer {
char map[SIZE_OF_ARRAY_X][SIZE_OF_ARRAY_Y];
int players;
int drop_counter;
struct drop *coins[40];
struct player *tab[4];
sem_t serwer_access;
struct queue * queue;
};


#endif // _COMMON_H_
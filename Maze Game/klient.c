#include "common.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
struct player *init_queue(int *a)
{
	int choice = 0;
	int b;
	b = shm_open("queue", O_RDWR, 0666);
	err(b == -1, "shm_open");
	ftruncate(b, sizeof(struct queue));
	struct queue *pdata_queue;
	pdata_queue = (struct queue *)
		mmap(NULL, sizeof(struct queue),
			 PROT_READ | PROT_WRITE, MAP_SHARED,
			 b, 0);

	err(pdata_queue == NULL, "mmap");
	if (pdata_queue->full == 1)
	{
		printf("Full\n");
		return NULL;
	}

	printf("Bot=0, Human=1");
	scanf("%d", &choice);
	system("clear");
	sem_post(&pdata_queue->queue);
	sem_wait(&pdata_queue->wait_for_data);

	*a = shm_open(pdata_queue->PID, O_RDWR, 0666);
	err(*a == -1, "shm_open");
	ftruncate(*a, sizeof(struct player));
	struct player *pdata_player;
	pdata_player = (struct player *)
		mmap(NULL, sizeof(struct player),
			 PROT_READ | PROT_WRITE, MAP_SHARED,
			 *a, 0);

	err(pdata_player == NULL, "mmap");

	close(b);

	if (choice == 1)
	{
		pdata_player->type[0] = 'H';
		pdata_player->type[1] = 'U';
		pdata_player->type[2] = 'M';
		pdata_player->type[3] = 'A';
		pdata_player->type[4] = 'N';
	}
	else if (choice == 0)
	{
		pdata_player->type[0] = 'B';
		pdata_player->type[1] = 'O';
		pdata_player->type[2] = 'T';
	}
	else
	{
		sem_post(&pdata_queue->wait_for_receive_data);
		return NULL;
	}
	sem_post(&pdata_queue->wait_for_receive_data);

	return pdata_player;
}

void *show(void *co)
{
	system("clear");
	struct player *pdata_player = (struct player *)co;
	int i = 0, j = 0;
	while (1)
	{
		i = 0, j = 0;

		for (i = 0; i < 5; i++)
		{
			for (j = 0; j < 5; j++)
			{

				printf("\033[s\033[%d;%dH%c\033[u", i + 1, j + 1, pdata_player->map[i][j]);
			}
		}
		//printf("id: %dPID:%sNumber:%dDeaths:%dFound:%dBrought:%d\n", pdata_player->id, pdata_player->PID, pdata_player->number, pdata_player->death, pdata_player->coinsf, pdata_player->coinsb);

		printf("\033[s\033[%d;%dHid: %d\033[u", 5, SIZE_OF_ARRAY_X + 2, pdata_player->id);
		printf("\033[s\033[%d;%dHPID: %s\033[u", 6, SIZE_OF_ARRAY_X + 2, pdata_player->PID);
		printf("\033[s\033[%d;%dHCoins found: %d\033[u", 7, SIZE_OF_ARRAY_X + 2, pdata_player->coinsf);
		printf("\033[s\033[%d;%dHCoins brought: %d\033[u", 8, SIZE_OF_ARRAY_X + 2, pdata_player->coinsb);
		printf("\033[s\033[%d;%dHDeaths: %d\033[u", 9, SIZE_OF_ARRAY_X + 2, pdata_player->death);
		usleep(200000);
	}
}

void *fun_move(void *co)
{
	struct player *pdata_player = (struct player *)co;

	char mark;
	while (1)
	{
		mark = getchar();
		sem_wait(&pdata_player->lock);
		if (mark == 'w')
		{
			pdata_player->move = 1;
		}
		if (mark == 's')
		{
			pdata_player->move = 2;
		}
		if (mark == 'a')
		{
			pdata_player->move = 3;
		}
		if (mark == 'd')
		{
			pdata_player->move = 4;
		}
		else if (mark == 'q')
		{
			sem_post(&pdata_player->lock);
			break;
		}
		sem_post(&pdata_player->lock);
	}
	pdata_player->move = -1;

	return NULL;
}

void *fun_move_bot(void *co)
{
	struct player *pdata_player = (struct player *)co;
	srand(time(NULL));
	int mark;
	char marked;
	while (1)
	{
		mark = rand() % 4 + 1;
		pdata_player->move = mark;
		usleep(1000000);
	}
	pdata_player->move = -1;

	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t showmap, move;
	int a;
	struct player *pdata_player = init_queue(&a);
	if (pdata_player == NULL)
		return 0;
	pthread_create(&showmap, NULL, show, pdata_player);
	system("/bin/stty raw -echo");
	if (pdata_player->type[0] == 'H')
		pthread_create(&move, NULL, fun_move, pdata_player);
	else
		pthread_create(&move, NULL, fun_move_bot, pdata_player);
	pthread_join(move, NULL);
	close(a);

	return 0;
}

#include "common.h"
#include <curses.h>
int guard = 0;
struct serwer *init_serwer(int *a)
{
	*a = shm_open("serwer", O_CREAT | O_RDWR, 0666);
	err(*a == -1, "shm_open");
	ftruncate(*a, sizeof(struct serwer));
	struct serwer *pdata_serwer;

	pdata_serwer = (struct serwer *)
		mmap(NULL, sizeof(struct serwer),
			 PROT_READ | PROT_WRITE, MAP_SHARED,
			 *a, 0);
	err(pdata_serwer == NULL, "mmap");

	pdata_serwer->players = 0;
	pdata_serwer->drop_counter = 0;
	sem_init(&pdata_serwer->serwer_access, 1, 1);

	return pdata_serwer;
}

struct queue *init_queue(int *a)
{
	*a = shm_open("queue", O_CREAT | O_RDWR, 0666);
	err(*a == -1, "shm_open");
	ftruncate(*a, sizeof(struct queue));
	struct queue *pdata_queue;
	pdata_queue = (struct queue *)
		mmap(NULL, sizeof(struct queue),
			 PROT_READ | PROT_WRITE, MAP_SHARED,
			 *a, 0);
	err(pdata_queue == NULL, "mmap");
	pdata_queue->full = 0;
	sem_init(&pdata_queue->queue, 1, 0);
	sem_init(&pdata_queue->wait_for_data, 1, 0);
	sem_init(&pdata_queue->wait_for_receive_data, 1, 0);
	return pdata_queue;
}
int wall_second(int x, int y, char tab[SIZE_OF_ARRAY_X][SIZE_OF_ARRAY_Y])
{
	if (tab[x][y] == '#' || tab[x][y] == '0' || tab[x][y] == 'A' || tab[x][y] == '1' || tab[x][y] == '2' || tab[x][y] == '3' || tab[x][y] == 'c' || tab[x][y] == 't' || tab[x][y] == 'T' || tab[x][y] == '*' || tab[x][y] == '%')
		return 0;
	return 1;
}

int wall(int x, int y, char tab[SIZE_OF_ARRAY_X][SIZE_OF_ARRAY_Y])
{
	if (tab[x][y] == '#')
		return 0;
	return 1;
}
void loadmap(struct player *pdata_player, struct serwer *a, int x2, int y2)
{
	int x1 = -2, y1 = -2, x, y;

	for (x = 0; x < 5; x++)
	{
		x1 = -2;
		for (y = 0; y < 5; y++)
		{
			if (x2 + x1 < 0 || x2 + x1 >= SIZE_OF_ARRAY_X || y2 + y1 < 0 || y2 + y1 >= SIZE_OF_ARRAY_Y)
			{
				pdata_player->map[y][x] = '#';
				x1++;
			}
			else
				pdata_player->map[y][x] = a->map[x2 + x1++][y2 + y1];
		}
		y1++;
	}

	return;
}
void *fun_queue(void *co)
{
	srand(time(NULL));
	int x = 0, y = 0, i = 0;
	struct serwer *a = (struct serwer *)co;
	while (1)
	{
		i = 0;
		sem_wait(&a->queue->queue);
		sem_wait(&a->serwer_access);
		if (a->players > 3)
		{
			sem_post(&a->serwer_access);
			continue;
		}
		while (i < 4)
		{
			if (a->tab[i]->is_init == 0)
				break;
			i++;
		}
		a->players++;
		if (a->players == 4)
			a->queue->full = 1;
		a->queue->PID[0] = a->tab[i]->PID[0];
		a->queue->PID[1] = a->tab[i]->PID[1];
		a->queue->PID[2] = a->tab[i]->PID[2];

		do
		{
			x = rand() % (SIZE_OF_ARRAY_X - 1);
			y = rand() % (SIZE_OF_ARRAY_Y - 1);
		} while (!wall(x, y, a->map));
		a->tab[i]->x = x;
		a->tab[i]->y = y;
		a->tab[i]->x_spawn = x;
		a->tab[i]->y_spawn = y;
		a->map[x][y] = a->tab[i]->number + '0';
		loadmap(a->tab[i], a, x, y);
		a->tab[i]->is_init = 1;

		sem_post(&a->queue->wait_for_data);
		sem_wait(&a->queue->wait_for_receive_data);
		sem_post(&a->serwer_access);
	}

	return NULL;
}

void *init_players(void *co)
{
	srand(time(NULL));
	int z = 0, id = 0, i = 0;
	char tab[3];
	struct serwer *a = (struct serwer *)co;
	while (i < 4)
	{
		z = rand() % 999 + 100;
		tab[0] = z % 10 + '0';
		z = z / 10;
		tab[1] = z % 10 + '0';
		z = z / 10;
		tab[2] = z % 10 + '0';
		z = z / 10;

		id = shm_open(tab, O_CREAT | O_RDWR, 0666);
		err(id == -1, "shm_open");
		ftruncate(id, sizeof(struct player));
		struct player *pdata_player;
		pdata_player = (struct player *)
			mmap(NULL, sizeof(struct player),
				 PROT_READ | PROT_WRITE, MAP_SHARED,
				 id, 0);
		err(pdata_player == NULL, "mmap");
		sem_init(&pdata_player->lock, 1, 1);
		a->tab[i] = pdata_player;
		a->tab[i]->PID[0] = tab[0];
		a->tab[i]->PID[1] = tab[1];
		a->tab[i]->PID[2] = tab[2];

		a->tab[i]->under = ' ';
		a->tab[i]->id = id;
		a->tab[i]->number = i;
		a->tab[i]->death = 0;
		a->tab[i]->coinsf = 0;
		a->tab[i]->coinsb = 0;
		a->tab[i]->move = 0;
		a->tab[i]->is_init = 0;

		i++;
	}

	return NULL;
}

void *init_coins(void *co)
{
	srand(time(NULL));
	int i = 0;
	struct serwer *a = (struct serwer *)co;
	while (i < 40)
	{
		a->coins[i] = (struct drop *)calloc(1, sizeof(struct drop));
		a->coins[i]->is_init = 0;
		i++;
	}
	return NULL;
}

void *show(void *co)
{
	struct serwer *pdata_serwer = (struct serwer *)co;
	int i = 0, j = 0;
	while (1)
	{
		sem_wait(&pdata_serwer->serwer_access);
		
		i = 0, j = 0;

		for (i = 0; i < SIZE_OF_ARRAY_X; i++)
		{
			for (j = 0; j < SIZE_OF_ARRAY_Y; j++)
			{
				printf("%c", pdata_serwer->map[i][j]);
			}
			printf("\n");
		}
		//printf("\033[s\033[%d;%dH%s\033[u",i,j,tab[z++]);
		sem_post(&pdata_serwer->serwer_access);
		usleep(100000);
		system("clear");
	}
}
void colission(struct serwer *ab, struct player *a, char znak)
{
	int num = znak - '0';
	ab->map[a->x_spawn][a->y_spawn] = '0' + a->number;
	ab->map[ab->tab[num]->x_spawn][ab->tab[num]->y_spawn] = '0' + num;
	ab->tab[num]->death++;
	a->death++;
	if ((a->coinsf + ab->tab[num]->coinsf) != 0)
	{
		ab->map[a->x][a->y] = 'D';
		ab->map[ab->tab[num]->x][ab->tab[num]->y] = ab->tab[num]->under;
		int i = 0;
		while (i < 40)
		{
			if (ab->coins[i]->is_init == 0)
				break;
			i++;
		}
		ab->drop_counter++;
		ab->coins[i]->value = a->coinsf + ab->tab[num]->coinsf;
		ab->coins[i]->is_init = 1;
		ab->coins[i]->x = a->x;
		ab->coins[i]->y = a->y;
	}
	else
	{
		ab->map[a->x][a->y] = a->under;
		ab->map[ab->tab[num]->x][ab->tab[num]->y] = ab->tab[num]->under;
	}
	a->x = a->x_spawn;
	a->y = a->y_spawn;
	ab->tab[num]->x = ab->tab[num]->x_spawn;
	ab->tab[num]->y = ab->tab[num]->y_spawn;

	ab->tab[num]->move = 0;
	a->move = 0;
	a->coinsf = 0;
	ab->tab[num]->coinsf = 0;
	a->under = ' ';
	ab->tab[num]->under = ' ';
}

void spawn(struct serwer *ab, struct player *a)
{
	ab->map[a->x_spawn][a->y_spawn] = '0' + a->number;
	a->death++;
	if (a->coinsf != 0)
	{
		ab->map[a->x][a->y] = 'D';
		int i = 0;
		while (i < 40)
		{
			if (ab->coins[i]->is_init == 0)
				break;
			i++;
		}
		ab->drop_counter++;
		ab->coins[i]->value = a->coinsf;
		ab->coins[i]->is_init = 1;
		ab->coins[i]->x = a->x;
		ab->coins[i]->y = a->y;
	}
	else
		ab->map[a->x][a->y] = a->under;
	a->x = a->x_spawn;
	a->y = a->y_spawn;
	a->move = 0;
	a->coinsf = 0;
}
void relocation(struct serwer *ab, struct player *a, int x, int y)
{
	char znak;
	if (ab->map[a->x + x][a->y + y] == 'c')
		a->coinsf += 1;
	if (ab->map[a->x + x][a->y + y] == 't')
		a->coinsf += 10;
	if (ab->map[a->x + x][a->y + y] == 'T')
		a->coinsf += 50;
	if (ab->map[a->x + x][a->y + y] == 'A')
	{
		a->coinsb += a->coinsf;
		a->coinsf = 0;
	}
	int i = 0;
	if (ab->map[a->x + x][a->y + y] == 'D')
	{
		while (i < 40)
		{
			if (ab->coins[i]->x == a->x + x && ab->coins[i]->y == a->y + y)
				break;
			i++;
		}
		a->coinsf += ab->coins[i]->value;
		ab->coins[i]->is_init = 0;
		ab->drop_counter--;
	}
	if (ab->map[a->x + x][a->y + y] == '1' || ab->map[a->x + x][a->y + y] == '2' || ab->map[a->x + x][a->y + y] == '3' || ab->map[a->x + x][a->y + y] == '0')
	{
		colission(ab, a, ab->map[a->x + x][a->y + y]);
		guard = 1;
	}

	if (ab->map[a->x + x][a->y + y] == '*')
	{
		spawn(ab, a);
		guard = 1;
	}
	if (!guard)
	{
		znak = ab->map[a->x + x][a->y + y];

		if (znak == '%')
		{
			sem_post(&ab->serwer_access);
			usleep(1000000);
			sem_wait(&ab->serwer_access);
		}
		if (a->under == '%' || a->under == 'A')
			ab->map[a->x][a->y] = a->under;
		else
			ab->map[a->x][a->y] = ' ';

		ab->map[a->x + x][a->y + y] = a->number + '0';
		a->x += x;
		a->y += y;
		a->under = znak;
	}
	guard = 0;
	return;
}

void *fun_move(void *co)
{
	srand(time(NULL));
	int i = 0;
	struct serwer *a = (struct serwer *)co;
	while (1)
	{
		sem_wait(&a->serwer_access);
		sem_wait(&a->tab[i]->lock);
		
		if (a->tab[i]->move == -1)
		{

			a->tab[i]->move = 0;
			a->tab[i]->under = ' ';
			a->tab[i]->death = 0;
			a->tab[i]->coinsf = 0;
			a->map[a->tab[i]->x][a->tab[i]->y] = ' ';
			a->tab[i]->x = 0;
			a->tab[i]->y = 0;
			a->tab[i]->x_spawn = 0;
			a->tab[i]->y_spawn = 0;
			a->tab[i]->coinsb = 0;
			a->tab[i]->move = 0;
			a->tab[i]->is_init = 0;
			a->players--;
			a->queue->full = 0;
		}
		else if (a->tab[i]->move == 1)
		{
			a->tab[i]->move = 0;
			if (wall(a->tab[i]->x - 1, a->tab[i]->y, a->map))
				relocation(a, a->tab[i], -1, 0);
		}
		else if (a->tab[i]->move == 2)
		{
			a->tab[i]->move = 0;
			if (wall(a->tab[i]->x + 1, a->tab[i]->y, a->map))
				relocation(a, a->tab[i], 1, 0);
		}
		else if (a->tab[i]->move == 3)
		{
			a->tab[i]->move = 0;
			if (wall(a->tab[i]->x, a->tab[i]->y - 1, a->map))
				relocation(a, a->tab[i], 0, -1);
		}
		else if (a->tab[i]->move == 4)
		{
			a->tab[i]->move = 0;
			if (wall(a->tab[i]->x, a->tab[i]->y + 1, a->map))
				relocation(a, a->tab[i], 0, 1);
		}
		loadmap(a->tab[i], a, a->tab[i]->x, a->tab[i]->y);
		sem_post(&a->tab[i]->lock);
		sem_post(&a->serwer_access);

		i++;
		if (i == 4)
		{
			i = 0;
			usleep(1000000);
		}
	}

	return NULL;
}

void loadmap_file(struct serwer *a)
{
	FILE *odczyt;
	int dex = 0, dey = 0;

	odczyt = fopen("out.txt", "rb");

	char znak;
	int licznik = 1;

	fseek(odczyt, -licznik, SEEK_END);
	int y = ftell(odczyt);
	licznik++;
	znak = fgetc(odczyt);
	if (y != 0)
	{
		while (y + 1)
		{
			//    fputc(znak,zapis);
			fseek(odczyt, -licznik, SEEK_END);
			licznik++;
			y--;
			znak = fgetc(odczyt);

			znak = fgetc(odczyt);
			a->map[dex][dey++] = znak;
			printf("%c", znak);
			if (dey == SIZE_OF_ARRAY_Y)
			{
				dey = 0;
				dex++;
				printf("\n");
			}
		}
	}
	srand(time(NULL));
	int x;
	do
	{
		x = rand() % (SIZE_OF_ARRAY_X - 1) + 1;
		y = rand() % (SIZE_OF_ARRAY_Y - 1) + 1;
	} while (a->map[x][y] != ' ');
	a->map[x][y] = 'A';
}
void set(struct serwer *a, char c)
{
	srand(time(NULL));
	int x, y;
	do
	{
		x = rand() % (SIZE_OF_ARRAY_X - 1) + 1;
		y = rand() % (SIZE_OF_ARRAY_Y - 1) + 1;
	} while (a->map[x][y] != ' ');
	a->map[x][y] = c;

	return;
}
int is_player(int x, int y, char tab[SIZE_OF_ARRAY_X][SIZE_OF_ARRAY_Y])
{
	if (tab[x][y] == '0' || tab[x][y] == '1' || tab[x][y] == '2' || tab[x][y] == '3')
		return 1;
	return 0;
}
void *spawn_monster(void *co)
{
	struct serwer *a = (struct serwer *)co;
	struct player monster;
	sem_wait(&a->serwer_access);
	int x, y;
	do
	{
		x = (rand() % (SIZE_OF_ARRAY_X - 1)+1);
		y = (rand() % (SIZE_OF_ARRAY_Y - 1)+1);
	} while (!wall_second(x, y, a->map));
	monster.x = x;
	monster.y = y;
	char under = ' ';
	sem_post(&a->serwer_access);
	srand(time(NULL));
	while (1)
	{

		sem_wait(&a->serwer_access);
		
		int x_choice;
		x_choice = rand() % 4;
		switch (x_choice)
		{
		case 0:
			if (!wall(monster.x, monster.y + 1, a->map))
			{
				sem_post(&a->serwer_access);
				continue;
			}
			if (is_player(monster.x, monster.y + 1, a->map))
			{

				a->map[monster.x][monster.y] = under;
				under = 'D';
				a->map[monster.x][monster.y + 1] = '*';
				for (x = 0; x < 4; x++)
				{
					if (a->tab[x]->x == monster.x && a->tab[x]->y == monster.y + 1)
						break;
				}
				int i = 0;
				while (i < 40)
				{
					if (a->coins[i]->is_init == 0)
						break;
					i++;
				}
				a->drop_counter++;
				a->coins[i]->value = a->tab[x]->coinsf;
				a->coins[i]->is_init = 1;
				a->coins[i]->x = a->tab[x]->x;
				a->coins[i]->y = a->tab[x]->y;

				spawn(a, a->tab[x]);
			}
			else
			{
				a->map[monster.x][monster.y] = under;
				under = a->map[monster.x][monster.y + 1];
				a->map[monster.x][monster.y + 1] = '*';
			}
			monster.y++;
			break;
		case 1:
			if (!wall(monster.x, monster.y - 1, a->map))
			{
				sem_post(&a->serwer_access);
				continue;
			}
			if (is_player(monster.x, monster.y - 1, a->map))
			{

				a->map[monster.x][monster.y] = under;
				under = 'D';
				a->map[monster.x][monster.y - 1] = '*';
				for (x = 0; x < 4; x++)
				{
					if (a->tab[x]->x == monster.x && a->tab[x]->y == monster.y - 1)
						break;
				}
				int i = 0;
				while (i < 40)
				{
					if (a->coins[i]->is_init == 0)
						break;
					i++;
				}
				a->drop_counter++;
				a->coins[i]->value = a->tab[x]->coinsf;
				a->coins[i]->is_init = 1;
				a->coins[i]->x = a->tab[x]->x;
				a->coins[i]->y = a->tab[x]->y;

				spawn(a, a->tab[x]);
			}
			else
			{
				a->map[monster.x][monster.y] = under;
				under = a->map[monster.x][monster.y - 1];
				a->map[monster.x][monster.y - 1] = '*';
			}
			monster.y--;
			break;
		case 2:
			if (!wall(monster.x + 1, monster.y, a->map))
			{
				sem_post(&a->serwer_access);
				continue;
			}
			if (is_player(monster.x + 1, monster.y, a->map))
			{

				a->map[monster.x][monster.y] = under;
				under = 'D';
				a->map[monster.x + 1][monster.y] = '*';
				for (x = 0; x < 4; x++)
				{
					if (a->tab[x]->x == monster.x + 1 && a->tab[x]->y == monster.y)
						break;
				}
				int i = 0;
				while (i < 40)
				{
					if (a->coins[i]->is_init == 0)
						break;
					i++;
				}
				a->drop_counter++;
				a->coins[i]->value = a->tab[x]->coinsf;
				a->coins[i]->is_init = 1;
				a->coins[i]->x = a->tab[x]->x;
				a->coins[i]->y = a->tab[x]->y;

				spawn(a, a->tab[x]);
			}
			else
			{
				a->map[monster.x][monster.y] = under;
				under = a->map[monster.x + 1][monster.y];
				a->map[monster.x + 1][monster.y] = '*';
			}
			monster.x++;
			break;
		case 3:
			if (!wall(monster.x - 1, monster.y, a->map))
			{
				sem_post(&a->serwer_access);
				continue;
			}
			if (is_player(monster.x - 1, monster.y, a->map))
			{

				a->map[monster.x][monster.y] = under;
				under = 'D';
				a->map[monster.x - 1][monster.y] = '*';
				for (x = 0; x < 4; x++)
				{
					if (a->tab[x]->x == monster.x - 1 && a->tab[x]->y == monster.y)
						break;
				}
				int i = 0;
				while (i < 40)
				{
					if (a->coins[i]->is_init == 0)
						break;
					i++;
				}
				a->drop_counter++;
				a->coins[i]->value = a->tab[x]->coinsf;
				a->coins[i]->is_init = 1;
				a->coins[i]->x = a->tab[x]->x;
				a->coins[i]->y = a->tab[x]->y;

				spawn(a, a->tab[x]);
			}
			else
			{
				a->map[monster.x][monster.y] = under;
				under = a->map[monster.x - 1][monster.y];
				a->map[monster.x - 1][monster.y] = '*';
			}
			monster.x--;
			break;
		}

		sem_post(&a->serwer_access);

		usleep(1000000);
	}
	return NULL;
}
void *comand(void *co)
{
	struct serwer *a = (struct serwer *)co;

	char mark;
	while (1)
	{
		mark = getchar();
		sem_wait(&a->serwer_access);
		
		if (mark == 'q' || mark == 'Q')
		{
			break;
		}
		if (mark == 'c')
		{
			set(a, 'c');
		}
		if (mark == 't')
		{
			set(a, 't');
		}
		if (mark == 'T')
		{
			set(a, 'T');
		}
		if (mark == '*')
		{
			pthread_t monster;
			pthread_create(&monster, NULL, spawn_monster, a);
		}
		sem_post(&a->serwer_access);
	}

	return NULL;
}
int main(int argc, char **argv)
{

	pthread_t queue, coin, showmap, player, move, com;

	struct queue *pdata_queue;
	struct serwer *pdata_serwer;
	int id, id1;
	pdata_serwer = init_serwer(&id);
	pdata_queue = init_queue(&id1);
	pdata_serwer->queue = pdata_queue;

	loadmap_file(pdata_serwer);
	pthread_create(&showmap, NULL, show, pdata_serwer);
	pthread_create(&player, NULL, init_players, pdata_serwer);
	pthread_join(player, NULL);

	pthread_create(&coin, NULL, init_coins, pdata_serwer);
	pthread_join(coin, NULL);

	pthread_create(&queue, NULL, fun_queue, pdata_serwer);

	pthread_create(&move, NULL, fun_move, pdata_serwer);
	pthread_create(&com, NULL, comand, pdata_serwer);
	pthread_join(com, NULL);

	char tab[4][4] = {0};

	int tab1[4];
	int i = 0, j = 0;
	for (i = 0; i < 40; i++)
	{
		free(pdata_serwer->coins[i]);
	}
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 3; j++)
		{
			tab[i][j] = pdata_serwer->tab[i]->PID[j];
		}
		tab1[i] = pdata_serwer->tab[i]->id;
	}
	for (i = 0; i < 4; i++)
		close(tab1[i]);

	close(id);
	close(id1);
	for (i = 0; i < 4; i++)
		shm_unlink(tab[i]);
	shm_unlink("serwer");
	shm_unlink("queue");

	return 0;
}

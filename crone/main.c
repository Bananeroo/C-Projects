#include "main.h"
struct queue *pdata_queue;
void server_status(int *a)
{
	*a = shm_open("nowy",O_EXCL,0666);
	if(*a!=-1){
		*a = 0; 
		return;
	}
	*a = shm_open("nowy", O_CREAT | O_RDWR, 0666);
	if(*a==-1) return;
	ftruncate(*a, sizeof(struct queue));
	
	pdata_queue = (struct queue *)
		mmap(NULL, sizeof(struct queue),
			 PROT_READ | PROT_WRITE, MAP_SHARED,
			 *a, 0);
	sem_init(&pdata_queue->main_file, 1, 1);

}

void display(){
	FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
	sem_wait(&pdata_queue->main_file);
		
    fp = fopen("/home/asfadfs/Wideo/crone/main.txt", "r");
    if (fp == NULL)
       {
		   sem_post(&pdata_queue->main_file);
		   return ;
		}

    while ((read = getline(&line, &len, fp)) != -1) {
        printf("%s", line);

    }

    fclose(fp);
	sem_post(&pdata_queue->main_file);
    if (line)
        free(line);

}

void run_script(void * arg){
	
	char *a = (char *) arg;
	
	if(strstr(a, "empty") != NULL) {
		return;
	}
	

	if(strstr(a, "echo") != NULL) {
		system(a);
	}
	else{
			char *tmp[1024];

		strcat(tmp,"gnome-terminal --command=/home/asfadfs/Wideo/crone/");
		strcat(tmp,a);
		system(tmp);
		memset(tmp, 0, 1024);
	}
	return NULL;
}

void * request_handler(void * arg){

		sem_wait(&pdata_queue->main_file);
		FILE * fp;
		char * line = NULL;
		size_t len = 0;
		ssize_t read;	
		fp = fopen("/home/asfadfs/Wideo/crone/main.txt", "r");
		if (fp == NULL)
		{
			sem_post(&pdata_queue->main_file);
			return ;
		}
		time_t rawtime = time(NULL);
   		struct tm *ptm = localtime(&rawtime);
			
		printf("The time is: %02d:%02d:%02d\n", ptm->tm_hour, 
				ptm->tm_min, ptm->tm_sec);
	
			
		while ((read = getline(&line, &len, fp)) != -1) {
		
			int init_size = strlen(line);
			char delim[] = " ";


			char *ptr = strtok(line, delim);
			if(ptr==NULL) continue;
			if(*ptr!='!'){
				int zmienna=atoi(ptr);
				if(zmienna!= ptm->tm_min) continue;
			}
			ptr = strtok(NULL, delim);
			if(*ptr!='!'){
				int zmienna=atoi(ptr);
				if(zmienna!= ptm->tm_hour) continue;
			}
			ptr = strtok(NULL, delim);
			if(*ptr!='!'){
				int zmienna=atoi(ptr);
				if(zmienna!= ptm->tm_mday) continue;
			}
			ptr = strtok(NULL, delim);
			if(*ptr!='!'){
				int zmienna=atoi(ptr);
				if(zmienna!= ptm->tm_mon) continue;
			}
			ptr = strtok(NULL, delim);
			if(*ptr!='!'){
				int zmienna=atoi(ptr);
				if(zmienna!= ptm->tm_wday) continue;
			}
			char *temp[1024];
				ptr = strtok(NULL, delim);
			while(ptr != NULL)
			{	
			
				if(*ptr!='\n'){strcat(temp,ptr); strcat(temp," "); }
				ptr = strtok(NULL, delim);
			
			}
			
			run_script(temp);
		
			memset(temp, 0, 1024);
}

		sem_post(&pdata_queue->main_file);

	return NULL;
}

int validate(char *a){
	int init_size = strlen(a);
	char delim[] = " ";

	char *ptr = strtok(a, delim);

	if(*ptr!= '!')
		if(atoi(ptr)>59 || atoi(ptr)<0) return 0;
		ptr = strtok(NULL, delim);
	if(*ptr!= '!')
	if( atoi(ptr)>23 || atoi(ptr)<0) return 0;
		ptr = strtok(NULL, delim);
	if(*ptr!= '!')
	if( atoi(ptr)>31 || atoi(ptr)<1) return 0;
		ptr = strtok(NULL, delim);
	if(*ptr!= '!')
	if( atoi(ptr)>12 || atoi(ptr)<1) return 0;
		ptr = strtok(NULL, delim);
	if(*ptr!= '!')
	if(atoi(ptr)>7 || atoi(ptr)<0) return 0;
		
	return 1;

	
}
void add_job(char * a){

	char *zm;
	strcat(zm,a);
	int tmp=validate(zm);
	if(tmp==0) {printf("Nie poprawny wpis"); return ;}
	sem_wait(&pdata_queue->main_file);
	FILE * fp;

    fp = fopen("/home/asfadfs/Wideo/crone/main.txt", "a+");
    if (fp == NULL)
    {
	  sem_post(&pdata_queue->main_file);
      printf("Nie udało się dodać wpisu");printf("Dodano wpis\n");

	  return ;
    }
	fputs("\n",fp);
	fputs(a, fp);
	fputs(" ",fp );
	fclose(fp);
	sem_post(&pdata_queue->main_file);
	printf("Dodano wpis\n");

}
void del_job(char * a){

	FILE *fp1;
	fp1= fopen("/home/asfadfs/Wideo/crone/main_zapas.txt", "w");
	if (fp1 == NULL)
       {
		   printf("Nie udało się usunąć");
		   return ;
	   }
	sem_wait(&pdata_queue->main_file);

	FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
	
    fp = fopen("/home/asfadfs/Wideo/crone/main.txt", "r");
    if (fp == NULL)
       {
		   sem_post(&pdata_queue->main_file);
		   return ;
		}
	char delim[] = " ";


	char *ptr = strtok(a, delim);
	ptr = strtok(NULL, delim);
    while ((read = getline(&line, &len, fp)) != -1) {
        printf("%s", line);
		if(strstr(line, ptr) != NULL) {
			continue;
		}
	fputs(line, fp1);
	fputs(" ",fp1 );
	
    }
	fclose(fp1);
    fclose(fp);
	  if (line)
        free(line);
	remove("main.txt");
	rename("main_zapas.txt","main.txt");

	
	sem_post(&pdata_queue->main_file);
	
}

int main(int argc, char **argv) {
	int id;
	server_status(&id);

	if(id==0) {
			
	int a;
	a = shm_open("nowy",  O_RDWR, 0666);
	if(a==-1) return 1;
	ftruncate(a, sizeof(struct queue));
	
	pdata_queue = (struct queue *)
		mmap(NULL, sizeof(struct queue),
			 PROT_READ | PROT_WRITE, MAP_SHARED,
			 a, 0);
		char * tmp[1024];
		for (int i = 1; i < argc; ++i)
		{
			strncat( tmp, argv[i], 20);
			strncat( tmp, " ", 1);
		}
		
	if(argc<7) {
		if(strstr(tmp, "-del") != NULL) {
				del_job(tmp);
			}
			else display();
		}
	else{
		
			 add_job(tmp);
	}
	close(a);
	}
	else if(id==-1) {
		printf("Nie udało się uruchomić pamięci współdzielonej");
		return -1;
	}
	else {
	printf("Cron uruchomiony\n");
	
		struct sigevent timer_event;
	timer_event.sigev_notify=SIGEV_THREAD;
	timer_event.sigev_notify_function=request_handler;
	timer_event.sigev_value.sival_ptr=NULL;
	timer_event.sigev_notify_attributes=NULL;
	timer_t timer_id;
	timer_create(CLOCK_REALTIME,&timer_event,&timer_id);


	struct itimerspec timer_time;
	timer_time.it_value.tv_sec=1;
	timer_time.it_value.tv_nsec=0;
	timer_time.it_interval.tv_sec=10;
	timer_time.it_interval.tv_nsec=0;
	timer_settime(timer_id,0,&timer_time,NULL);
  	getchar();
			close(id);
			shm_unlink("nowy");
	}

}
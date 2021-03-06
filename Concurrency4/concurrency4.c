/**************************************
 * Alessandro Lim / Kevin Turkington
 * Group 13-07
 * CS444
 * Concurrency 4
 **************************************/
 //Reference: https://adrientetar.github.io/doc/other/Semaphore.pdf
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
//GLOBALS
sem_t no_waiting_chairs;
sem_t barber_chair;
sem_t sleeping_barber;
sem_t begin_haircut;
sem_t identifier_sem;

int identifier = 0;
int no_customers = 0;

int open_chair_max = 4;
const int customer_modifier = 2;

int haircut_duration = 1;
//END GLOBALS

void get_hair_cut(int id)
{
	printf("customer %d: Recieving haircut.\n",id);
	sleep(haircut_duration);
	printf("customer %d: Haircut done.\n",id);
}

void customer(int id)
{
	int open_chairs;

	//is barber sleeping?
	sem_getvalue(&no_waiting_chairs,&open_chairs);
	if(no_customers == 0){
		no_customers++;

		sem_post(&sleeping_barber);
		printf("customer %d: Wakes up barber\n",id);
		sem_wait(&barber_chair);
	}
	else if(open_chairs == 0){
		printf("customer %d: Leaving, Barbershop is full.\n",id);
		return;
	}
	else{
		no_customers++;

		printf("customer %d: waits for haircut\n",id);
		sem_wait(&no_waiting_chairs);
		sem_wait(&barber_chair);
		sem_post(&no_waiting_chairs);
	}

	sem_post(&begin_haircut);
	get_hair_cut(id);
	no_customers--;
}

void *customer_thread(void* data){
	
	while(1){
		//sleep(rand() % (customer_modifier*open_chair_max) +1);
		sleep(rand() %5 + 1);
		
		//allows for unqiue IDS
		sem_wait(&identifier_sem);
		identifier++;
		sem_post(&identifier_sem);

		customer(identifier);
	}
}

void cut_hair()
{
	printf("barber: Cutting customer hair.\n");
	sleep(haircut_duration);
	printf("barber: Done cutting hair.\n");
}

void barber()
{
	int open_chairs;

	sem_getvalue(&no_waiting_chairs,&open_chairs);
	if(open_chairs == open_chair_max && no_customers == 0){
		printf("barber: sleeping...\n");
		sem_wait(&sleeping_barber);
		printf("barber: wokeup.\n");
	}
	sem_post(&barber_chair);
	sem_wait(&begin_haircut);
	cut_hair();
}

void *barber_thread(void* data){
	while(1){
		barber();
	}
}


int main(int argc,char* argv[])
{
	srand(time(NULL));
	if(argc <= 1){
		printf("Note: to specify n chairs run: './concurrency4 n'\n\n");
	}
    else{
        open_chair_max = atoi(argv[1]);
    }

	int i;
	pthread_t barber;
	pthread_t customers[open_chair_max * customer_modifier];

	
	sem_init(&no_waiting_chairs,0,open_chair_max);
	sem_init(&barber_chair,0,0);
	sem_init(&sleeping_barber,0,0);
	sem_init(&begin_haircut,0,0);
	sem_init(&identifier_sem,0,1);

	pthread_create(&barber,NULL,barber_thread,NULL);
	for(i =0;i < open_chair_max * customer_modifier; i++){
		pthread_create(&customers[i], NULL, customer_thread, NULL);
	}

	pthread_join(barber,NULL);
	for(i =0;i < open_chair_max * customer_modifier; i++){
		pthread_join(customers[i],NULL);
	}

	return 0;
}

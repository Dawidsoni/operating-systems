#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#define THREAD_COUNT 7

typedef struct { 
	pthread_mutex_t mutex;
	pthread_mutexattr_t attr; 
	pthread_cond_t cond;
	int counter;		
} sem_t; 

pthread_t thread_arr[THREAD_COUNT];
sem_t tobacco_med, paper_med, matches_med;
sem_t tobacco_ing, paper_ing, matches_ing;
sem_t lock_med, smoke_req;
int has_tobacco, has_paper, has_matches;

void sem_init(sem_t* sem, unsigned val) {
	int mutex_opt = PTHREAD_MUTEX_ERRORCHECK;
	assert(pthread_mutexattr_settype(&sem->attr, mutex_opt) == 0);
	assert(pthread_mutex_init(&sem->mutex, &sem->attr) == 0);
	assert(pthread_cond_init(&sem->cond, NULL) == 0);
	sem->counter = val;
} 

void sem_wait(sem_t* sem) {
	assert(pthread_mutex_lock(&sem->mutex) == 0);
	while(sem->counter == 0) {
		assert(pthread_cond_wait(&sem->cond, &sem->mutex) == 0);
	}	
	sem->counter--;	
	assert(pthread_mutex_unlock(&sem->mutex) == 0);	
}

void sem_post(sem_t* sem) {
	assert(pthread_mutex_lock(&sem->mutex) == 0);
	sem->counter++;
	pthread_cond_broadcast(&sem->cond);
	assert(pthread_mutex_unlock(&sem->mutex) == 0);		
} 

void sem_getvalue(sem_t* sem, int* val) {
	*val = sem->counter;
}

void* agent() {
	while(1) {
		sem_wait(&smoke_req);
		int turn = rand() % 3;
		if(turn == 0) {
			sem_post(&tobacco_med);
			sem_post(&paper_med);			
		}else if(turn == 1) {
			sem_post(&tobacco_med);
			sem_post(&matches_med);						
		}else {
			sem_post(&paper_med);
			sem_post(&matches_med);						
		}			
	}
	return 0;	
}

void* paper_mediator() {
	while(1) {
		sem_wait(&paper_med);
		sem_wait(&lock_med);
		if(has_matches) {
			has_matches = 0;
			sem_post(&tobacco_ing);			
		}else if(has_tobacco) {
			has_tobacco = 0;
			sem_post(&matches_ing);
		}else {
			has_paper = 1;
		}		
		sem_post(&lock_med);			
	}
	return 0;
}

void* tobacco_mediator() {
	while(1) {
		sem_wait(&tobacco_med);
		sem_wait(&lock_med);
		if(has_matches) {
			has_matches = 0;
			sem_post(&paper_ing);			
		}else if(has_paper) {
			has_paper = 0;
			sem_post(&matches_ing);
		}else {
			has_tobacco = 1;
		}		
		sem_post(&lock_med);			
	}
	return 0;	
}

void* matches_mediator() {
	while(1) {
		sem_wait(&matches_med);
		sem_wait(&lock_med);
		if(has_paper) {
			has_paper = 0;
			sem_post(&tobacco_ing);			
		}else if(has_tobacco) {
			has_tobacco = 0;
			sem_post(&paper_ing);
		}else {
			has_matches = 1;
		}		
		sem_post(&lock_med);			
	}	
	return 0;	
}

void* paper_smoker() {
	while(1) {
		sem_wait(&paper_ing);
		printf("Paper smoker is smoking\n");
		sem_post(&smoke_req);
	}
	return 0;	
}

void* tobacco_smoker() {
	while(1) {
		sem_wait(&tobacco_ing);		
		printf("Tobacco smoker is smoking\n");		
		sem_post(&smoke_req);
	}
	return 0;
}

void* matches_smoker() {
	while(1) {
		sem_wait(&matches_ing);		
		printf("Matches smoker is smoking\n");		
		sem_post(&smoke_req);
	}
	return 0;	
}

void init_sem_list() {
	sem_init(&tobacco_med, 0);
	sem_init(&paper_med, 0);
	sem_init(&matches_med, 0);
	sem_init(&tobacco_ing, 0);
	sem_init(&paper_ing, 0);
	sem_init(&matches_ing, 0);
	sem_init(&lock_med, 1);
	sem_init(&smoke_req, 1);
}

void run_simulation() {
	pthread_create(&thread_arr[0], NULL, agent, NULL);
	pthread_create(&thread_arr[1], NULL, paper_mediator, NULL);
	pthread_create(&thread_arr[2], NULL, tobacco_mediator, NULL);
	pthread_create(&thread_arr[3], NULL, matches_mediator, NULL);
	pthread_create(&thread_arr[4], NULL, paper_smoker, NULL);
	pthread_create(&thread_arr[5], NULL, tobacco_smoker, NULL);
	pthread_create(&thread_arr[6], NULL, matches_smoker, NULL);	
	for(int i = 0; i < THREAD_COUNT; i++) {	
		pthread_join(thread_arr[i], NULL);		
	}	
}

int main() {
	srand(time(NULL));
	init_sem_list();		
	run_simulation();
	printf("Exit program\n"); 	
	return 0;
}

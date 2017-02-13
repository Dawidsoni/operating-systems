#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#define PHIL_COUNT 5

typedef struct { 
	pthread_mutex_t mutex;
	pthread_mutexattr_t attr; 
	pthread_cond_t cond;
	int counter;		
} sem_t; 

pthread_t thread_arr[PHIL_COUNT];
sem_t fork_arr[PHIL_COUNT];
sem_t sem_waiter;

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

void ignore_int_sig() {
	sigset_t ignore_set;
	sigemptyset(&ignore_set);
	sigaddset(&ignore_set, SIGINT);   
	pthread_sigmask(SIG_BLOCK, &ignore_set, NULL);
}

void set_cancel_type() {
	int old_state;
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &old_state);
}

void random_sleep() {
	int rand_secs = rand() % 100;
	usleep(rand_secs);	
}

void phil_think() {
	random_sleep();
}

void phil_eat(int num) {
	random_sleep();
	printf("Philosopher %d is eating\n", num);
}

int next_fork(int num) {
	return (num + 1) % PHIL_COUNT;
}

void take_forks(int num) {
	int next_num = next_fork(num);
	sem_wait(&sem_waiter);
	sem_wait(&fork_arr[num]);
	sem_wait(&fork_arr[next_num]);
}

void put_forks(int num) {
	int next_num = next_fork(num);
	sem_post(&fork_arr[num]);
	sem_post(&fork_arr[next_num]);
	sem_post(&sem_waiter);
}

void* phil(void* num_ptr) {
	ignore_int_sig();		
	set_cancel_type();
	int num = *(int*)num_ptr;
	while(1) {
		phil_think();
		take_forks(num);
		phil_eat(num);
		put_forks(num);
	}	
	return 0;
}

void sig_handler() {
	printf("Signal handler\n"); 
	for(int i = 0; i < PHIL_COUNT; i++) {
		pthread_cancel(thread_arr[i]);
	}
}

void set_handler(void (*handler_ptr)()) {
	struct sigaction action;
	action.sa_handler = handler_ptr;
	sigaction(SIGINT, &action, NULL);		
}

void init_sem_list() {
	for(int i = 0; i < PHIL_COUNT; i++) {
		sem_init(&fork_arr[i], 1);
	}	
	sem_init(&sem_waiter, PHIL_COUNT - 1);
}

void run_simulation() {
	int num_arr[PHIL_COUNT];	
	for(int i = 0; i < PHIL_COUNT; i++) {
		num_arr[i] = i;		
		pthread_create(&thread_arr[i], NULL, phil, &num_arr[i]);
	}	
	for(int i = 0; i < PHIL_COUNT; i++) {	
		pthread_join(thread_arr[i], NULL);		
	}			
}

int main() {
	srand(time(NULL));
	set_handler(sig_handler);	
	init_sem_list();	
	run_simulation();
	printf("Exit program\n"); 
	return 0;
}

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#define UNUSED(x) (void)(x)

typedef struct { 
	pthread_mutex_t mutex;
	pthread_mutexattr_t attr; 
	pthread_cond_t cond;
	int counter;		
} sem_t; 

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

int counter;
sem_t sem;
const int LOOP_SIZE = 100000;
const int THREAD_COUNT = 10;

void* increment(void* argv) {	
	UNUSED(argv);
	sem_wait(&sem);
	for(int i = 0; i < LOOP_SIZE; i++) {
		counter++;
	}
	sem_post(&sem);	
	return 0;
}

int main() {
	sem_init(&sem, 1);
	pthread_t thread_arr[THREAD_COUNT];
	for(int i = 0; i < THREAD_COUNT; i++) {
		pthread_create(&thread_arr[i], NULL, &increment, NULL);
	}
	for(int i = 0; i < THREAD_COUNT; i++) {
		pthread_join(thread_arr[i], NULL);
	}
	int sem_counter;
	sem_getvalue(&sem, &sem_counter);
	assert(sem_counter == 1);
	assert(counter == LOOP_SIZE * THREAD_COUNT);
	printf("Counter: %d\n", counter);
	return 0;
}

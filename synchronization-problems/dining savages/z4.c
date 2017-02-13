#include <semaphore.h>
#include <sys/stat.h>  
#include <fcntl.h> 
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#define SAVAGE_COUNT 10
#define COULDRON_SIZE 3
#define SEM_PERM (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define SEM_COOK_NAME "/savage_sem_cook"
#define SEM_COULDRON_NAME "/savage_sem_queue"
#define SEM_LOCK_NAME "/savage_sem_lock"

static int* stew_count;
int pid_list[SAVAGE_COUNT + 1];

void random_sleep() {
	int rand_secs = rand() % 100;
	usleep(rand_secs);	
}

void savage_eat(int num) {
	printf("PID: %d, savage %d is eating\n", getpid(), num);	
}

void savage_sleep() {
	random_sleep();
}

sem_t* get_sem_couldron() {
	return sem_open(SEM_COULDRON_NAME, O_APPEND);		
}

sem_t* get_sem_lock() {
	return sem_open(SEM_LOCK_NAME, O_APPEND);		
}

sem_t* get_sem_cook() {
	return sem_open(SEM_COOK_NAME, O_APPEND);	
}

void savage(int num) {
	sem_t* sem_cook = get_sem_cook();
	sem_t* sem_lock = get_sem_lock();
	sem_t* sem_couldron = get_sem_couldron();
	while(1) {
		sem_wait(sem_lock);		
		if(*stew_count == 0) {
			sem_post(sem_cook);
			sem_wait(sem_couldron);
		}
		savage_eat(num);
		(*stew_count)--;
		sem_post(sem_lock);		
		savage_sleep();		
	}
}

void cook() {
	sem_t* sem_cook = get_sem_cook();
	sem_t* sem_couldron = get_sem_couldron();
	while(1) {
		sem_wait(sem_cook);
		printf("Cooking stew\n");
		*stew_count = COULDRON_SIZE;
		sem_post(sem_couldron);
	}
}

void init_sem_list() {
	sem_open(SEM_COULDRON_NAME, O_CREAT | O_EXCL, SEM_PERM, 0);
	sem_open(SEM_LOCK_NAME, O_CREAT | O_EXCL, SEM_PERM, 1);
	sem_open(SEM_COOK_NAME, O_CREAT | O_EXCL, SEM_PERM, 0);
}

void init_mapping() {
	int fd = PROT_READ | PROT_WRITE;
	int flags = MAP_SHARED | MAP_ANONYMOUS;
	stew_count = mmap(NULL, sizeof(*stew_count), fd, flags, -1, 0);	
	(*stew_count) = COULDRON_SIZE;
}

void destroy_sem_list() {
	sem_unlink(SEM_COULDRON_NAME);
	sem_unlink(SEM_LOCK_NAME);
	sem_unlink(SEM_COOK_NAME);
}

void destroy_mapping() {
	munmap(stew_count, sizeof(*stew_count));
}

void kill_children() {
	for(int i = 0; i < SAVAGE_COUNT + 1; i++) {
		kill(pid_list[i], SIGKILL);	
	}	
}

void sig_handler() {
	printf("Signal handler\n");
	destroy_sem_list();
	destroy_mapping();
	kill_children();
}

void set_handler(void (*handler_ptr)()) {
	struct sigaction action;
	action.sa_handler = handler_ptr;
	sigaction(SIGINT, &action, NULL);		
}

void run_simulation() {
	int pid = fork();
	if(pid == 0) {
		cook();
		exit(EXIT_SUCCESS);
	}
	pid_list[0] = pid;
	for(int i = 0; i < SAVAGE_COUNT; i++) {
		int pid = fork();
		if(pid == 0) {
			savage(i);
			exit(EXIT_SUCCESS);
		}		
		pid_list[i + 1] = pid;
	}	
	set_handler(sig_handler);	
	for(int i = 0; i < SAVAGE_COUNT + 1; i++) {
		int status;
		waitpid(pid_list[i], &status, 0);
	}
}

int main() {
	srand(time(NULL));
	init_sem_list();
	init_mapping();
	run_simulation();
	destroy_sem_list();
	destroy_mapping();
	return 0;
}

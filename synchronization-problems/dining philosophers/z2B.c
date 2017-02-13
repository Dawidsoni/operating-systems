#include <semaphore.h>
#include <sys/stat.h>  
#include <fcntl.h> 
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#define PHIL_COUNT 5
#define SEM_PERM (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

int pid_list[PHIL_COUNT];

void sem_fork_name(int num, char* ptr) {
	sprintf(ptr, "/philosopher_fork_%d", num);
}

void sem_waiter_name(char* ptr) {
	sprintf(ptr, "/philosophers_waiter");	
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
	printf("PID: %d, philosopher %d is eating\n", getpid(), num);
}

int next_num(int num) {
	return (num + 1) % PHIL_COUNT;
}

sem_t* get_sem_fork(int num) {
	char sem_name[30];
	sem_fork_name(num, sem_name);
	return sem_open(sem_name, O_APPEND);
}

sem_t* get_sem_waiter() {
	char sem_name[30];
	sem_waiter_name(sem_name);
	return sem_open(sem_name, O_APPEND);	
}

void take_forks(sem_t* waiter, sem_t* l_fork, sem_t* r_fork) {
	sem_wait(waiter);
	sem_wait(l_fork);
	sem_wait(r_fork);		
}

void put_forks(sem_t* waiter, sem_t* l_fork, sem_t* r_fork) {
	sem_post(l_fork);
	sem_post(r_fork);
	sem_post(waiter);
}

void phil(int num) {
	sem_t* waiter = get_sem_waiter();
	sem_t* l_fork = get_sem_fork(num);
	sem_t* r_fork = get_sem_fork(next_num(num));
	while(1) {
		phil_think();
		take_forks(waiter, l_fork, r_fork);
		phil_eat(num);
		put_forks(waiter, l_fork, r_fork);
	}	
}

void init_sem_list() {
	char sem_name[30];
	for(int i = 0; i < PHIL_COUNT; i++) {
		sem_fork_name(i, sem_name);
		sem_open(sem_name, O_CREAT | O_EXCL, SEM_PERM, 1);	
	}		
	sem_waiter_name(sem_name);
	sem_open(sem_name, O_CREAT | O_EXCL, SEM_PERM, PHIL_COUNT - 1);	
}

void destroy_sem_list() {
	char sem_name[30];
	for(int i = 0; i < PHIL_COUNT; i++) {
		sem_fork_name(i, sem_name);
		sem_unlink(sem_name);
	}	
	sem_waiter_name(sem_name);
	sem_unlink(sem_name);		
}

void kill_children() {
	for(int i = 0; i < PHIL_COUNT; i++) {
		kill(pid_list[i], SIGKILL);	
	}	
}

void sig_handler() {
	printf("Signal handler\n");
	kill_children();
	destroy_sem_list();
}

void set_handler(void (*handler_ptr)()) {
	struct sigaction action;
	action.sa_handler = handler_ptr;
	sigaction(SIGINT, &action, NULL);		
}

void run_simulation() {
	for(int i = 0; i < PHIL_COUNT; i++) {
		int pid = fork();
		if(pid == 0) {
			phil(i);
			exit(EXIT_SUCCESS);
		}		
		pid_list[i] = pid;
	}	
	set_handler(sig_handler);	
	for(int i = 0; i < PHIL_COUNT; i++) {
		int status;
		waitpid(pid_list[i], &status, 0);
	}
}

int main() {
	srand(time(NULL));
	init_sem_list();
	run_simulation();
	destroy_sem_list();
	printf("Exit program\n"); 
	return 0;
}

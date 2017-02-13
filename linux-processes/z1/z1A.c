#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>

int child_pid;

void show_child() {
	char str[10];
	sprintf(str, "%d", child_pid);
	char* argv[] = {"/bin/ps", "-p", str, "-o", "pid,ppid,comm", 0};
	execv("/bin/ps", argv);			
}

void prog1() {
	child_pid = fork();
	if (child_pid != 0) {
		int ps_pid = fork();
		if(ps_pid == 0) {
			show_child();			
		}else {
			int status;
			waitpid(ps_pid, &status, 0);
		}
	}else {
		exit(EXIT_SUCCESS);
	}	
}

void sig_handler(int signal) {
	printf("Signal: %d\n", signal);
	show_child();
}

void prog2() {
	struct sigaction action;
	action.sa_handler = sig_handler;
	action.sa_flags = SA_NOCLDWAIT;
	sigaction(SIGCHLD, &action, NULL);	
	child_pid = fork();
	if (child_pid != 0) {
		pause();
	}else {
		exit(EXIT_SUCCESS);
	}
}

int main(int argc, char* argv[]) {
	if(argc < 2) {
		printf("Error: too few arguments\n"); 
	}else if(strcmp(argv[1], "-a") == 0) {
		prog1();
	}else if(strcmp(argv[1], "-b") == 0) {
		prog2();		
	}else {
		printf("Error: invalid argument\n"); 		
	}	
	return 0;
}

#include <stdio.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <signal.h>

int main() {
	int reaper_pid;
	int parent_pid = getpid();
	printf("PID: %d\n", parent_pid);
	prctl(PR_GET_CHILD_SUBREAPER, &reaper_pid);
	printf("Child subreaper: %d\n", reaper_pid);
	int child_pid = fork();
	if(child_pid == 0) {
		while(kill(parent_pid, SIGKILL) != -1) {}
		char* argv[] = {"/bin/ps", "-o", "pid,ppid,comm", 0};
		execv("/bin/ps", argv);					
	}
	return 0;
}

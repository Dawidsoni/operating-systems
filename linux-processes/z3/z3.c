#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

void func1(int pipe1[2], int pipe2[2]) {
	close(pipe2[1]);   
	int is_end = 1;
	int word_count = 0;
	char arr[100];
	while(1) {
		is_end = scanf("%s", arr);
		if(is_end == -1) {
			close(pipe1[1]);
			break;
		}
		write(pipe1[1], arr, strlen(arr));
		write(pipe1[1], " ", 1);
		word_count += 1;		
	} 
	printf("Word count: %d\n", word_count);				
}

void func2(int pipe1[2], int pipe2[2]) {
	close(pipe1[1]);
	char buf;
	int index = 0;
	char arr[100];
	while(read(pipe1[0], &buf, 1) > 0) {
		if(buf == ' ') {
			write(pipe2[1], arr, index);
			write(pipe2[1], " ", 1);			
			index = 0;
		}else if(isalnum(buf)) {
			arr[index] = buf;
			index += 1;
		}
	}
	close(pipe2[1]);	   
}

void func3(int pipe1[2], int pipe2[2]) {
	close(pipe1[1]);   
	close(pipe2[1]);
	int char_count = 0;
	char buf;
	while(read(pipe2[0], &buf, 1) > 0) {
		if(buf == ' ') {
			printf("\n");		
		}else if(isalnum(buf)) {
			char_count += 1;
			printf("%c", buf);		
		}
	}		
	printf("Char count: %d\n", char_count);	
}

void close_pipes(int pipe1[2], int pipe2[2]) {
	close(pipe1[1]);   
	close(pipe2[1]);   	
}

void wait_for_children() {
	int status;
	for(int i = 0; i < 3; i++) {
		int pid = wait(&status);
		printf("Process %d terminated\n", pid);
	}
}

int main() {
	int pipe1[2], pipe2[2];
	pipe(pipe1);
	pipe(pipe2);
	int child1 = fork();
	if(child1 == 0) {
		func1(pipe1, pipe2);
		return 0;
	}
	int child2 = fork();
	if(child2 == 0) {
		func2(pipe1, pipe2);
		return 0;
	}
	int child3 = fork();
	if(child3 == 0) {
		func3(pipe1, pipe2);
		return 0;
	}	
	close_pipes(pipe1, pipe2);
	wait_for_children();	
	printf("Terminating parent process\n");
	return 0;
}

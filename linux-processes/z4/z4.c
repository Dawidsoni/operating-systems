#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
#include <ucontext.h>

const int ERR_FD = 2;

void print_err_signal(int signal) {
	char str_addr[100];
	sprintf(str_addr, "Error signal: %d\n", signal);
	write(ERR_FD, str_addr, strlen(str_addr));		
}

void print_err_addr(siginfo_t* info) {
	char str_addr[100];
	sprintf(str_addr, "Error address: %p\n", info->si_addr);
	write(ERR_FD, str_addr, strlen(str_addr));	
}

void print_err_type(siginfo_t* info) {	
	if(info->si_code == SEGV_MAPERR) {
		char str_type[100] = "Signal type: address not mapped to object\n";
		write(ERR_FD, str_type, strlen(str_type));				
	}else if(info->si_code == SEGV_ACCERR) {	
		char str_type[100] = "Signal type: invalid permissions for mapped object\n";	
		write(ERR_FD, str_type, strlen(str_type));				
	}else {
		char str_type[100];
		sprintf(str_type, "Signal type: %d", info->si_code);
		write(ERR_FD, str_type, strlen(str_type));						
	}
}

void print_err_stack(ucontext_t* ctx) {
	char str_addr[60];
	unsigned char* rip = (unsigned char*)ctx->uc_mcontext.gregs[REG_RIP];
	unsigned char* rsp = (unsigned char*)ctx->uc_mcontext.gregs[REG_RSP];		
	sprintf(str_addr, "Stack address: %p\nFunction address: %p\n", rip, rsp);
	write(ERR_FD, str_addr, strlen(str_addr));	
}

void print_err_backtrace() {
	const int BUF_SIZE = 20;
	char b_info[] = "Backtrace: \n";
	void* buf[BUF_SIZE];
	write(ERR_FD, b_info, strlen(b_info));
	backtrace(buf, BUF_SIZE);
	backtrace_symbols_fd(buf, BUF_SIZE, ERR_FD);	
}

void sig_action(int signal, siginfo_t* info, void* ctx_pointer) {
	ucontext_t* ctx = ctx_pointer;
	print_err_signal(signal);
	print_err_addr(info);
	print_err_type(info);
	print_err_stack(ctx);	
	print_err_backtrace();
	exit(EXIT_FAILURE);
}

void set_handler() {
	struct sigaction action;
	action.sa_sigaction = sig_action;
	action.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &action, NULL);			
}

int main(int argc, char* argv[]) {
	set_handler();
	if(argc < 2) {
		printf("Error: too few arguments\n"); 
	}else if(strcmp(argv[1], "-a") == 0) {
		char* text = NULL;
		puts(text);
	}else if(strcmp(argv[1], "-b") == 0) {
		void* func = set_handler;
		*(int*)(func) = 1;		
	}else {
		printf("Error: invalid argument\n"); 		
	}	
	return 0;
}

#include <stdio.h>
#include <ucontext.h>
#include <ctype.h>

ucontext_t ctx_main, ctx_func1, ctx_func2, ctx_func3;
char func1_stack[16384];
char func2_stack[16384];
char func3_stack[16384];

char arr[100];
int is_end = 1;

void func1() {
	int word_count = 0;
	while(1) {
		is_end = scanf("%s", arr);
		if(is_end == -1) {
			break;
		}
		word_count += 1;
		swapcontext(&ctx_func1, &ctx_func2);		
	}
	printf("Word count: %d\n", word_count);			
}

void func2() {
	while(is_end != -1) {
		int index = 0;
		for(int i = 0; i < 100; i++) {
			if(isalnum(arr[i])) {
				arr[index] = arr[i];
				index++;
			}
			if(arr[i] == 0) {
				break;
			}
		}
		arr[index] = 0;
		swapcontext(&ctx_func2, &ctx_func3);		
	}
}

void func3() {
	int char_count = 0;
	while(is_end != -1) {
		printf("%s\n", arr);
		for(int i = 0; i < 100; i++) {
			if(arr[i] == 0) {
				break;
			}			
			char_count++;
		}
		swapcontext(&ctx_func3, &ctx_func1);			 				
	}
	printf("Char count: %d\n", char_count);
}

void initcontext(ucontext_t* ctx, char* stack, void (*func)(), ucontext_t* link) {
   getcontext(ctx);
   ctx->uc_stack.ss_sp = stack;
   ctx->uc_stack.ss_size = 16384;
   ctx->uc_link = link;
   makecontext(ctx, func, 0);	
}

int main() {
	initcontext(&ctx_func1, func1_stack, func1, &ctx_func2);
	initcontext(&ctx_func2, func2_stack, func2, &ctx_func3);
	initcontext(&ctx_func3, func3_stack, func3, NULL);
	swapcontext(&ctx_main, &ctx_func1);
	return 0;
}

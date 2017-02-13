#include "common.h"

void fill(int *dst, int n) {
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			dst[i * n + j] = i * n + j;
}

void transpose1(int *dst, int *src, int n) {
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			dst[j * n + i] = src[i * n + j];
}

void transpose2(int *dst, int *src, int n) {
	int BLOCK_LENGTH = 8;
	if(n < BLOCK_LENGTH) {
		BLOCK_LENGTH = n;
	}
	for(int i = 0; i < n; i += BLOCK_LENGTH) {
		for(int j = 0; j < n; j += BLOCK_LENGTH) {
			for(int k = i; k < i + BLOCK_LENGTH && k < n; k++) {
				for(int l = j; l < j + BLOCK_LENGTH && l < n; l++) {
					dst[l * n + k] = src[k * n + l];
				}
			}
		}		
	}
}

/* this procedure was used to measure which BLOCK_LENGTH gives best results */
void testTranspose(int *dst, int *src, int n, int blockLength) {
	int m = n;
	m -= (m % blockLength);
	for(int i = 0; i < m; i += blockLength) {
		for(int j = 0; j < m; j += blockLength) {
			for(int k = i; k < i + blockLength; k++) {
				for(int l = j; l < j + blockLength; l++) {
					dst[l * n + k] = src[k * n + l];
				}
			}
		}		
	}
	for(int i = m; i < n; i++) {
		for(int j = m; j < n; j++) {
			dst[j * n + i] = src[i * n + j];			
		}
	}		
}

void testBlockLength(int *dst, int* src, int n, int size, int length) {
	_timer_t timer;
	timer_reset(&timer);
	timer_start(&timer);
	for(int i = 0; i < 20; i++) {
		bzero(dst, size);
		flush_cache();
		testTranspose(dst, src, n, length);
	}
	timer_stop(&timer);
	timer_print(&timer);	
}

void testBlockLengths(int *dst, int* src, int n, int size) {
	for(int i = 1; i <= 24; i++) {
		printf("Block length = %d \n", i);
		testBlockLength(dst, src, n, size, i);
	}
}

int main(int argc, char **argv) {
	int opt, exp = -1, var = -1;
	bool err = false;
	while ((opt = getopt(argc, argv, "n:v:")) != -1) {
		if (opt == 'n')
		  exp = atoi(optarg);
		else if (opt == 'v')
		  var = atoi(optarg);
		else
		  err = true;
	}
	if (err || exp < 0 || var < 0 || var >= 2) {
		fprintf(stderr, "Usage: %s -n log2(size) -v variant\n", argv[0]);
		return 1;
	}
	int n = 1 << exp;
	int size = n * n * sizeof(int);
	int *src = NULL, *dst = NULL;
	posix_memalign((void **)&src, getpagesize(), size);
	posix_memalign((void **)&dst, getpagesize(), size);
	printf("Generate matrix %d x %d (%d KiB)\n", n, n, size >> 10);
	fill(src, n);
	bzero(dst, size);
	flush_cache();
	printf("Performing matrix transposition.\n");
	_timer_t timer;
	timer_reset(&timer);
	timer_start(&timer);
	if (var == 0) 
		transpose1(dst, src, n);
	else
		transpose2(dst, src, n);
	timer_stop(&timer);
	timer_print(&timer);
	free(src);
	free(dst);
	return 0;
}

#include "common.h"
#define DEBUG 0

int array_walk(volatile int *array, int steps) {
	int sum = 0, i = 0;
	do {
		if (i < 0)
			break;
		#if DEBUG
			printf("%d -> %d\n", i, array[i]);
		#endif
		i = array[i];
		sum += i;
	}while (--steps);
	return sum;
}



void generate_permutation(int *array, int size) {
	const int LINE_SIZE = 16;
	for (int i = 0; i < size; i += LINE_SIZE) {
		if(i + LINE_SIZE < size) {
			array[i] = i + LINE_SIZE;			
		}else {
			array[i] = -1;
		}
	}
}

/*
Cache line size

Processor fetch data in chunks (cache lines) so accessing data in the same cache line is cheap.
If we omit some cache lines, execution time is much better.

Tests:

LINE_SIZE = 4 
time: about 0.50s.
 
LINE_SIZE = 8
time: about 0.45s.
 
LINE_SIZE = 16
time: about 0.47s.
 
LINE_SIZE = 32
time: about 0.36s.
 
LINE_SIZE = 64
time: about 0.26s.

LINE_SIZE = 128
time: about 0.20s.

Conclusion: similar times between 4 and 16 LINE_SIZE so values are in the same cache line
Cache line size: 16 * sizeof(int) = 64 bytes
*/

/*
L1, L2, L3 sizes

When our array size exceeds cache size, some data have to be loaded from the memory of higher level.
For instance, if our array size exceeds L1 cache size, some data have to be loaded from L2 cache.
So we should see performance drops if we exceed current cache level.
 
L1 size tests:

./cache -n 12 -s 26 -t 1000000
time: about 0.70s.

./cache -n 13 -s 26 -t 1000000
time: about 1.40s.

./cache -n 14 -s 26 -t 1000000
time: about 5.10s

./cache -n 15 -s 26 -t 1000000
time: about 10.10s.

Conclusion: drop between 32 KiB and 64 KiB so L1 size is 32 KiB

L2 size tests:

./cache -n 14 -s 26 -t 50000
time: about 0.27s.

./cache -n 14 -s 26 -t 50000
time: about 0.50s.

./cache -n 14 -s 26 -t 50000
time: about 1.10s

./cache -n 14 -s 26 -t 50000
time: about 2.15s.

Conclusion: drop between 256 KiB and 512 KiB so L2 size is 256 KiB

L3 size test:

./cache -n 19 -s 26 -t 10000
time: about 2s.

./cache -n 20 -s 26 -t 10000
time: about 4s.

./cache -n 21 -s 26 -t 10000
time: about 13s.

./cache -n 22 -s 26 -t 10000
time: about 25s.

Conclusion: drop between 4 MB and 8 MB so L3 size is in range 4-8 MB (my L3 size is 6 MB)
*/

int main(int argc, char **argv) {
	int opt, size = -1, steps = -1, times = -1;
	bool error = false;
	while ((opt = getopt(argc, argv, "n:s:t:")) != -1) {
		if (opt == 'n') 
		  size = 1 << atoi(optarg);
		else if (opt == 's')
		  steps = 1 << atoi(optarg);
		else if (opt == 't')
		  times = atoi(optarg);
		else
		  error = true;
	}
	if (error || size < 0 || steps < 0 || times < 0) {
		printf("Usage: %s -n log2(size) -s log2(steps) -t times\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	int *array = NULL;
	if (posix_memalign((void **)&array, getpagesize(), size * sizeof(int)) != 0)
		fail("Failed to allocate memory!");
	printf("Generate array of %d elements (%ld KiB)\n", size, size * sizeof(int) >> 10);
	generate_permutation(array, size);
	flush_cache();
	printf("Perfom walk %d times with %d steps each.\n", times, steps);
	_timer_t t;
	timer_reset(&t);
	for (int i = 0; i < times; i++) {
		timer_start(&t);
		array_walk(array, steps);
		timer_stop(&t);
	}
	timer_print(&t);
	free(array);
	return EXIT_SUCCESS;
}

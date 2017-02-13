CC	= gcc -g -pthread
CFLAGS	= -c -std=gnu99 -Wall -Wextra

BINS = z2A z2B

all: $(BINS)

z2A: z2A.o
z2A.o: z2A.c

z2B: z2B.o
z2B.o: z2B.c

clean:
	@rm -vf *.o $(BINS)
	

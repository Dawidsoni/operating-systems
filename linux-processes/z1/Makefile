CC	= gcc -g
CFLAGS	= -c -std=gnu99 -Wall -Wextra

BINS = z1A z1B

all: $(BINS)

z1A: z1A.o
z1A.o: z1A.c

z1B: z1B.o
z1B.o: z1B.c

clean:
	@rm -vf *.o $(BINS)
	

BINS = z1

z1: z1.o
	gcc -pthread -o z1 z1.o
	
z1.o: z1.c
	gcc -c -std=gnu99 -Wall -Wextra z1.c
	
clean:
	@rm -vf *.o $(BINS)
	

BINS = z5

z5: z5.o
	gcc -pthread -o z5 z5.o
	
z5.o: z5.c
	gcc -c -std=gnu99 -Wall -Wextra z5.c
	
clean:
	@rm -vf *.o $(BINS)
	

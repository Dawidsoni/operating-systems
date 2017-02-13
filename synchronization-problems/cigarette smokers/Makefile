BINS = z6

z6: z6.o
	gcc -pthread -o z6 z6.o
	
z6.o: z6.c
	gcc -c -std=gnu99 -Wall -Wextra z6.c
	
clean:
	@rm -vf *.o $(BINS)
	


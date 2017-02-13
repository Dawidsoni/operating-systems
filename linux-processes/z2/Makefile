BINS = z2 

z2: z2.o
	gcc -o z2 z2.o
	
z2.o: z2.c
	gcc -c -std=gnu99 -Wall -Wextra z2.c
	
clean:
	@rm -vf *.o $(BINS)
	

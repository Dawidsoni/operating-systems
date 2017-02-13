BINS = z4

z4: z4.o
	gcc -o z4 z4.o
	
z4.o: z4.c
	gcc -c -std=gnu99 -Wall -Wextra z4.c
	
clean:
	@rm -vf *.o $(BINS)
	

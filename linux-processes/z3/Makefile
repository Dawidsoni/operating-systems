BINS = z3

z3: z3.o
	gcc -o z3 z3.o
	
z3.o: z3.c
	gcc -c -std=gnu99 -Wall -Wextra z3.c
	
clean:
	@rm -vf *.o $(BINS)
	

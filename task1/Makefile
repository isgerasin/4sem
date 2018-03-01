CC=gcc
CFLAGS=-fprofile-arcs -ftest-coverage

a.out: test.o list.o
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

list.o: list.h

clean:
	rm *.o *.out

gcovtest:
	./a.out
	gcov -b ./list.c
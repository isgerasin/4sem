CC=gcc
CFLAGS=-fprofile-arcs -ftest-coverage

a.out: test.o list.o
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c list.h
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm *.o *.out

gcovtest:
	./a.out
	gcov -b ./list.c
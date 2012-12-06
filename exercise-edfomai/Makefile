CFLAGS += $(shell xeno-config --skin=native --cflags)
LDFLAGS += $(shell xeno-config --skin=native --ldflags)
LDFLAGS += -lrtdk

all: mars_rover example1

mars_rover: mars_rover.o
	$(CC) mars_rover.o $(LDFLAGS) -o mars_rover
mars_rover.o: mars_rover.c
	$(CC) -c $(CFLAGS) mars_rover.c

example1: example1.o
	$(CC) example1.o $(LDFLAGS) -o example1
example1.o: example1.c
	$(CC) -c $(CFLAGS) example1.c

clean:
	rm -f *.o mars_rover example1 test test_b


CFLAGS += $(shell xeno-config --skin=native --cflags)
LDFLAGS += $(shell xeno-config --skin=native --ldflags)
LDFLAGS += -lrtdk

all: multitask semaphore test test_b

multitask: multitask.o
	$(CC) multitask.o $(LDFLAGS) -o multitask
multitask.o: multitask.c
	$(CC) -c $(CFLAGS) multitask.c

semaphore: semaphore.o
	$(CC) semaphore.o $(LDFLAGS) -o semaphore
semaphore.o: semaphore.c
	$(CC) -c $(CFLAGS) semaphore.c

test: test.o
	$(CC) test.o $(LDFLAGS) -o test
test.o: test.c
	$(CC) -c $(CFLAGS) test.c

test_b: test_b.o
	$(CC) test_b.o $(LDFLAGS) -o test_b
test_b.o: test_b.c
	$(CC) -c $(CFLAGS) test_b.c

clean:
	rm -f *.o multitask semaphore test test_b


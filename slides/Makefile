CFLAGS += $(shell xeno-config --skin=native --cflags)
LDFLAGS += $(shell xeno-config --skin=native --ldflags)
LDFLAGS += -lrtdk

all: xeno-1shot xeno-3periodics

xeno-1shot: xeno-1shot.o
	$(CC) xeno-1shot.o $(LDFLAGS) -o xeno-1shot
xeno-1shot.o: xeno-1shot.c
	$(CC) -c $(CFLAGS) xeno-1shot.c

xeno-3periodics: xeno-3periodics.o
	$(CC) xeno-3periodics.o $(LDFLAGS) -o xeno-3periodics
xeno-3periodics.o: xeno-3periodics.c
	$(CC) -c $(CFLAGS) xeno-3periodics.c

clean:
	rm -f *.o xeno-1shot xeno-3periodics


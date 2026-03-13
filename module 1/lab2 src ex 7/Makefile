CC = gcc
CFLAGS = -O0 -Wall
LDFLAGS = -lpthread

ALL = coordinator producer worker

all: $(ALL)

coordinator: coordinator.c common.h
	$(CC) $(CFLAGS) -o coordinator coordinator.c $(LDFLAGS)

producer: producer.c common.h
	$(CC) $(CFLAGS) -o producer producer.c

worker: worker.c common.h
	$(CC) $(CFLAGS) -o worker worker.c

clean:
	rm -f $(ALL)

.PHONY: all clean

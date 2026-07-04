CC      = gcc
CFLAGS  = -Wall -Wextra -O2

TARGETS = udp_coordinator udp_producer udp_worker

all: $(TARGETS)

udp_coordinator: udp_coordinator.c udp_common.h
	$(CC) $(CFLAGS) -o $@ $<

udp_producer: udp_producer.c udp_common.h
	$(CC) $(CFLAGS) -o $@ $<

udp_worker: udp_worker.c udp_common.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean

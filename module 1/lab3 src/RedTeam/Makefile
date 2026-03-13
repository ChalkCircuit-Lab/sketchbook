CC = gcc

# -fno-stack-protector : disable stack canaries so overflow corrupts the return address
# -D_FORTIFY_SOURCE=0  : disable glibc's hardened strcpy/memcpy that abort on overflow
# -z execstack         : mark stack as executable (standard for overflow demos)
# -no-pie              : disable position-independent executable
# -O0                  : no optimizations, preserve vulnerable stack layout
# CFLAGS = -O0 -fno-stack-protector -D_FORTIFY_SOURCE=0 -z execstack -no-pie
LDFLAGS = -lpthread

all: coord_naive

coord_naive: coordinator_naive.c common.h
	$(CC) $(CFLAGS) -o coord_naive coordinator_naive.c $(LDFLAGS)

clean:
	rm -f coord_naive

.PHONY: all clean

CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=c11 -D_DEFAULT_SOURCE
LDFLAGS = -lssl -lcrypto

TARGETS = client server

# ── build ─────────────────────────────────────────────────────────────────────
all: certs $(TARGETS)

client: client.c common.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

server: server.c common.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# ── TLS certificate for QUIC (self-signed, generated once) ───────────────────
certs: server.cert server.key

server.cert server.key:
	@echo "Generating self-signed TLS certificate for QUIC..."
	openssl req -nodes -new -x509 \
	    -keyout server.key -out server.cert \
	    -subj "/CN=localhost" -days 365 2>/dev/null
	@echo "Done: server.cert / server.key"

# ── clean ─────────────────────────────────────────────────────────────────────
clean:
	rm -f $(TARGETS)

distclean: clean
	rm -f server.cert server.key

# ── quick-run helpers ─────────────────────────────────────────────────────────
# Open two terminals (or two tmux panes) and run the server first, then client.
#
# TCP – streaming, 500 MB, 1 000-byte blocks:
#   ./server -p tcp
#   ./client -p tcp -b 1000 -d 500
#
# TCP – streaming, 1 GB, 65 535-byte blocks:
#   ./server -p tcp
#   ./client -p tcp -b 65535 -d 1024
#
# UDP – streaming, 500 MB, 10 000-byte blocks:
#   ./server -p udp
#   ./client -p udp -b 10000 -d 500
#
# UDP – stop-and-wait, 500 MB, 1 000-byte blocks:
#   ./server -p udp
#   ./client -p udp -b 1000 -d 500 -m stop-wait
#
# QUIC – streaming, 500 MB, 10 000-byte blocks:
#   ./server -p quic
#   ./client -p quic -b 10000 -d 500
#
# QUIC – stop-and-wait, 500 MB, 1 000-byte blocks:
#   ./server -p quic
#   ./client -p quic -b 1000 -d 500 -m stop-wait
#
# Suggested test matrix  (3 protocols × 2 modes × 5 block sizes × 2 data sizes)
#   Block sizes : 1  500  1000  10000  65535  bytes
#   Data sizes  : 500 MB (512)  and  1 GB (1024)
#   Modes       : streaming  stop-wait  (stop-wait optional for TCP)
#
# NOTE: Stop-and-wait for UDP and QUIC is mandatory per the assignment spec.
#       Stop-and-wait for TCP is optional (TCP already ACKs internally).

.PHONY: all clean distclean certs

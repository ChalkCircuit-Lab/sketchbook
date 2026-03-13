CC = gcc
CFLAGS = -O0 -Wall -Iinclude
LDFLAGS = -l:libmsquic.so.2 -lpthread

MSQUIC_VER = v2.5.6
MSQUIC_RAW = https://raw.githubusercontent.com/microsoft/msquic/$(MSQUIC_VER)/src/inc

ALL = quic_coordinator quic_producer quic_worker

all: headers certs $(ALL)

quic_coordinator: quic_coordinator.c quic_common.h include/msquic.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

quic_producer: quic_producer.c quic_common.h include/msquic.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

quic_worker: quic_worker.c quic_common.h include/msquic.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Download msquic development headers (the libmsquic apt package only has the .so)
headers: include/msquic.h

include/msquic.h:
	@echo ">>> Downloading msquic headers ($(MSQUIC_VER))..."
	@mkdir -p include
	wget -q -O include/msquic.h        $(MSQUIC_RAW)/msquic.h
	wget -q -O include/msquic_posix.h   $(MSQUIC_RAW)/msquic_posix.h
	wget -q -O include/quic_sal_stub.h  $(MSQUIC_RAW)/quic_sal_stub.h
	@echo ">>> Headers downloaded to include/"

# Generate self-signed certificate for the QUIC coordinator (TLS 1.3 is mandatory)
certs: server.cert server.key

server.cert server.key:
	@echo ">>> Generating self-signed TLS certificate..."
	openssl req -nodes -new -x509 -keyout server.key -out server.cert \
		-subj "/CN=localhost" -days 365 2>/dev/null
	@echo ">>> Certificate generated: server.cert / server.key"

clean:
	rm -f $(ALL) server.cert server.key

distclean: clean
	rm -rf include

# --- SETUP INSTRUCTIONS ---
# 1. Install msquic runtime:
#      curl -sSL https://packages.microsoft.com/keys/microsoft.asc | sudo tee /etc/apt/trusted.gpg.d/microsoft.asc
#      sudo apt-add-repository https://packages.microsoft.com/ubuntu/$(lsb_release -rs)/prod
#      sudo apt update
#      sudo apt install libmsquic
#
# 2. Build (headers are downloaded automatically):
#      make
#
# 3. Run (3 terminals):
#      Terminal 1:  ./quic_coordinator
#      Terminal 2:  ./quic_producer 5
#      Terminal 3:  ./quic_worker

.PHONY: all clean distclean certs headers

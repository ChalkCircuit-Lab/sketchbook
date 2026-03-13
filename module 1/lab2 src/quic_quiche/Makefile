CC       = gcc
CFLAGS   = -Wall -Wextra -O2 -I quiche-repo/quiche/include
LDFLAGS  = quiche-repo/target/release/libquiche.a -lm -ldl -lpthread

TARGETS  = quiche_coordinator quiche_producer quiche_worker

QUICHE_LIB    = quiche-repo/target/release/libquiche.a
QUICHE_HEADER = quiche-repo/quiche/include/quiche.h

# ============================================================================
# Default target: build deps, generate certs, compile everything
# ============================================================================
all: deps certs $(TARGETS)
	@echo ""
	@echo "=== Build complete ==="
	@echo "Run in 3 terminals:"
	@echo "  Terminal 1:  ./quiche_coordinator"
	@echo "  Terminal 2:  ./quiche_producer 5"
	@echo "  Terminal 3:  ./quiche_worker"

quiche_coordinator: quiche_coordinator.c quiche_common.h $(QUICHE_LIB)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

quiche_producer: quiche_producer.c quiche_common.h $(QUICHE_LIB)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

quiche_worker: quiche_worker.c quiche_common.h $(QUICHE_LIB)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# ============================================================================
# Build quiche from source (requires Rust toolchain)
# ============================================================================
deps: $(QUICHE_LIB)

$(QUICHE_LIB): $(QUICHE_HEADER)
	@echo ">>> Building quiche (this takes a few minutes on first run)..."
	cd quiche-repo && cargo build --release --features ffi
	@echo ">>> quiche built: $(QUICHE_LIB)"

$(QUICHE_HEADER):
	@command -v cargo >/dev/null 2>&1 || { \
		echo ""; \
		echo "ERROR: Rust toolchain (cargo) not found."; \
		echo "Install it with:"; \
		echo "  curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh"; \
		echo "  source ~/.cargo/env"; \
		echo ""; \
		exit 1; \
	}
	@echo ">>> Cloning quiche repository..."
	git clone --recursive https://github.com/cloudflare/quiche.git quiche-repo
	@echo ">>> quiche cloned to quiche-repo/"

# ============================================================================
# Generate self-signed TLS certificate (QUIC mandates TLS 1.3)
# ============================================================================
certs: server.cert server.key

server.cert server.key:
	@echo ">>> Generating self-signed TLS certificate..."
	openssl req -nodes -new -x509 -keyout server.key -out server.cert \
		-subj "/CN=localhost" -days 365 2>/dev/null
	@echo ">>> Certificate generated: server.cert / server.key"

# ============================================================================
# Cleanup
# ============================================================================
clean:
	rm -f $(TARGETS) server.cert server.key

distclean: clean
	rm -rf quiche-repo

# ============================================================================
# SETUP INSTRUCTIONS
# ============================================================================
# 1. Install Rust (if not already):
#      curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
#      source ~/.cargo/env
#
#sudo apt install build-essential cmake pkg-config libclang-dev libfontconfig1-dev

#
# 2. Build everything (clones quiche, compiles Rust lib, compiles C code):
#      make
#
# 3. Run (3 terminals):
#      Terminal 1:  ./quiche_coordinator
#      Terminal 2:  ./quiche_producer 5
#      Terminal 3:  ./quiche_worker
#
# Notes:
#   - First build takes 2-5 minutes (Rust compilation + BoringSSL).
#   - Subsequent builds are fast (only C code recompiles).
#   - `make distclean` removes the quiche-repo entirely.
#   - The static library (libquiche.a) bundles BoringSSL, so no
#     separate OpenSSL/BoringSSL linkage is needed at runtime.

.PHONY: all clean distclean deps certs

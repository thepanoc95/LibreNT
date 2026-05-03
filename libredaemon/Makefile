# Makefile for libredaemon cross-compilation
# Supports Linux-to-Windows cross-compilation using MinGW-w64

# Default target
TARGET ?= x86_64-pc-windows-gnu
BINARY_NAME = libredaemon

.PHONY: all build build-release clean check help install-deps

all: build

# Build debug for Windows target
build:
	cargo build --target $(TARGET)

# Build release for Windows target
build-release:
	cargo build --target $(TARGET) --release

# Build for both architectures
build-all:
	$(MAKE) build-release TARGET=x86_64-pc-windows-gnu
	$(MAKE) build-release TARGET=i686-pc-windows-gnu

# Clean build artifacts
clean:
	cargo clean

# Check code (runs on host)
check:
	cargo check

# Format code
fmt:
	cargo fmt

# Run clippy
clippy:
	cargo clippy --target $(TARGET)

# Install MinGW-w64 dependencies (Debian/Ubuntu)
install-deps:
	sudo apt-get update && sudo apt-get install -y \
		mingw-w64 \
		gcc-mingw-w64-x86-64 \
		gcc-mingw-w64-i686

# Build and strip the binary for smaller size
strip: build-release
	@if [ "$(TARGET)" = "x86_64-pc-windows-gnu" ]; then \
		x86_64-w64-mingw32-strip target/$(TARGET)/release/$(BINARY_NAME).exe; \
	elif [ "$(TARGET)" = "i686-pc-windows-gnu" ]; then \
		i686-w64-mingw32-strip target/$(TARGET)/release/$(BINARY_NAME).exe; \
	fi

# Show help
help:
	@echo "libredaemon Makefile - Cross-compilation support"
	@echo ""
	@echo "Usage: make [target] [TARGET=arch]"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build for Windows (default)"
	@echo "  build        - Build debug for Windows"
	@echo "  build-release- Build release for Windows"
	@echo "  build-all    - Build for both x86_64 and i686 Windows"
	@echo "  clean        - Clean build artifacts"
	@echo "  check        - Run cargo check on host"
	@echo "  fmt          - Format code with rustfmt"
	@echo "  clippy       - Run clippy on target"
	@echo "  install-deps - Install MinGW-w64 dependencies"
	@echo "  strip        - Build release and strip symbols"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Targets:"
	@echo "  x86_64-pc-windows-gnu - 64-bit Windows (default)"
	@echo "  i686-pc-windows-gnu   - 32-bit Windows"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Build debug for 64-bit Windows"
	@echo "  make build-release      # Build release for 64-bit Windows"
	@echo "  make build-release TARGET=i686-pc-windows-gnu  # Build for 32-bit Windows"
#!/bin/bash
# Build script for cross-compiling WinELF with mingw-w64
# Usage: ./build-winelf.sh [debug|release]

set -e

BUILD_TYPE="${1:-release}"

echo "Building WinELF with mingw-w64 cross-compiler"
echo "Build type: $BUILD_TYPE"

# Check for Rust
if ! command -v cargo &> /dev/null; then
    echo "Error: cargo not found"
    echo "Install Rust: curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh"
    exit 1
fi

# Install Windows target if not present
rustup target add x86_64-pc-windows-gnu

# Build
cd winelf
cargo build --release --target x86_64-pc-windows-gnu

echo "Build complete!"
echo "Output: winelf/target/x86_64-pc-windows-gnu/release/winelf-loader.exe"
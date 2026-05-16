#!/bin/bash
# PROJECT:     LibreNT
# LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
# PURPOSE:     Project maintenance script
# COPYRIGHT:   Copyright 2026 Panoc95

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
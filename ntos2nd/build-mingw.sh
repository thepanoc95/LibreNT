#!/bin/bash
# PROJECT:     LibreNT
# LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
# PURPOSE:     Project maintenance script
# COPYRIGHT:   Copyright 2026 Panoc95

set -e

BUILD_TYPE="${1:-release}"

echo "Building ntos2nd with mingw-w64 cross-compiler"
echo "Build type: $BUILD_TYPE"

# Check for mingw-w64 cross-compiler
if ! command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    echo "Error: x86_64-w64-mingw32-gcc not found"
    echo "Install mingw-w64: apt install mingw-w64"
    exit 1
fi

# Create build directory
mkdir -p build-mingw

# Configure CMake with mingw-w64 cross-compiler
cmake -B build-mingw \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
    -DCMAKE_RC_COMPILER=x86_64-w64-mingw32-windres \
    -DCMAKE_SYSTEM_NAME=Windows

# Build
cmake --build build-mingw --config $BUILD_TYPE

echo "Build complete!"
echo "Output: build-mingw/ntos2nd_exe.exe"
# libredaemon

A minimal system daemon for LibreNT Windows experimental environment.

## Overview

`libredaemon` is a lightweight daemon that executes an init script (`init.rc`) at startup. It provides:

- Init script parsing
- Process spawning (using Windows API)
- Environment variable management
- Registry modification
- Logging utilities

## Architecture

```
src/
├── main.rs          # Entry point
├── init.rs          # init.rc parser
├── process.rs       # Process spawning logic
├── registry.rs      # Registry operations
├── environment.rs   # Environment handling
└── logger.rs        # Logging utilities
```

## Building

### Requirements

- Rust SDK (2021 edition)
- Windows (for native WinAPI support)

### Build Commands

```powershell
# Debug build
cargo build

# Release build
cargo build --release
```

### Cross-compiling from Linux

The project supports cross-compilation from Linux to Windows using MinGW-w64.

#### Quick Start

```bash
# Install dependencies (Debian/Ubuntu)
sudo apt install mingw-w64

# Install the Windows target
rustup target add x86_64-pc-windows-gnu

# Build using Makefile (recommended)
make build-release

# Or build manually
cargo build --release --target x86_64-pc-windows-gnu
```

#### Build Targets

| Target | Description | Output |
|--------|-------------|--------|
| `x86_64-pc-windows-gnu` | 64-bit Windows | `libredaemon.exe` |
| `i686-pc-windows-gnu` | 32-bit Windows | `libredaemon.exe` |

#### Makefile Commands

```bash
make build          # Debug build for Windows
make build-release  # Release build for Windows
make build-all      # Build for both architectures
make strip          # Build release and strip symbols (smaller binary)
make clean          # Clean build artifacts
```

#### Output Location

Cross-compiled binaries are placed in `target/<target>/release/libredaemon.exe`

## Usage

```powershell
# Run with default init.rc
libredaemon.exe

# Run with custom init script
libredaemon.exe path\to\custom.rc
```

## Init Script Format

The init script format is line-based. Supported commands:

| Command | Description |
|---------|-------------|
| `start <exe>` | Start a process |
| `spawn <exe> [args...]` | Spawn a process with arguments |
| `setenv <key> <value>` | Set environment variable |
| `regset <path> <value>` | Set registry value |
| `log <message>` | Log a message |

Comments start with `#`. Empty lines are ignored.

### Example init.rc

```
# LibreNT Init Script
setenv LIBRENT 1
regset HKCU\Software\LibreNT\Mode debug
spawn notepad.exe
log Boot complete
```

## Features

- **Init Script Parsing**: Line-based command parsing with comment support
- **Process Spawning**: Creates processes with `CREATE_SUSPENDED`, then resumes immediately
- **Environment Variables**: Uses `SetEnvironmentVariableW` API
- **Registry Operations**: Uses `RegCreateKeyExW` and `RegSetValueExW` APIs
- **Logging**: Simple console output with `[INFO]` and `[ERR]` prefixes


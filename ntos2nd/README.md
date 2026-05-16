# ntos2nd

A usermode kernel for Windows that provides a sandboxed environment for running programs, bridging Windows APIs to an open source environment.

## Overview

ntos2nd implements a usermode Windows NT-like kernel that:
- Runs programs in a sandboxed child process
- Routes unknown/undocumented Windows APIs to the host system
- Implements its own syscalls mirroring Windows NT Native API
- Provides API bridge functionality for Windows compatibility

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    Host Windows NT                       │
│                  (ntoskrnl.exe, ntdll.dll)               │
└─────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────┐
│                   ntos2nd Kernel                         │
│                                                          │
│  ┌─────────────┐  ┌──────────────┐  ┌────────────────┐ │
│  │   Syscall   │  │  API Bridge  │  │   Sandbox      │ │
│  │  Interface  │  │   Layer      │  │ Infrastructure │ │
│  └─────────────┘  └──────────────┘  └────────────────┘ │
└─────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────┐
│              Sandboxed Child Process                     │
│                                                          │
│  ┌─────────────┐  ┌──────────────┐  ┌────────────────┐ │
│  │  Win32 API  │  │ NT Native API│  │ IPC Client     │ │
│  │   Calls     │  │   Calls       │  │                │ │
│  └─────────────┘  └──────────────┘  └────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

## Components

### kernel.c/h - Core Kernel
- Process and thread management
- Handle table management
- Basic kernel infrastructure

### syscall.c/h - Syscall Interface
- Windows NT Native API syscall implementation
- Syscall dispatcher
- Handle management functions

### apibridge.c/h - API Bridge Layer
- Windows API forwarding
- Dynamic API resolution
- Hot-patch support

### sandbox.c/h - Sandbox Infrastructure
- Process sandboxing
- IPC communication
- Syscall filtering
- Memory access restrictions

### undocumented.c/h - Undocumented API Passthrough
- Resolution of undocumented NT APIs
- Forwarding to host system
- Dynamic symbol resolution

## Building

### Native Windows Build (MSVC)
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Native Windows Build (MinGW)
```bash
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
cmake --build . --config Release
```

## Cross-Compilation (Linux → Windows)

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt install mingw-w64 cmake

# Fedora
sudo dnf install mingw64-gcc mingw64-cmake

# Arch Linux
sudo pacman -S mingw-w64-gcc mingw-w64-cmake
```

### Build with mingw-w64
```bash
# Using the helper script
./build-mingw.sh release

# Or manually with CMake
cmake -B build-mingw \
    -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
    -DCMAKE_RC_COMPILER=x86_64-w64-mingw32-windres \
    -DCMAKE_SYSTEM_NAME=Windows
cmake --build build-mingw --config Release
```

### Build WinELF with Cross-Compilation
```bash
# Install Rust Windows target
rustup target add x86_64-pc-windows-gnu

# Build WinELF
./build-winelf.sh release

# Or manually
cd winelf
cargo build --release --target x86_64-pc-windows-gnu
```

## Usage

```bash
# Run a program in the sandbox
ntos2nd.exe notepad.exe

# Run with specific options
ntos2nd.exe --sandbox --filter myapp.exe
```

## Features


### NT Native Wrapper Coverage

`NTCall.c` provides direct wrapper entry points routed through `NTOS2NDHandleSyscall`.
Current wrappers include:

- `NtCloseWrap`
- `NtOpenProcessWrap`
- `NtReadFileWrap`
- `NtWriteFileWrap`
- `NtAllocateVirtualMemoryWrap`

These wrappers are declared in `NTCall.h` for consumers embedding ntos2nd.

### Process Sandbox
- Child processes run isolated from host
- IPC communication for syscall forwarding
- Configurable restrictions (filesystem, network, registry)

### API Compatibility
- Documented Win32 APIs forwarded to host
- Undocumented NT APIs resolved dynamically
- Custom API implementations for open-source compatibility

### Security
- Syscall filtering
- Memory access restrictions
- Process and thread limits

## License

(C) 2025 OakyMacintosh

## Future Work

- [ ] JIT syscall interception
- [ ] Memory protection enhancements
- [ ] Network isolation
- [ ] Filesystem virtualization
- [ ] Registry redirection

## WinELF - ELF Support for Windows

A Rust-based ELF loader that enables running Linux ELF executables on Windows through the ntos2nd kernel.

### Architecture

```
┌─────────────────────────────────────────────────┐
│              Windows Host                       │
│  ┌──────────────────────────────────────────┐   │
│  │           WinELF Loader                  │   │
│  │  ┌─────────┐ ┌─────────┐ ┌────────────┐ │   │
│  │  │  ELF    │ │ Memory  │ │ Emulator/  │ │   │
│  │  │ Parser  │ │ Manager │ │ Syscalls   │ │   │
│  │  └─────────┘ └─────────┘ └────────────┘ │   │
│  └──────────────────────────────────────────┘   │
└─────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────┐
│           ELF Executable (Linux)                │
│  - Loadable segments mapped to Windows memory  │
│  - Syscalls translated to Windows equivalents  │
│  - Dynamic linking handled                     │
└─────────────────────────────────────────────────┘
```

### Building WinELF

```bash
cd winelf
cargo build --release
```

### Features


### NT Native Wrapper Coverage

`NTCall.c` provides direct wrapper entry points routed through `NTOS2NDHandleSyscall`.
Current wrappers include:

- `NtCloseWrap`
- `NtOpenProcessWrap`
- `NtReadFileWrap`
- `NtWriteFileWrap`
- `NtAllocateVirtualMemoryWrap`

These wrappers are declared in `NTCall.h` for consumers embedding ntos2nd.

- **ELF Parsing**: Full 32/64-bit ELF support using goblin
- **Memory Management**: Virtual memory mapping with proper protections
- **Segment Loading**: PT_LOAD segment handling with BSS zeroing
- **Syscall Translation**: Linux syscalls to Windows API mapping
- **Modular Design**: Pluggable emulation backend

### Usage

```bash
# Load and run an ELF executable
winelf-loader.exe ./hello_world

# Or use programatically
use winelf::{ElfExecutable, ElfLoader};

let elf = ElfExecutable::parse("program")?;
let mut loader = ElfLoader::new(elf)?;
loader.load()?;
loader.run()?;
```

### Components

| Module | Description |
|--------|-------------|
| `elf/` | ELF binary parsing and representation |
| `loader/` | ELF loading and execution engine |
| `memory/` | Memory management and mapping |
| `syscall/` | Linux syscall handling and translation |
| `emulation/` | Optional CPU instruction emulation |

### Roadmap

- [ ] Dynamic linking support (ld-linux.so)
- [ ] Thread-local storage (TLS)
- [ ] Signal handling
- [ ] Full x86-64 emulation
- [ ] ARM64 support
- [ ] Integration with ntos2nd kernel
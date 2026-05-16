# POSIX Subsystem Strategy for ntos2nd

## Decision

**Do not use either of the proposed methods**:

1. WDK-based runtime kernel-memory injection
2. Offline binary patch/merge into `ntoskrnl.exe`

Instead, implement POSIX support as a **supported user-mode subsystem** hosted by `ntos2nd`, with clean NT syscall translation and optional process sandboxing.

## Why this decision

- Kernel-memory injection and kernel binary patching are unsafe, fragile across updates, and incompatible with secure boot / code integrity.
- Maintaining a patched `ntoskrnl.exe` fork is not practical and creates severe reliability and legal/distribution risk.
- A user-mode subsystem can still deliver Linux/POSIX compatibility while remaining testable, update-safe, and redistributable.

## Recommended architecture

### 1) `libposixnt` translation layer (user mode)

Create a library that maps POSIX ABI calls onto NT-native semantics exposed by ntos2nd:

- `open/read/write/close` -> `NtCreateFile/NtReadFile/NtWriteFile/NtClose`
- `fork/exec` -> process cloning model + `CreateProcess`/`NtCreateProcess` strategy
- `mmap/munmap/mprotect` -> `NtAllocateVirtualMemory/NtFreeVirtualMemory/NtProtectVirtualMemory`
- `poll/select/epoll` -> wait-object abstraction over NT handles
- `pthread` primitives -> NT thread + synchronization object wrappers

### 2) POSIX runtime service inside ntos2nd

Add a subsystem module in `ntos2nd` responsible for:

- path translation (`/` <-> NT path namespace)
- FD table management over ntos2nd handles
- signal emulation strategy
- errno mapping from NTSTATUS/Win32 errors

### 3) Execution front-end

Provide a launcher (`ntposix`) to run POSIX-targeted binaries through:

- existing WinELF path for ELF payloads
- optional PE+POSIX runtime mode for native Windows builds

### 4) Driver usage boundaries

If WDK is used at all, limit it to **supported extension points** only (e.g., file-system mini-filter or observability), not code injection and not kernel image patching.

## Implementation phases

### Phase 1: Core ABI
- file descriptors + file I/O
- `stat`, `lseek`, directory enumeration
- `mmap` basics

### Phase 2: Process and threading
- `execve`, `waitpid`
- `pthread` primitives
- IPC (`pipe`, named pipe-backed transport)

### Phase 3: Signals and advanced runtime
- signal delivery model
- `select/poll/epoll` compatibility layer
- PTY/tty behavior

### Phase 4: Toolchain and packaging
- SDK headers + libc shim docs
- conformance tests
- install-time component registration

## Security model

- no kernel patching
- no runtime kernel memory writes
- maintain auditable user-mode boundary
- leverage signed, versioned binaries only

## Next concrete code tasks

1. Add `ntos2nd/posix/` module scaffold (`posix.c`, `posix.h`, `fdtable.c`, `errno_map.c`).
2. Implement FD table + `open/read/write/close` wrappers on top of existing NT wrappers.
3. Add tests in `ntos2nd/tests/` for path translation and errno mapping.
4. Add `ntposix` launcher stub and wiring in `CMakeLists.txt`.

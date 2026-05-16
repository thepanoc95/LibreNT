# ntos2nd POSIX Subsystem (FIPS PUB 151-1-aligned scaffold)

This directory contains a **usable scaffold** for a POSIX subsystem split into:

- `runtime/posixsubsystem.c` -> user-mode DLL (`posixsubsystem.dll`)
- `driver/posixsubsys_driver.c` -> kernel companion `.sys` skeleton (WDK build target)
- `include/posix_abi.h` -> shared ABI and IOCTL/message contracts

## FIPS 151-1 alignment intent

FIPS PUB 151-1 defines POSIX.1 system API expectations. In this repository we map that expectation to:

- process management entrypoints (`PosixSpawnProcess`)
- errno translation boundary (`PosixTranslateNtStatusToErrno`)
- kernel/user ABI device contract (`\\.\\LibreNTPosix`)

This is an incremental base for implementing POSIX.1 API surface without patching `ntoskrnl.exe`.

## Build notes

- DLL can be compiled with MinGW/MSVC as part of ntos2nd build integration.
- `.sys` requires WDK and should be built in a dedicated VS/WDK driver project.


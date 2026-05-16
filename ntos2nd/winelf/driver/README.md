<!--
 PROJECT:     LibreNT
 LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 PURPOSE:     Project documentation
 COPYRIGHT:   Copyright 2026 Panoc95
-->



# WinELF NTDDK Driver

This directory contains a minimal NTDDK-based kernel-mode driver skeleton for WinELF.
The driver exposes a device interface for future ELF execution support on Windows NT.

## Contents

- `winelf_driver.c` - driver entry point, device creation, symbolic link, and IOCTL dispatch.
- `winelf_abi.h` - shared device names and IOCTL definitions for user-mode <-> kernel-mode communication.

## Build notes

- This driver must be built with the Windows Driver Kit (WDK).
- Use a normal kernel driver project or WDK-enabled CMake toolchain.
- The current implementation is intentionally minimal and returns `STATUS_NOT_IMPLEMENTED` for execution requests.

## Device interface

- Device name: `\\Device\\WinElf`
- DOS symlink: `\\DosDevices\\WinElf`
- IOCTLs:
  - `IOCTL_WINELF_EXECUTE_ELF`
  - `IOCTL_WINELF_QUERY_STATUS`

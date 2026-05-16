<!--
 PROJECT:     LibreNT
 LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 PURPOSE:     Project documentation
 COPYRIGHT:   Copyright 2026 Panoc95
-->



Hello World ELF Example
=====================

This is a simple C program that can be compiled to ELF format for testing WinELF.

Example source code:

```c
#include <unistd.h>

int main() {
    write(1, "Hello from ELF on Windows!\n", 28);
    return 0;
}
```

Compile on Linux:
```bash
gcc -static -o hello hello.c
```

Run with WinELF:
```bash
winelf-loader.exe ./hello
```
WDK Driver Support
------------------

This repository also includes a minimal NTDDK helper driver in `driver/`.
The driver exposes a kernel-side device interface for future ELF execution integration on Windows NT.

The current implementation is a stub and is intended to be built separately with the Windows Driver Kit.

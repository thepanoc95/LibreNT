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
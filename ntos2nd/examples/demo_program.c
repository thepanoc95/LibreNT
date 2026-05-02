/*
    ntos2nd - Demo Program
    
    A simple demonstration of running inside the ntos2nd sandbox.
    
    (C) 2025 OakyMacintosh
*/

#include <windows.h>
#include <stdio.h>

int main(void) {
    printf("[demo] ntos2nd Demo Program\n");
    printf("[demo] =====================\n\n");
    
    printf("[demo] Running in sandbox mode...\n");
    
    /* Demonstrate basic Windows API usage */
    DWORD version = GetVersion();
    printf("[demo] Windows version: 0x%08X\n", version);
    
    /* Test file operations */
    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    printf("[demo] Temp path: %s\n", tempPath);
    
    /* Test string operations */
    char testStr[] = "Hello from ntos2nd!";
    printf("[demo] String test: %s\n", testStr);
    
    /* Get system info */
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    printf("[demo] Processor count: %u\n", sysInfo.dwNumberOfProcessors);
    
    printf("\n[demo] Demo complete!\n");
    
    return 0;
}
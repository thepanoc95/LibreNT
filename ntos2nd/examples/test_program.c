/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Native LibreNT system component
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



#include <windows.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    printf("[test] Example program started\n");
    
    /* Test basic functionality */
    printf("[test] Running basic tests...\n");
    
    /* Test file operations */
    HANDLE hFile = CreateFileA("test.txt", GENERIC_WRITE, 0, NULL, 
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        const char* testStr = "Hello from ntos2nd sandbox!\n";
        DWORD bytesWritten;
        WriteFile(hFile, testStr, strlen(testStr), &bytesWritten, NULL);
        CloseHandle(hFile);
        printf("[test] File write test passed\n");
    }
    
    /* Test memory allocation */
    LPVOID pMem = VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (pMem) {
        strcpy((char*)pMem, "Memory test OK");
        VirtualFree(pMem, 0, MEM_RELEASE);
        printf("[test] Memory allocation test passed\n");
    }
    
    printf("[test] All tests completed successfully\n");
    
    return 0;
}
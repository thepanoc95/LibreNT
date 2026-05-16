/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Native LibreNT system component
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



/*
    ntos2nd - Main Entry Point
    
    Usermode Windows NT-like kernel for Windows.
    
    (C) 2025 OakyMacintosh
*/

#include "MSWindows.System.h"
#include "ntos2nd/kernel.h"
#include "ntos2nd/apibridge.h"
#include "ntos2nd/sandbox.h"
#include "ntos2nd/undocumented.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
    printf("ntos2nd v%s\n", MSWindowsSystemVersion);
    
    /* Initialize kernel */
    NTSTATUS status = NTOS2NDInitialize();
    if (!NT_SUCCESS(status)) {
        printf("Failed to initialize kernel: 0x%08X\n", status);
        return 1;
    }
    
    /* Configure sandbox */
    NTOS2ND_SANDBOX_CONFIG config = {0};
    config.bRestrictFileSystem = TRUE;
    config.bRestrictNetwork = TRUE;
    config.bRestrictRegistry = TRUE;
    config.bEnableSyscallFiltering = TRUE;
    config.dwProcessLimit = 16;
    config.dwThreadLimit = 64;
    
    if (argc > 1) {
        /* Run the specified program in the sandbox */
        printf("Launching: %s\n", argv[1]);
        NTOS2ND_PROCESS* process = NTOS2NDRunProgram(argv[1], &config);
        
        if (process) {
            printf("Process started with PID: %u\n", process->ProcessId);
            
            /* Wait for completion */
            NTOS2NDWaitForProcessTermination(process, INFINITE);
            printf("Process completed\n");
        } else {
            printf("Failed to launch program\n");
        }
    } else {
        printf("Usage: ntos2nd <program.exe> [args...]\n");
        printf("Example: ntos2nd notepad.exe\n");
    }
    
    /* Shutdown */
    NTOS2NDShutdown();
    
    return 0;
}
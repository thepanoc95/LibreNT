/*
    ntos2nd - Kernel Test Program
    
    Demonstrates using the ntos2nd kernel as a host.
    
    (C) 2025 OakyMacintosh
*/

#include <windows.h>
#include <stdio.h>
#include "../ntos2nd/kernel.h"
#include "../ntos2nd/sandbox.h"

int main(int argc, char* argv[]) {
    printf("[host] ntos2nd Kernel Test\n");
    printf("[host] ===================\n\n");
    
    /* Initialize kernel */
    NTSTATUS status = NTOS2NDInitialize();
    if (!NT_SUCCESS(status)) {
        printf("[host] Failed to initialize kernel: 0x%08X\n", status);
        return 1;
    }
    printf("[host] Kernel initialized successfully\n");
    
    /* Configure sandbox */
    NTOS2ND_SANDBOX_CONFIG sandboxConfig = {0};
    sandboxConfig.bRestrictFileSystem = FALSE;
    sandboxConfig.bRestrictNetwork = FALSE;
    sandboxConfig.bRestrictRegistry = TRUE;
    sandboxConfig.bEnableSyscallFiltering = TRUE;
    sandboxConfig.dwProcessLimit = 4;
    sandboxConfig.dwThreadLimit = 16;
    
    /* Run test program */
    if (argc > 1) {
        printf("[host] Running: %s\n", argv[1]);
        NTOS2ND_PROCESS* process = NTOS2NDRunProgram(argv[1], &sandboxConfig);
        
        if (process) {
            printf("[host] Process started with PID: %u\n", process->ProcessId);
            NTOS2NDWaitForProcessTermination(process, INFINITE);
            printf("[host] Process terminated\n");
        } else {
            printf("[host] Failed to start process\n");
        }
    } else {
        printf("[host] No program specified. Usage: ntos2nd_test <program.exe>\n");
    }
    
    /* Shutdown kernel */
    NTOS2NDShutdown();
    printf("[host] Test complete\n");
    
    return 0;
}
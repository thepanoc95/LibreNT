/*
    ntos2nd - Main Kernel Entry Point
    
    (C) 2025 OakyMacintosh
*/

#include "kernel.h"
#include "syscall.h"
#include "apibridge.h"
#include "sandbox.h"
#include "undocumented.h"

/* Main kernel instance */
static BOOL g_bKernelRunning = FALSE;

/* Initialize all kernel subsystems */
NTSTATUS NTOS2NDInitialize(void) {
    NTSTATUS status;
    
    /* Initialize main kernel */
    status = NTOS2NDInitializeKernel();
    if (!NT_SUCCESS(status)) {
        return status;
    }
    
    /* Initialize API bridge */
    status = NTOS2NDInitializeAPIBridge();
    if (!NT_SUCCESS(status)) {
        NTOS2NDShutdownKernel();
        return status;
    }
    
    /* Initialize undocumented API layer */
    status = NTOS2NDInitializeUndocumentedAPI();
    if (!NT_SUCCESS(status)) {
        NTOS2NDShutdownAPIBridge();
        NTOS2NDShutdownKernel();
        return status;
    }
    
    g_bKernelRunning = TRUE;
    
    printf("[ntos2nd] Kernel v%d.%d.%d fully initialized\n",
           NTOS2ND_VERSION_MAJOR, NTOS2ND_VERSION_MINOR, NTOS2ND_VERSION_PATCH);
    
    return STATUS_SUCCESS;
}

/* Shutdown kernel */
NTSTATUS NTOS2NDShutdown(void) {
    if (!g_bKernelRunning) {
        return STATUS_SUCCESS;
    }
    
    g_bKernelRunning = FALSE;
    
    NTOS2NDShutdownAPIBridge();
    NTOS2NDShutdownKernel();
    
    printf("[ntos2nd] Kernel shutdown complete\n");
    
    return STATUS_SUCCESS;
}

/* Run a program in the sandbox */
NTOS2ND_PROCESS* NTOS2NDRunProgram(const char* ProgramPath, NTOS2ND_SANDBOX_CONFIG* Config) {
    if (!g_bKernelRunning) {
        return NULL;
    }
    
    if (Config) {
        return NTOS2NDLoadSandboxProcess(ProgramPath, Config);
    } else {
        return NTOS2NDCreateProcess(ProgramPath, TRUE);
    }
}

/* Wait for process */
NTSTATUS NTOS2NDWaitForProcessTermination(NTOS2ND_PROCESS* Process, DWORD dwMilliseconds) {
    return NTOS2NDWaitForProcess(Process, dwMilliseconds);
}

/* Get kernel status */
BOOL NTOS2NDIsRunning(void) {
    return g_bKernelRunning;
}

/* Get kernel version */
const char* NTOS2NDGetVersion(void) {
    static const char* version = "ntos2nd v1.0.0";
    return version;
}
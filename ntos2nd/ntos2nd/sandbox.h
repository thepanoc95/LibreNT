/*
    ntos2nd - Sandbox and IPC Layer
    
    Implements sandboxed execution of child processes with IPC communication.
    
    (C) 2025 OakyMacintosh
*/

#ifndef NTOS2ND_SANDBOX_H
#define NTOS2ND_SANDBOX_H

#include "kernel.h"
#include "syscall.h"

/* Sandbox configuration */
typedef struct _NTOS2ND_SANDBOX_CONFIG {
    BOOL bRestrictFileSystem;      /* Restrict file system access */
    BOOL bRestrictNetwork;         /* Restrict network access */
    BOOL bRestrictRegistry;        /* Restrict registry access */
    BOOL bEnableSyscallFiltering;  /* Enable syscall filtering */
    BOOL bEnableMemorySandbox;     /* Enable memory sandbox */
    DWORD dwProcessLimit;          /* Max child processes */
    DWORD dwThreadLimit;           /* Max threads per process */
    char szAllowedPaths[4][260];  /* Allowed file paths */
} NTOS2ND_SANDBOX_CONFIG;

/* Sandbox context */
typedef struct _NTOS2ND_SANDBOX {
    NTOS2ND_PROCESS* Process;
    NTOS2ND_SANDBOX_CONFIG Config;
    CRITICAL_SECTION Lock;
    uint32_t SyscallCount;
    uint32_t RestrictedSyscallCount;
} NTOS2ND_SANDBOX;

/* IPC message types for sandbox communication */
#define SANDBOX_MSG_TYPE_SYSCALL        0x1001
#define SANDBOX_MSG_TYPE_API_CALL       0x1002
#define SANDBOX_MSG_TYPE_EXCEPTION      0x1003
#define SANDBOX_MSG_TYPE_EXIT           0x1004
#define SANDBOX_MSG_TYPE_DEBUG          0x1005

/* Sandbox IPC message */
typedef struct _NTOS2ND_SANDBOX_IPC {
    uint32_t MessageType;
    uint32_t ProcessId;
    uint32_t ThreadId;
    uint32_t SyscallNumber;
    uint64_t Parameters[16];
    uint64_t Result;
    NTSTATUS Status;
    DWORD dwErrorCode;
} NTOS2ND_SANDBOX_IPC;

/* Sandbox functions */
NTSTATUS NTOS2NDInitializeSandbox(NTOS2ND_SANDBOX* Sandbox, NTOS2ND_SANDBOX_CONFIG* Config);
void NTOS2NDShutdownSandbox(NTOS2ND_SANDBOX* Sandbox);

/* Child process loader */
NTOS2ND_PROCESS* NTOS2NDLoadSandboxProcess(const char* ProcessPath, NTOS2ND_SANDBOX_CONFIG* Config);
NTSTATUS NTOS2NDUnloadSandboxProcess(NTOS2ND_PROCESS* Process);

/* IPC handlers */
NTSTATUS NTOS2NDHandleSandboxIPCMessages(NTOS2ND_PROCESS* Process);
NTSTATUS NTOS2NDSendSandboxMessage(NTOS2ND_PROCESS* Process, NTOS2ND_SANDBOX_IPC* Message);
NTSTATUS NTOS2NDReceiveSandboxMessage(NTOS2ND_PROCESS* Process, NTOS2ND_SANDBOX_IPC* Message);

/* Syscall filtering */
BOOL NTOS2NDIsSyscallAllowed(NTOS2ND_SANDBOX* Sandbox, uint32_t SyscallNumber);
NTSTATUS NTOS2NDFilterSyscall(NTOS2ND_SANDBOX* Sandbox, NTOS2ND_SYSCALL_CONTEXT* Context);

/* Memory sandbox */
NTSTATUS NTOS2NDSandboxMemoryAccess(NTOS2ND_SANDBOX* Sandbox, uint64_t Address, size_t Size, BOOL bWrite);

#endif /* NTOS2ND_SANDBOX_H */
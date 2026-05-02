/*
    ntos2nd - Sandbox and IPC Layer Implementation
    
    (C) 2025 OakyMacintosh
*/

#include "sandbox.h"
#include <psapi.h>

/* Default sandbox configuration */
static NTOS2ND_SANDBOX_CONFIG g_DefaultSandboxConfig = {
    .bRestrictFileSystem = TRUE,
    .bRestrictNetwork = TRUE,
    .bRestrictRegistry = TRUE,
    .bEnableSyscallFiltering = TRUE,
    .bEnableMemorySandbox = TRUE,
    .dwProcessLimit = 16,
    .dwThreadLimit = 64,
    .szAllowedPaths = { "C:\\Windows", "C:\\Program Files", "" }
};

/* Worker thread for sandbox process communication */
static DWORD WINAPI NTOS2NDSandboxWorker(LPVOID lpParameter) {
    NTOS2ND_PROCESS* Process = (NTOS2ND_PROCESS*)lpParameter;
    NTOS2ND_SANDBOX_IPC Message;
    DWORD dwBytesRead;
    
    while (Process->State == ProcessStateRunning) {
        if (ReadFile(Process->PipeRead, &Message, sizeof(Message), &dwBytesRead, NULL) && 
            dwBytesRead == sizeof(Message)) {
            
            NTOS2ND_SYSCALL_CONTEXT Context;
            Context.Process = Process;
            Context.SyscallNumber = Message.SyscallNumber;
            Context.ParameterCount = 5;
            memcpy(Context.Parameters, Message.Parameters, sizeof(uint64_t) * 5);
            
            /* Dispatch syscall */
            NTOS2NDDispatchSyscall(&Context);
            
            Message.Result = Context.Result;
            Message.Status = Context.Status;
            
            /* Send response */
            DWORD dwBytesWritten;
            WriteFile(Process->PipeWrite, &Message, sizeof(Message), &dwBytesWritten, NULL);
        }
        
        Sleep(1); /* Yield */
    }
    
    return 0;
}

/* Initialize sandbox */
NTSTATUS NTOS2NDInitializeSandbox(NTOS2ND_SANDBOX* Sandbox, NTOS2ND_SANDBOX_CONFIG* Config) {
    if (!Sandbox) {
        return STATUS_INVALID_PARAMETER;
    }
    
    memset(Sandbox, 0, sizeof(NTOS2ND_SANDBOX));
    
    if (Config) {
        memcpy(&Sandbox->Config, Config, sizeof(NTOS2ND_SANDBOX_CONFIG));
    } else {
        memcpy(&Sandbox->Config, &g_DefaultSandboxConfig, sizeof(NTOS2ND_SANDBOX_CONFIG));
    }
    
    InitializeCriticalSection(&Sandbox->Lock);
    Sandbox->SyscallCount = 0;
    Sandbox->RestrictedSyscallCount = 0;
    
    return STATUS_SUCCESS;
}

/* Shutdown sandbox */
void NTOS2NDShutdownSandbox(NTOS2ND_SANDBOX* Sandbox) {
    if (!Sandbox) {
        return;
    }
    
    DeleteCriticalSection(&Sandbox->Lock);
    memset(Sandbox, 0, sizeof(NTOS2ND_SANDBOX));
}

/* Load sandbox process */
NTOS2ND_PROCESS* NTOS2NDLoadSandboxProcess(const char* ProcessPath, NTOS2ND_SANDBOX_CONFIG* Config) {
    NTOS2ND_PROCESS* Process = NTOS2NDCreateProcess(ProcessPath, TRUE);
    if (!Process) {
        return NULL;
    }
    
    /* Apply sandbox configuration */
    if (Config) {
        /* Restrict process permissions */
        HANDLE hToken;
        if (OpenProcessToken(Process->ProcessHandle, TOKEN_ALL_ACCESS, &hToken)) {
            /* Drop unnecessary privileges */
            TOKEN_PRIVILEGES tp;
            memset(&tp, 0, sizeof(tp));
            tp.PrivilegeCount = 1;
            LookupPrivilegeValueA(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
            tp.Privileges[0].Attributes = SE_PRIVILEGE_REMOVED;
            AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
            CloseHandle(hToken);
        }
    }
    
    return Process;
}

/* Unload sandbox process */
NTSTATUS NTOS2NDUnloadSandboxProcess(NTOS2ND_PROCESS* Process) {
    return NTOS2NDTerminateProcess(Process);
}

/* Handle sandbox IPC messages */
NTSTATUS NTOS2NDHandleSandboxIPCMessages(NTOS2ND_PROCESS* Process) {
    return NTOS2NDSandboxWorker(Process);
}

/* Send sandbox message */
NTSTATUS NTOS2NDSendSandboxMessage(NTOS2ND_PROCESS* Process, NTOS2ND_SANDBOX_IPC* Message) {
    DWORD dwBytesWritten;
    BOOL bResult = WriteFile(Process->PipeWrite, Message, sizeof(NTOS2ND_SANDBOX_IPC), &dwBytesWritten, NULL);
    return bResult ? STATUS_SUCCESS : GetLastError();
}

/* Receive sandbox message */
NTSTATUS NTOS2NDReceiveSandboxMessage(NTOS2ND_PROCESS* Process, NTOS2ND_SANDBOX_IPC* Message) {
    DWORD dwBytesRead;
    BOOL bResult = ReadFile(Process->PipeRead, Message, sizeof(NTOS2ND_SANDBOX_IPC), &dwBytesRead, NULL);
    return bResult ? STATUS_SUCCESS : GetLastError();
}

/* Check if syscall is allowed */
BOOL NTOS2NDIsSyscallAllowed(NTOS2ND_SANDBOX* Sandbox, uint32_t SyscallNumber) {
    if (!Sandbox->Config.bEnableSyscallFiltering) {
        return TRUE;
    }
    
    /* Block dangerous syscalls by default */
    switch (SyscallNumber) {
        case SYSCALL_NtOpenProcess:
        case SYSCALL_NtOpenThread:
            /* Allow only if explicitly permitted */
            return FALSE;
            
        case SYSCALL_NtCreateSection:
            /* Could be used for code injection */
            return FALSE;
            
        default:
            return TRUE;
    }
}

/* Filter syscall */
NTSTATUS NTOS2NDFilterSyscall(NTOS2ND_SANDBOX* Sandbox, NTOS2ND_SYSCALL_CONTEXT* Context) {
    DWORD ticks = GetTickCount();
    
    if (!NTOS2NDIsSyscallAllowed(Sandbox, Context->SyscallNumber)) {
        Sandbox->RestrictedSyscallCount++;
        printf("[ntos2nd] Blocked restricted syscall %u\n", Context->SyscallNumber);
        return STATUS_ACCESS_DENIED;
    }
    
    Sandbox->SyscallCount++;
    
    /* Rate limiting */
    if (Sandbox->SyscallCount % 1000 == 0) {
        printf("[ntos2nd] Processed %u syscalls\n", Sandbox->SyscallCount);
    }
    
    /* Call the syscall */
    return NTOS2NDDispatchSyscall(Context);
}

/* Sandbox memory access */
NTSTATUS NTOS2NDSandboxMemoryAccess(NTOS2ND_SANDBOX* Sandbox, uint64_t Address, size_t Size, BOOL bWrite) {
    if (!Sandbox->Config.bEnableMemorySandbox) {
        return STATUS_SUCCESS;
    }
    
    /* Check for invalid addresses */
    if (Address < 0x10000) {
        return STATUS_ACCESS_VIOLATION;
    }
    
    /* Additional checks can be added here */
    return STATUS_SUCCESS;
}
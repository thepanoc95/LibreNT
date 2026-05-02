/*
    ntos2nd - Usermode Windows NT-like Kernel Implementation
    
    (C) 2025 OakyMacintosh
*/

#include "kernel.h"

/* Kernel globals */
static BOOL g_bKernelInitialized = FALSE;
static CRITICAL_SECTION g_KernelLock;
static NTOS2ND_PROCESS g_ProcessTable[NTOS2ND_MAX_PROCESSES];
static uint32_t g_NextProcessId = 1;
static NTOS2ND_THREAD g_ThreadTable[NTOS2ND_MAX_PROCESSES * NTOS2ND_MAX_THREADS_PER_PROCESS];
static uint32_t g_NextThreadId = 1;
static NTOS2ND_API_BRIDGE g_ApiBridges[NTOS2ND_MAX_API_BRIDGES];
static uint32_t g_ApiBridgeCount = 0;

/* Helper function to find free process slot */
static NTOS2ND_PROCESS* NTOS2NDFindFreeProcessSlot(void) {
    for (int i = 0; i < NTOS2ND_MAX_PROCESSES; i++) {
        if (g_ProcessTable[i].Header.Signature != 0x50524F43) { /* 'PROC' */
            memset(&g_ProcessTable[i], 0, sizeof(NTOS2ND_PROCESS));
            g_ProcessTable[i].Header.Signature = 0x50524F43;
            return &g_ProcessTable[i];
        }
    }
    return NULL;
}

/* Helper function to find free thread slot */
static NTOS2ND_THREAD* NTOS2NDFindFreeThreadSlot(void) {
    for (int i = 0; i < NTOS2ND_MAX_PROCESSES * NTOS2ND_MAX_THREADS_PER_PROCESS; i++) {
        if (g_ThreadTable[i].Header.Signature != 0x54485244) { /* 'THRD' */
            memset(&g_ThreadTable[i], 0, sizeof(NTOS2ND_THREAD));
            g_ThreadTable[i].Header.Signature = 0x54485244;
            return &g_ThreadTable[i];
        }
    }
    return NULL;
}

/* Worker thread for child process communication */
static DWORD WINAPI NTOS2NDWorkerThread(LPVOID lpParameter) {
    NTOS2ND_PROCESS* Process = (NTOS2ND_PROCESS*)lpParameter;
    NTOS2ND_IPC_MESSAGE Message;
    DWORD dwBytesRead;
    
    while (Process->State == ProcessStateRunning) {
        if (ReadFile(Process->PipeRead, &Message, sizeof(Message), &dwBytesRead, NULL) && 
            dwBytesRead == sizeof(Message)) {
            
            /* Handle syscall from child process */
            NTOS2NDHandleSyscall(Process, Message.SyscallNumber,
                                 (uint64_t*)&Message.Parameter1, 5, &Message.Result);
            Message.Status = 0; /* STATUS_SUCCESS */
            
            /* Send response back */
            DWORD dwBytesWritten;
            WriteFile(Process->PipeWrite, &Message, sizeof(Message), &dwBytesWritten, NULL);
        }
    }
    
    return 0;
}

/* Initialize the kernel */
NTSTATUS NTOS2NDInitializeKernel(void) {
    if (g_bKernelInitialized) {
        return STATUS_SUCCESS;
    }
    
    InitializeCriticalSection(&g_KernelLock);
    memset(g_ProcessTable, 0, sizeof(g_ProcessTable));
    memset(g_ThreadTable, 0, sizeof(g_ThreadTable));
    memset(g_ApiBridges, 0, sizeof(g_ApiBridges));
    
    g_bKernelInitialized = TRUE;
    
    printf("[ntos2nd] Kernel initialized (v%d.%d.%d)\n", 
           NTOS2ND_VERSION_MAJOR, NTOS2ND_VERSION_MINOR, NTOS2ND_VERSION_PATCH);
    
    return STATUS_SUCCESS;
}

/* Shutdown the kernel */
NTSTATUS NTOS2NDShutdownKernel(void) {
    if (!g_bKernelInitialized) {
        return STATUS_SUCCESS;
    }
    
    /* Terminate all running processes */
    for (int i = 0; i < NTOS2ND_MAX_PROCESSES; i++) {
        if (g_ProcessTable[i].Header.Signature == 0x50524F43) {
            NTOS2NDTerminateProcess(&g_ProcessTable[i]);
        }
    }
    
    DeleteCriticalSection(&g_KernelLock);
    g_bKernelInitialized = FALSE;
    
    printf("[ntos2nd] Kernel shutdown\n");
    
    return STATUS_SUCCESS;
}

/* Create a new process */
NTOS2ND_PROCESS* NTOS2NDCreateProcess(const char* ProcessPath, BOOL bSandboxed) {
    NTOS2ND_PROCESS* Process = NTOS2NDFindFreeProcessSlot();
    if (!Process) {
        return NULL;
    }
    
    EnterCriticalSection(&g_KernelLock);
    
    Process->ProcessId = g_NextProcessId++;
    Process->ParentProcessId = GetCurrentProcessId();
    Process->State = ProcessStateCreated;
    Process->bIsSandboxed = bSandboxed;
    strncpy(Process->ProcessName, ProcessPath, NTOS2ND_MAX_PATH - 1);
    InitializeCriticalSection(&Process->Lock);
    
    if (bSandboxed) {
        /* Create pipes for IPC */
        SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
        if (!CreatePipe(&Process->PipeRead, &Process->PipeWrite, &sa, NTOS2ND_PIPE_BUFFER_SIZE)) {
            LeaveCriticalSection(&g_KernelLock);
            return NULL;
        }
        
        /* Create child process in suspended state */
        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi = { 0 };
        
        if (CreateProcessA(NULL, (LPSTR)ProcessPath, NULL, NULL, TRUE,
                          CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
            Process->ProcessHandle = pi.hProcess;
            
            /* Create worker thread for IPC */
            HANDLE hWorkerThread = CreateThread(NULL, 0, NTOS2NDWorkerThread, 
                                               Process, 0, NULL);
            if (hWorkerThread) {
                CloseHandle(hWorkerThread);
            }
            
            ResumeThread(pi.hThread);
            CloseHandle(pi.hThread);
            
            Process->State = ProcessStateRunning;
        } else {
            CloseHandle(Process->PipeRead);
            CloseHandle(Process->PipeWrite);
            LeaveCriticalSection(&g_KernelLock);
            return NULL;
        }
    }
    
    LeaveCriticalSection(&g_KernelLock);
    
    printf("[ntos2nd] Created process %u (%s)\n", Process->ProcessId, ProcessPath);
    
    return Process;
}

/* Terminate a process */
NTSTATUS NTOS2NDTerminateProcess(NTOS2ND_PROCESS* Process) {
    if (!Process || Process->Header.Signature != 0x50524F43) {
        return STATUS_INVALID_PARAMETER;
    }
    
    EnterCriticalSection(&Process->Lock);
    
    if (Process->ProcessHandle) {
        TerminateProcess(Process->ProcessHandle, 0);
        CloseHandle(Process->ProcessHandle);
        Process->ProcessHandle = NULL;
    }
    
    if (Process->PipeRead) {
        CloseHandle(Process->PipeRead);
        Process->PipeRead = NULL;
    }
    
    if (Process->PipeWrite) {
        CloseHandle(Process->PipeWrite);
        Process->PipeWrite = NULL;
    }
    
    Process->State = ProcessStateTerminated;
    
    LeaveCriticalSection(&Process->Lock);
    DeleteCriticalSection(&Process->Lock);
    
    Process->Header.Signature = 0;
    
    printf("[ntos2nd] Terminated process %u\n", Process->ProcessId);
    
    return STATUS_SUCCESS;
}

/* Wait for process termination */
NTSTATUS NTOS2NDWaitForProcess(NTOS2ND_PROCESS* Process, DWORD dwMilliseconds) {
    if (!Process || Process->Header.Signature != 0x50524F43) {
        return STATUS_INVALID_PARAMETER;
    }
    
    WaitForSingleObject(Process->ProcessHandle, dwMilliseconds);
    
    return STATUS_SUCCESS;
}

/* Create a thread */
NTOS2ND_THREAD* NTOS2NDCreateThread(NTOS2ND_PROCESS* Process, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter) {
    NTOS2ND_THREAD* Thread = NTOS2NDFindFreeThreadSlot();
    if (!Thread) {
        return NULL;
    }
    
    Thread->ThreadId = g_NextThreadId++;
    Thread->ProcessId = Process->ProcessId;
    Thread->State = ThreadStateRunning;
    
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)lpStartAddress, 
                                  lpParameter, 0, &Thread->ThreadIdWindows);
    if (hThread) {
        Thread->ThreadHandle = hThread;
        return Thread;
    }
    
    Thread->Header.Signature = 0;
    return NULL;
}

/* Terminate a thread */
NTSTATUS NTOS2NDTerminateThread(NTOS2ND_THREAD* Thread) {
    if (!Thread || Thread->Header.Signature != 0x54485244) {
        return STATUS_INVALID_PARAMETER;
    }
    
    if (Thread->ThreadHandle) {
        TerminateThread(Thread->ThreadHandle, 0);
        CloseHandle(Thread->ThreadHandle);
    }
    
    Thread->State = ThreadStateTerminated;
    Thread->Header.Signature = 0;
    
    return STATUS_SUCCESS;
}

/* Handle syscall from child process */
NTSTATUS NTOS2NDHandleSyscall(NTOS2ND_PROCESS* Process, uint32_t SyscallNumber, 
                               uint64_t* Parameters, uint32_t ParameterCount, uint64_t* Result) {
    NTSTATUS status = STATUS_SUCCESS;
    
    switch (SyscallNumber) {
        case NTOS2ND_SYS_OPEN_PROCESS: {
            DWORD dwDesiredAccess = (DWORD)Parameters[0];
            BOOL bInheritHandle = (BOOL)Parameters[1];
            DWORD dwProcessId = (DWORD)Parameters[2];
            
            HANDLE hProcess = OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
            *Result = (uint64_t)(uintptr_t)hProcess;
            status = hProcess ? STATUS_SUCCESS : GetLastError();
            break;
        }
        
        case NTOS2ND_SYS_CLOSE_HANDLE: {
            HANDLE Handle = (HANDLE)(uintptr_t)Parameters[0];
            *Result = CloseHandle(Handle) ? 1 : 0;
            status = STATUS_SUCCESS;
            break;
        }
        
        case NTOS2ND_SYS_READ_FILE: {
            HANDLE hFile = (HANDLE)(uintptr_t)Parameters[0];
            LPVOID lpBuffer = (LPVOID)(uintptr_t)Parameters[1];
            DWORD nNumberOfBytesToRead = (DWORD)Parameters[2];
            
            DWORD lpNumberOfBytesRead = 0;
            BOOL bResult = ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, &lpNumberOfBytesRead, NULL);
            *Result = bResult ? (uint64_t)lpNumberOfBytesRead : 0;
            status = bResult ? STATUS_SUCCESS : GetLastError();
            break;
        }
        
        case NTOS2ND_SYS_WRITE_FILE: {
            HANDLE hFile = (HANDLE)(uintptr_t)Parameters[0];
            LPCVOID lpBuffer = (LPCVOID)(uintptr_t)Parameters[1];
            DWORD nNumberOfBytesToWrite = (DWORD)Parameters[2];
            
            DWORD lpNumberOfBytesWritten = 0;
            BOOL bResult = WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, &lpNumberOfBytesWritten, NULL);
            *Result = bResult ? (uint64_t)lpNumberOfBytesWritten : 0;
            status = bResult ? STATUS_SUCCESS : GetLastError();
            break;
        }
        
        case NTOS2ND_SYS_CREATE_FILE: {
            LPCSTR lpFileName = (LPCSTR)(uintptr_t)Parameters[0];
            DWORD dwDesiredAccess = (DWORD)Parameters[1];
            DWORD dwShareMode = (DWORD)Parameters[2];
            LPSECURITY_ATTRIBUTES lpSecurityAttributes = (LPSECURITY_ATTRIBUTES)(uintptr_t)Parameters[3];
            DWORD dwCreationDisposition = (DWORD)Parameters[4];
            
            HANDLE hFile = CreateFileA(lpFileName, dwDesiredAccess, dwShareMode,
                                       lpSecurityAttributes, dwCreationDisposition, 0, NULL);
            *Result = (uint64_t)(uintptr_t)hFile;
            status = hFile != INVALID_HANDLE_VALUE ? STATUS_SUCCESS : GetLastError();
            break;
        }
        
        default:
            /* Unknown syscall - forward to host */
            *Result = 0;
            status = STATUS_NOT_IMPLEMENTED;
            break;
    }
    
    return status;
}

/* Send IPC message */
NTSTATUS NTOS2NDSendIPCMessage(NTOS2ND_PROCESS* Process, NTOS2ND_IPC_MESSAGE* Message) {
    DWORD dwBytesWritten;
    BOOL bResult = WriteFile(Process->PipeWrite, Message, sizeof(NTOS2ND_IPC_MESSAGE), &dwBytesWritten, NULL);
    return bResult ? STATUS_SUCCESS : GetLastError();
}

/* Receive IPC message */
NTSTATUS NTOS2NDReceiveIPCMessage(NTOS2ND_PROCESS* Process, NTOS2ND_IPC_MESSAGE* Message) {
    DWORD dwBytesRead;
    BOOL bResult = ReadFile(Process->PipeRead, Message, sizeof(NTOS2ND_IPC_MESSAGE), &dwBytesRead, NULL);
    return bResult ? STATUS_SUCCESS : GetLastError();
}

/* Register API implementation */
NTSTATUS NTOS2NDRegisterAPI(const char* ApiName, void* Implementation, uint32_t BridgeType) {
    if (g_ApiBridgeCount >= NTOS2ND_MAX_API_BRIDGES) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    NTOS2ND_API_BRIDGE* Bridge = &g_ApiBridges[g_ApiBridgeCount];
    Bridge->ApiNumber = g_ApiBridgeCount;
    Bridge->ApiName = ApiName;
    Bridge->BridgeType = BridgeType;
    Bridge->Implementation = Implementation;
    
    g_ApiBridgeCount++;
    
    return STATUS_SUCCESS;
}

/* Get API implementation */
void* NTOS2NDGetAPIImplementation(const char* ApiName) {
    for (uint32_t i = 0; i < g_ApiBridgeCount; i++) {
        if (strcmp(g_ApiBridges[i].ApiName, ApiName) == 0) {
            return g_ApiBridges[i].Implementation;
        }
    }
    return NULL;
}
/*
    ntos2nd - Windows NT Syscall Interface Implementation
    
    (C) 2025 OakyMacintosh
*/

#include "syscall.h"

/* Compatibility definitions for mingw-w64 (missing NT types) */
#ifndef UNICODE_STRING_DEFINED
#define UNICODE_STRING_DEFINED
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;
#endif

#ifndef OBJECT_ATTRIBUTES_DEFINED
#define OBJECT_ATTRIBUTES_DEFINED
typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;
    PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;
#endif

#ifndef CLIENT_ID_DEFINED
#define CLIENT_ID_DEFINED
typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;
#endif

#ifndef IO_STATUS_BLOCK_DEFINED
#define IO_STATUS_BLOCK_DEFINED
typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        DWORD Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;
#endif

/* Handle table */
static NTOS2ND_HANDLE_ENTRY g_HandleTable[8192];
static uint32_t g_NextHandle = 0x1000;

/* Handle management */
uint32_t NTOS2NDCreateHandle(uint32_t Type, void* Pointer) {
    for (int i = 0; i < 8192; i++) {
        if (g_HandleTable[i].Handle == 0) {
            uint32_t NewHandle = g_NextHandle++;
            g_HandleTable[i].Handle = NewHandle;
            g_HandleTable[i].Type = Type;
            g_HandleTable[i].Pointer = Pointer;
            g_HandleTable[i].Win32Handle = (HANDLE)(uintptr_t)Pointer;
            return NewHandle;
        }
    }
    return 0;
}

void* NTOS2NDGetHandlePointer(uint32_t Handle) {
    for (int i = 0; i < 8192; i++) {
        if (g_HandleTable[i].Handle == Handle) {
            return g_HandleTable[i].Pointer;
        }
    }
    return NULL;
}

void NTOS2NDCloseHandle(uint32_t Handle) {
    for (int i = 0; i < 8192; i++) {
        if (g_HandleTable[i].Handle == Handle) {
            memset(&g_HandleTable[i], 0, sizeof(NTOS2ND_HANDLE_ENTRY));
            break;
        }
    }
}

BOOL NTOS2NDIsValidHandle(uint32_t Handle) {
    for (int i = 0; i < 8192; i++) {
        if (g_HandleTable[i].Handle == Handle) {
            return TRUE;
        }
    }
    return FALSE;
}

/* Syscall dispatcher - main entry point */
NTSTATUS NTOS2NDDispatchSyscall(NTOS2ND_SYSCALL_CONTEXT* Context) {
    NTSTATUS status = STATUS_NOT_IMPLEMENTED;
    
    switch (Context->SyscallNumber) {
        case SYSCALL_NtClose:
            status = NTOS2NDSysNtClose((uint32_t)Context->Parameters[0]);
            break;
            
        case SYSCALL_NtCreateFile:
            status = NTOS2NDSysNtCreateFile(
                (uint32_t*)Context->Parameters[0],
                (uint32_t)Context->Parameters[1],
                Context->Parameters[2],
                Context->Parameters[3],
                Context->Parameters[4],
                (uint32_t)Context->Parameters[5],
                (uint32_t)Context->Parameters[6],
                (uint32_t)Context->Parameters[7],
                (uint32_t)Context->Parameters[8],
                Context->Parameters[9],
                (uint32_t)Context->Parameters[10]
            );
            break;
            
        case SYSCALL_NtOpenProcess:
            status = NTOS2NDSysNtOpenProcess(
                (uint32_t*)Context->Parameters[0],
                (uint32_t)Context->Parameters[1],
                Context->Parameters[2],
                Context->Parameters[3]
            );
            break;
            
        case SYSCALL_NtReadFile:
            status = NTOS2NDSysNtReadFile(
                (uint32_t)Context->Parameters[0],
                (uint32_t)Context->Parameters[1],
                Context->Parameters[2],
                Context->Parameters[3],
                Context->Parameters[4],
                Context->Parameters[5],
                (uint32_t)Context->Parameters[6],
                Context->Parameters[7],
                Context->Parameters[8]
            );
            break;
            
        case SYSCALL_NtWriteFile:
            status = NTOS2NDSysNtWriteFile(
                (uint32_t)Context->Parameters[0],
                (uint32_t)Context->Parameters[1],
                Context->Parameters[2],
                Context->Parameters[3],
                Context->Parameters[4],
                Context->Parameters[5],
                (uint32_t)Context->Parameters[6],
                Context->Parameters[7],
                Context->Parameters[8]
            );
            break;
            
        case SYSCALL_NtAllocateVirtualMemory:
            status = NTOS2NDSysNtAllocateVirtualMemory(
                (uint32_t)Context->Parameters[0],
                (uint64_t*)Context->Parameters[1],
                Context->Parameters[2],
                (uint64_t*)Context->Parameters[3],
                (uint32_t)Context->Parameters[4],
                (uint32_t)Context->Parameters[5]
            );
            break;
            
        case SYSCALL_NtCreateThread:
            status = NTOS2NDSysNtCreateThread(
                (uint32_t*)Context->Parameters[0],
                (uint32_t)Context->Parameters[1],
                Context->Parameters[2],
                (uint32_t)Context->Parameters[3],
                Context->Parameters[4],
                Context->Parameters[5],
                Context->Parameters[6],
                (uint32_t)Context->Parameters[7]
            );
            break;
            
        case SYSCALL_NtTerminateProcess:
            status = NTOS2NDSysNtTerminateProcess(
                (uint32_t)Context->Parameters[0],
                (int)Context->Parameters[1]
            );
            break;
            
        case SYSCALL_NtWaitForSingleObject:
            status = NTOS2NDSysNtWaitForSingleObject(
                (uint32_t)Context->Parameters[0],
                (uint32_t)Context->Parameters[1],
                Context->Parameters[2]
            );
            break;
            
        case NTOS2ND_SYSCALL_UNKNOWN_API:
            /* Forward unknown/undocumented API to host */
            status = NTOS2NDForwardToHost(
                (uint32_t)Context->Parameters[0],
                (uint64_t*)Context->Parameters[1],
                &Context->Result
            );
            break;
            
        default:
            /* Unknown syscall - forward to native implementation */
            status = NTOS2NDForwardToHost(Context->SyscallNumber, 
                                         Context->Parameters, &Context->Result);
            break;
    }
    
    Context->Status = status;
    return status;
}

/* NtClose implementation */
NTSTATUS NTOS2NDSysNtClose(uint32_t Handle) {
    if (!NTOS2NDIsValidHandle(Handle)) {
        return STATUS_INVALID_HANDLE;
    }
    
    void* ptr = NTOS2NDGetHandlePointer(Handle);
    if (ptr) {
        /* If it's a Windows handle, close it */
        HANDLE h = (HANDLE)(uintptr_t)ptr;
        if (h && h != INVALID_HANDLE_VALUE) {
            CloseHandle(h);
        }
    }
    
    NTOS2NDCloseHandle(Handle);
    return STATUS_SUCCESS;
}

/* NtCreateFile implementation */
NTSTATUS NTOS2NDSysNtCreateFile(
    uint32_t* FileHandle,
    uint32_t DesiredAccess,
    uint64_t ObjectAttributes,
    uint64_t IoStatusBlock,
    uint64_t AllocationSize,
    uint32_t FileAttributes,
    uint32_t ShareAccess,
    uint32_t CreateDisposition,
    uint32_t CreateOptions,
    uint64_t EaBuffer,
    uint32_t EaLength
) {
    /* Extract filename from ObjectAttributes */
    UNICODE_STRING* fileName = (UNICODE_STRING*)(ObjectAttributes + sizeof(OBJECT_ATTRIBUTES));
    char AnsiName[260] = {0};
    WideCharToMultiByte(CP_UTF8, 0, fileName->Buffer, fileName->Length / 2, AnsiName, 260, NULL, NULL);
    
    HANDLE hFile = CreateFileA(AnsiName, DesiredAccess, ShareAccess,
                              NULL, CreateDisposition, FileAttributes, NULL);
    
    if (hFile == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }
    
    *FileHandle = NTOS2NDCreateHandle(NTOS2ND_HANDLE_TYPE_FILE, (void*)(uintptr_t)hFile);
    
    /* Set IO status */
    if (IoStatusBlock) {
        PIO_STATUS_BLOCK statusBlock = (PIO_STATUS_BLOCK)(uintptr_t)IoStatusBlock;
        statusBlock->Status = STATUS_SUCCESS;
        statusBlock->Information = 0;
    }
    
    return STATUS_SUCCESS;
}

/* NtOpenProcess implementation */
NTSTATUS NTOS2NDSysNtOpenProcess(
    uint32_t* ProcessHandle,
    uint32_t DesiredAccess,
    uint64_t ObjectAttributes,
    uint64_t ClientId
) {
    CLIENT_ID* clientId = (CLIENT_ID*)(uintptr_t)ClientId;
    
    HANDLE hProcess = OpenProcess(DesiredAccess, FALSE, (DWORD)(uintptr_t)clientId->UniqueProcess);
    
    if (!hProcess) {
        return GetLastError();
    }
    
    *ProcessHandle = NTOS2NDCreateHandle(NTOS2ND_HANDLE_TYPE_PROCESS, (void*)(uintptr_t)hProcess);
    return STATUS_SUCCESS;
}

/* NtReadFile implementation */
NTSTATUS NTOS2NDSysNtReadFile(
    uint32_t FileHandle,
    uint32_t Event,
    uint64_t ApcRoutine,
    uint64_t ApcContext,
    uint64_t IoStatusBlock,
    uint64_t Buffer,
    uint32_t Length,
    uint64_t ByteOffset,
    uint64_t Key
) {
    HANDLE hFile = (HANDLE)(uintptr_t)NTOS2NDGetHandlePointer(FileHandle);
    if (!hFile) {
        return STATUS_INVALID_HANDLE;
    }
    
    LARGE_INTEGER offset;
    if (ByteOffset) {
        offset = *(LARGE_INTEGER*)(uintptr_t)ByteOffset;
    } else {
        offset.QuadPart = 0;
    }
    
    OVERLAPPED overlapped = {0};
    overlapped.Offset = offset.LowPart;
    overlapped.OffsetHigh = offset.HighPart;
    
    DWORD bytesRead = 0;
    BOOL result = ReadFile(hFile, (LPVOID)(uintptr_t)Buffer, Length, &bytesRead, &overlapped);
    
    if (IoStatusBlock) {
        PIO_STATUS_BLOCK statusBlock = (PIO_STATUS_BLOCK)(uintptr_t)IoStatusBlock;
        statusBlock->Status = result ? STATUS_SUCCESS : GetLastError();
        statusBlock->Information = bytesRead;
    }
    
    return result ? STATUS_SUCCESS : GetLastError();
}

/* NtWriteFile implementation */
NTSTATUS NTOS2NDSysNtWriteFile(
    uint32_t FileHandle,
    uint32_t Event,
    uint64_t ApcRoutine,
    uint64_t ApcContext,
    uint64_t IoStatusBlock,
    uint64_t Buffer,
    uint32_t Length,
    uint64_t ByteOffset,
    uint64_t Key
) {
    HANDLE hFile = (HANDLE)(uintptr_t)NTOS2NDGetHandlePointer(FileHandle);
    if (!hFile) {
        return STATUS_INVALID_HANDLE;
    }
    
    LARGE_INTEGER offset;
    if (ByteOffset) {
        offset = *(LARGE_INTEGER*)(uintptr_t)ByteOffset;
    } else {
        offset.QuadPart = 0;
    }
    
    OVERLAPPED overlapped = {0};
    overlapped.Offset = offset.LowPart;
    overlapped.OffsetHigh = offset.HighPart;
    
    DWORD bytesWritten = 0;
    BOOL result = WriteFile(hFile, (LPCVOID)(uintptr_t)Buffer, Length, &bytesWritten, &overlapped);
    
    if (IoStatusBlock) {
        PIO_STATUS_BLOCK statusBlock = (PIO_STATUS_BLOCK)(uintptr_t)IoStatusBlock;
        statusBlock->Status = result ? STATUS_SUCCESS : GetLastError();
        statusBlock->Information = bytesWritten;
    }
    
    return result ? STATUS_SUCCESS : GetLastError();
}

/* NtAllocateVirtualMemory implementation */
NTSTATUS NTOS2NDSysNtAllocateVirtualMemory(
    uint32_t ProcessHandle,
    uint64_t* BaseAddress,
    uint64_t ZeroBits,
    uint64_t* RegionSize,
    uint32_t AllocationType,
    uint32_t Protect
) {
    SIZE_T size = (SIZE_T)*RegionSize;
    
    void* addr = VirtualAlloc(*BaseAddress ? (LPVOID)(uintptr_t)*BaseAddress : NULL,
                              size, AllocationType, Protect);
    
    if (!addr) {
        return GetLastError();
    }
    
    *BaseAddress = (uint64_t)(uintptr_t)addr;
    *RegionSize = (uint64_t)size;
    
    return STATUS_SUCCESS;
}

/* NtCreateThread implementation */
NTSTATUS NTOS2NDSysNtCreateThread(
    uint32_t* ThreadHandle,
    uint32_t DesiredAccess,
    uint64_t ObjectAttributes,
    uint32_t ProcessHandle,
    uint64_t ClientId,
    uint64_t ThreadContext,
    uint64_t InitialTeb,
    uint32_t CreateSuspended
) {
    (void)DesiredAccess;
    (void)ObjectAttributes;
    (void)InitialTeb;
    
    NTOS2ND_PROCESS* Process = (NTOS2ND_PROCESS*)(uintptr_t)NTOS2NDGetHandlePointer(ProcessHandle);
    if (!Process) {
        return STATUS_INVALID_PARAMETER;
    }
    
    /* For now, create thread in current process context */
    DWORD threadId;
    /* Extract start address from context - simplified */
    void* startAddr = (void*)(uintptr_t)ThreadContext;
    
    HANDLE hThread = CreateThread(NULL, 0, 
        (LPTHREAD_START_ROUTINE)startAddr,
        NULL,
        CreateSuspended ? CREATE_SUSPENDED : 0,
        &threadId);
    
    if (!hThread) {
        return GetLastError();
    }
    
    *ThreadHandle = NTOS2NDCreateHandle(NTOS2ND_HANDLE_TYPE_THREAD, (void*)(uintptr_t)hThread);
    
    if (ClientId) {
        CLIENT_ID* clientId = (CLIENT_ID*)(uintptr_t)ClientId;
        clientId->UniqueProcess = (HANDLE)(uintptr_t)Process->ProcessId;
        clientId->UniqueThread = (HANDLE)(uintptr_t)threadId;
    }
    
    return STATUS_SUCCESS;
}

/* NtTerminateProcess implementation */
NTSTATUS NTOS2NDSysNtTerminateProcess(uint32_t ProcessHandle, int ExitStatus) {
    NTOS2ND_PROCESS* Process = (NTOS2ND_PROCESS*)(uintptr_t)NTOS2NDGetHandlePointer(ProcessHandle);
    
    if (Process) {
        return NTOS2NDTerminateProcess(Process);
    }
    
    /* If handle is 0, terminate current process */
    if (ProcessHandle == 0) {
        ExitProcess(ExitStatus);
    }
    
    return STATUS_SUCCESS;
}

/* NtWaitForSingleObject implementation */
NTSTATUS NTOS2NDSysNtWaitForSingleObject(
    uint32_t Handle,
    uint32_t WaitReason,
    uint64_t Timeout
) {
    void* ptr = NTOS2NDGetHandlePointer(Handle);
    if (!ptr) {
        return STATUS_INVALID_HANDLE;
    }
    
    HANDLE h = (HANDLE)(uintptr_t)ptr;
    DWORD dwTimeout = Timeout ? (DWORD)Timeout : INFINITE;
    
    DWORD result = WaitForSingleObject(h, dwTimeout);
    
    switch (result) {
        case WAIT_OBJECT_0:
            return STATUS_SUCCESS;
        case WAIT_TIMEOUT:
            return STATUS_TIMEOUT;
        case WAIT_FAILED:
            return GetLastError();
        default:
            return STATUS_SUCCESS;
    }
}

/* Forward unknown/undocumented API to host */
NTSTATUS NTOS2NDForwardToHost(uint32_t ApiNumber, uint64_t* Parameters, uint64_t* Result) {
    /* This function would use various techniques to call undocumented APIs:
     * 1. LKG (LdrGetProcedureAddress) for dynamic resolution
     * 2. Direct syscalls using ntdll!Nt* stubs
     * 3. Export forwarding from host ntoskrnl
     */
    
    printf("[ntos2nd] Forwarding unknown API %u to host\n", ApiNumber);
    
    /* For now, return not implemented */
    *Result = 0;
    return STATUS_NOT_IMPLEMENTED;
}
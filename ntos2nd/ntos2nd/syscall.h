/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Native LibreNT system component
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



/*
    ntos2nd - Windows NT Syscall Interface
    
    Mirrors Windows NT Native API syscalls (Nt*)
    
    (C) 2025 OakyMacintosh
*/

#ifndef NTOS2ND_SYSCALL_H
#define NTOS2ND_SYSCALL_H

#include "kernel.h"

/* Windows NT Syscall Numbers (x86/x64) */
#define SYSCALL_NtClose                    0x0000
#define SYSCALL_NtCreateFile               0x0001
#define SYSCALL_NtCreateSection            0x0002
#define SYSCALL_NtCreateProcess            0x0003
#define SYSCALL_NtCreateThread             0x0004
#define SYSCALL_NtDeleteValueKey           0x0005
#define SYSCALL_NtDeleteFile               0x0006
#define SYSCALL_NtEnumerateKey             0x0007
#define SYSCALL_NtEnumerateValueKey        0x0008
#define SYSCALL_NtFlushBuffersFile         0x0009
#define SYSCALL_NtFlushVirtualMemory       0x000A
#define SYSCALL_NtFreeVirtualMemory        0x000B
#define SYSCALL_NtGetValueKey            0x000C
#define SYSCALL_NtImpersonateThread        0x000D
#define SYSCALL_NtMapViewOfSection         0x000E
#define SYSCALL_NtNotifyChangeKey          0x000F
#define SYSCALL_NtOpenProcess              0x0010
#define SYSCALL_NtOpenThread               0x0011
#define SYSCALL_NtOpenSection              0x0012
#define SYSCALL_NtOpenFile                 0x0013
#define SYSCALL_NtQueryInformationFile     0x0014
#define SYSCALL_NtQueryInformationProcess  0x0015
#define SYSCALL_NtQueryInformationThread   0x0016
#define SYSCALL_NtQuerySystemInformation   0x0017
#define SYSCALL_NtQueueApcThread           0x0018
#define SYSCALL_NtReadFile                 0x0019
#define SYSCALL_NtReplaceKey               0x001A
#define SYSCALL_NtReplyPort                0x001B
#define SYSCALL_NtResumeThread             0x001C
#define SYSCALL_NtSaveKey                  0x001D
#define SYSCALL_NtSetContextThread         0x001E
#define SYSCALL_NtSetInformationFile       0x001F
#define SYSCALL_NtSetInformationProcess    0x0020
#define SYSCALL_NtSetInformationThread     0x0021
#define SYSCALL_NtSetValueKey              0x0022
#define SYSCALL_NtSuspendThread            0x0023
#define SYSCALL_NtTerminateProcess         0x0024
#define SYSCALL_NtTerminateThread          0x0025
#define SYSCALL_NtUnmapViewOfSection       0x0026
#define SYSCALL_NtWaitForSingleObject      0x0027
#define SYSCALL_NtWriteFile                0x0028
#define SYSCALL_NtYieldExecution           0x0029
#define SYSCALL_NtAllocateVirtualMemory  0x002A

/* Extended syscall numbers for internal kernel use */
#define NTOS2ND_SYSCALL_BASE               0x1000
#define NTOS2ND_SYSCALL_UNKNOWN_API        0x1001
#define NTOS2ND_SYSCALL_GET_API_ADDR       0x1002
#define NTOS2ND_SYSCALL_SET_SANDBOX_OPT    0x1003

/* Handle types */
#define NTOS2ND_HANDLE_TYPE_PROCESS        0x0001
#define NTOS2ND_HANDLE_TYPE_THREAD         0x0002
#define NTOS2ND_HANDLE_TYPE_FILE           0x0003
#define NTOS2ND_HANDLE_TYPE_SECTION        0x0004
#define NTOS2ND_HANDLE_TYPE_KEY            0x0005
#define NTOS2ND_HANDLE_TYPE_PORT           0x0006

/* Handle table entry */
typedef struct _NTOS2ND_HANDLE_ENTRY {
    uint32_t Handle;
    uint32_t Type;
    uint32_t ProcessId;
    void* Pointer;
    union {
        HANDLE Win32Handle;
        void* KernelObject;
    };
} NTOS2ND_HANDLE_ENTRY;

/* Syscall context */
typedef struct _NTOS2ND_SYSCALL_CONTEXT {
    NTOS2ND_PROCESS* Process;
    uint32_t SyscallNumber;
    uint64_t Parameters[16];
    uint32_t ParameterCount;
    uint64_t Result;
    NTSTATUS Status;
} NTOS2ND_SYSCALL_CONTEXT;

/* Syscall dispatcher */
NTSTATUS NTOS2NDDispatchSyscall(NTOS2ND_SYSCALL_CONTEXT* Context);

/* Individual syscall implementations */

/* NtClose - Close a handle */
NTSTATUS NTOS2NDSysNtClose(uint32_t Handle);

/* NtCreateFile - Create or open a file */
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
);

/* NtOpenProcess - Open a process object */
NTSTATUS NTOS2NDSysNtOpenProcess(
    uint32_t* ProcessHandle,
    uint32_t DesiredAccess,
    uint64_t ObjectAttributes,
    uint64_t ClientId
);

/* NtReadFile - Read from a file */
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
);

/* NtWriteFile - Write to a file */
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
);

/* NtAllocateVirtualMemory - Allocate virtual memory */
NTSTATUS NTOS2NDSysNtAllocateVirtualMemory(
    uint32_t ProcessHandle,
    uint64_t* BaseAddress,
    uint64_t ZeroBits,
    uint64_t* RegionSize,
    uint32_t AllocationType,
    uint32_t Protect
);

/* NtFreeVirtualMemory - Free virtual memory */
NTSTATUS NTOS2NDSysNtFreeVirtualMemory(
    uint32_t ProcessHandle,
    uint64_t* BaseAddress,
    uint64_t* RegionSize,
    uint32_t FreeType
);

/* NtCreateThread - Create a thread */
NTSTATUS NTOS2NDSysNtCreateThread(
    uint32_t* ThreadHandle,
    uint32_t DesiredAccess,
    uint64_t ObjectAttributes,
    uint32_t ProcessHandle,
    uint64_t ClientId,
    uint64_t ThreadContext,
    uint64_t InitialTeb,
    uint32_t CreateSuspended
);

/* NtTerminateProcess - Terminate a process */
NTSTATUS NTOS2NDSysNtTerminateProcess(uint32_t ProcessHandle, int ExitStatus);

/* NtWaitForSingleObject - Wait for object */
NTSTATUS NTOS2NDSysNtWaitForSingleObject(
    uint32_t Handle,
    uint32_t WaitReason,
    uint64_t Timeout
);

/* Handle management */
uint32_t NTOS2NDCreateHandle(uint32_t Type, void* Pointer);
void* NTOS2NDGetHandlePointer(uint32_t Handle);
void NTOS2NDCloseHandle(uint32_t Handle);
BOOL NTOS2NDIsValidHandle(uint32_t Handle);

/* Forward unknown/undocumented API to host */
NTSTATUS NTOS2NDForwardToHost(uint32_t ApiNumber, uint64_t* Parameters, uint64_t* Result);

#endif /* NTOS2ND_SYSCALL_H */
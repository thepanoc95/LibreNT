/*
    ntos2nd - NT Syscall Wrappers
    
    Direct wrappers for NT Native API syscalls.
    
    (C) 2025 OakyMacintosh
*/

#include "ntos2nd/syscall.h"
#include "ntos2nd/kernel.h"
#include "ntos2nd/apibridge.h"  /* For NT type definitions */
#include <windows.h>

/* Compatibility: ensure NT types are defined */
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

#ifndef IO_STATUS_BLOCK_DEFINED
#define IO_STATUS_BLOCK_DEFINED
typedef struct _IO_STATUS_BLOCK IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;
struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        DWORD Pointer;
    };
    ULONG_PTR Information;
};
#endif

/* NtClose wrapper */
NTSTATUS NTAPI NtCloseWrap(HANDLE Handle) {
    uint64_t params[1] = { (uint64_t)(uintptr_t)Handle };
    uint64_t result;
    NTOS2NDHandleSyscall(NULL, SYSCALL_NtClose, params, 1, &result);
    return (NTSTATUS)result;
}

/* NtCreateFile wrapper */
NTSTATUS NTAPI NtCreateFileWrap(
    PHANDLE FileHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    PVOID EaBuffer,
    ULONG EaLength
) {
    NTOS2ND_SYSCALL_CONTEXT ctx = {0};
    ctx.SyscallNumber = SYSCALL_NtCreateFile;
    
    *(FileHandle) = NULL;
    /* Real implementation would fill ctx.Parameters and call dispatcher */
    
    return STATUS_SUCCESS;
}
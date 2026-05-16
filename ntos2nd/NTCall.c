/*
    ntos2nd - NT Syscall Wrappers

    Direct wrappers for NT Native API syscalls.

    (C) 2025 OakyMacintosh
*/

#include "ntos2nd/syscall.h"
#include "ntos2nd/kernel.h"
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

NTSTATUS NTAPI NtCloseWrap(HANDLE Handle) {
    uint64_t params[1] = { (uint64_t)(uintptr_t)Handle };
    uint64_t result = 0;
    NTSTATUS status = NTOS2NDHandleSyscall(NULL, SYSCALL_NtClose, params, 1, &result);
    return NT_SUCCESS(status) ? (NTSTATUS)result : status;
}

NTSTATUS NTAPI NtOpenProcessWrap(
    PHANDLE ProcessHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PCLIENT_ID ClientId
) {
    if (!ProcessHandle || !ClientId) {
        return STATUS_INVALID_PARAMETER;
    }

    uint32_t kernelHandle = 0;
    uint64_t params[4] = {
        (uint64_t)(uintptr_t)&kernelHandle,
        (uint64_t)DesiredAccess,
        (uint64_t)(uintptr_t)ObjectAttributes,
        (uint64_t)(uintptr_t)ClientId,
    };

    uint64_t result = 0;
    NTSTATUS status = NTOS2NDHandleSyscall(NULL, SYSCALL_NtOpenProcess, params, 4, &result);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    *ProcessHandle = (HANDLE)(uintptr_t)NTOS2NDGetHandlePointer(kernelHandle);
    return (NTSTATUS)result;
}

NTSTATUS NTAPI NtReadFileWrap(
    HANDLE FileHandle,
    HANDLE Event,
    PVOID ApcRoutine,
    PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock,
    PVOID Buffer,
    ULONG Length,
    PLARGE_INTEGER ByteOffset,
    PULONG Key
) {
    uint64_t params[9] = {
        (uint64_t)(uintptr_t)FileHandle,
        (uint64_t)(uintptr_t)Event,
        (uint64_t)(uintptr_t)ApcRoutine,
        (uint64_t)(uintptr_t)ApcContext,
        (uint64_t)(uintptr_t)IoStatusBlock,
        (uint64_t)(uintptr_t)Buffer,
        (uint64_t)Length,
        (uint64_t)(uintptr_t)ByteOffset,
        (uint64_t)(uintptr_t)Key,
    };

    uint64_t result = 0;
    NTSTATUS status = NTOS2NDHandleSyscall(NULL, SYSCALL_NtReadFile, params, 9, &result);
    return NT_SUCCESS(status) ? (NTSTATUS)result : status;
}

NTSTATUS NTAPI NtWriteFileWrap(
    HANDLE FileHandle,
    HANDLE Event,
    PVOID ApcRoutine,
    PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock,
    const VOID* Buffer,
    ULONG Length,
    PLARGE_INTEGER ByteOffset,
    PULONG Key
) {
    uint64_t params[9] = {
        (uint64_t)(uintptr_t)FileHandle,
        (uint64_t)(uintptr_t)Event,
        (uint64_t)(uintptr_t)ApcRoutine,
        (uint64_t)(uintptr_t)ApcContext,
        (uint64_t)(uintptr_t)IoStatusBlock,
        (uint64_t)(uintptr_t)Buffer,
        (uint64_t)Length,
        (uint64_t)(uintptr_t)ByteOffset,
        (uint64_t)(uintptr_t)Key,
    };

    uint64_t result = 0;
    NTSTATUS status = NTOS2NDHandleSyscall(NULL, SYSCALL_NtWriteFile, params, 9, &result);
    return NT_SUCCESS(status) ? (NTSTATUS)result : status;
}

NTSTATUS NTAPI NtAllocateVirtualMemoryWrap(
    HANDLE ProcessHandle,
    PVOID* BaseAddress,
    ULONG_PTR ZeroBits,
    PSIZE_T RegionSize,
    ULONG AllocationType,
    ULONG Protect
) {
    if (!BaseAddress || !RegionSize) {
        return STATUS_INVALID_PARAMETER;
    }

    uint64_t baseAddress64 = (uint64_t)(uintptr_t)(*BaseAddress);
    uint64_t regionSize64 = (uint64_t)(*RegionSize);
    uint64_t params[6] = {
        (uint64_t)(uintptr_t)ProcessHandle,
        (uint64_t)(uintptr_t)&baseAddress64,
        (uint64_t)ZeroBits,
        (uint64_t)(uintptr_t)&regionSize64,
        (uint64_t)AllocationType,
        (uint64_t)Protect,
    };

    uint64_t result = 0;
    NTSTATUS status = NTOS2NDHandleSyscall(NULL, SYSCALL_NtAllocateVirtualMemory, params, 6, &result);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    *BaseAddress = (PVOID)(uintptr_t)baseAddress64;
    *RegionSize = (SIZE_T)regionSize64;
    return (NTSTATUS)result;
}

#ifndef NTOS2ND_NTCALL_H
#define NTOS2ND_NTCALL_H

#include <windows.h>
#include "ntos2nd/kernel.h"

NTSTATUS NTAPI NtCloseWrap(HANDLE Handle);
NTSTATUS NTAPI NtOpenProcessWrap(PHANDLE ProcessHandle, ACCESS_MASK DesiredAccess, void* ObjectAttributes, void* ClientId);
NTSTATUS NTAPI NtReadFileWrap(HANDLE FileHandle, HANDLE Event, PVOID ApcRoutine, PVOID ApcContext, void* IoStatusBlock, PVOID Buffer, ULONG Length, PLARGE_INTEGER ByteOffset, PULONG Key);
NTSTATUS NTAPI NtWriteFileWrap(HANDLE FileHandle, HANDLE Event, PVOID ApcRoutine, PVOID ApcContext, void* IoStatusBlock, const VOID* Buffer, ULONG Length, PLARGE_INTEGER ByteOffset, PULONG Key);
NTSTATUS NTAPI NtAllocateVirtualMemoryWrap(HANDLE ProcessHandle, PVOID* BaseAddress, ULONG_PTR ZeroBits, PSIZE_T RegionSize, ULONG AllocationType, ULONG Protect);

#endif

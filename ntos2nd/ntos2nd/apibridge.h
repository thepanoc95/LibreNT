/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Native LibreNT system component
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



/*
    ntos2nd - Windows API Bridge Layer
    
    Bridges Windows APIs (both documented and undocumented) to the kernel.
    
    (C) 2025 OakyMacintosh
*/

#ifndef NTOS2ND_API_BRIDGE_H
#define NTOS2ND_API_BRIDGE_H

#include "kernel.h"

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
typedef struct _IO_STATUS_BLOCK IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;
struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        DWORD Pointer;
    };
    ULONG_PTR Information;
};
#endif

/* API categories */
#define NTOS2ND_API_CATEGORY_KERNEL       1
#define NTOS2ND_API_CATEGORY_USER         2
#define NTOS2ND_API_CATEGORY_GDI           3
#define NTOS2ND_API_CATEGORY_ADVANCED      4

/* Forward declarations for NT types used in function pointer typedefs */
typedef struct _IO_STATUS_BLOCK IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;
typedef void (WINAPI* PIO_APC_ROUTINE)(PVOID, PIO_STATUS_BLOCK, ULONG);

/* API function types */
typedef DWORD (WINAPI* PFN_GetVersion)(void);
typedef BOOL (WINAPI* PFN_ReadFile)(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef BOOL (WINAPI* PFN_WriteFile)(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef HANDLE (WINAPI* PFN_CreateFileA)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef BOOL (WINAPI* PFN_CloseHandle)(HANDLE);
typedef DWORD (WINAPI* PFN_WaitForSingleObject)(HANDLE, DWORD);
typedef HANDLE (WINAPI* PFN_CreateThread)(LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD);
typedef BOOL (WINAPI* PFN_TerminateProcess)(HANDLE, UINT);
typedef void (WINAPI* PFN_ExitProcess)(UINT);
typedef LPVOID (WINAPI* PFN_VirtualAlloc)(LPVOID, SIZE_T, DWORD, DWORD);
typedef BOOL (WINAPI* PFN_VirtualFree)(LPVOID, SIZE_T, DWORD);

/* Undocumented API function types */
typedef NTSTATUS (WINAPI* PFN_NtClose)(HANDLE);
typedef NTSTATUS (WINAPI* PFN_NtCreateFile)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK,
                                            PLARGE_INTEGER, ULONG, ULONG, ULONG, ULONG, PVOID, ULONG);
typedef NTSTATUS (WINAPI* PFN_NtOpenProcess)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PCLIENT_ID);
typedef NTSTATUS (WINAPI* PFN_NtReadFile)(HANDLE, HANDLE, PIO_APC_ROUTINE, PVOID, PIO_STATUS_BLOCK,
                                          PVOID, ULONG, PLARGE_INTEGER, PULONG);
typedef NTSTATUS (WINAPI* PFN_NtWriteFile)(HANDLE, HANDLE, PIO_APC_ROUTINE, PVOID, PIO_STATUS_BLOCK,
                                           PVOID, ULONG, PLARGE_INTEGER, PULONG);
typedef NTSTATUS (WINAPI* PFN_NtAllocateVirtualMemory)(HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG);
typedef NTSTATUS (WINAPI* PFN_NtFreeVirtualMemory)(HANDLE, PVOID*, PSIZE_T, ULONG);

/* API bridge entry */
typedef struct _NTOS2ND_API_ENTRY {
    const char* Name;
    const char* DllName;
    void* FunctionAddress;
    uint32_t Category;
    uint32_t Flags;
    void* ForwardAddress;  /* For undocumented APIs, the real address in ntdll/ntoskrnl */
} NTOS2ND_API_ENTRY;

/* API database */
static NTOS2ND_API_ENTRY g_ApiDatabase[] = {
    /* Documented Win32 APIs */
    { "GetVersion", "kernel32.dll", NULL, NTOS2ND_API_CATEGORY_KERNEL, 0, NULL },
    { "ReadFile", "kernel32.dll", NULL, NTOS2ND_API_CATEGORY_KERNEL, 0, NULL },
    { "WriteFile", "kernel32.dll", NULL, NTOS2ND_API_CATEGORY_KERNEL, 0, NULL },
    { "CreateFileA", "kernel32.dll", NULL, NTOS2ND_API_CATEGORY_KERNEL, 0, NULL },
    { "CloseHandle", "kernel32.dll", NULL, NTOS2ND_API_CATEGORY_KERNEL, 0, NULL },
    { "WaitForSingleObject", "kernel32.dll", NULL, NTOS2ND_API_CATEGORY_KERNEL, 0, NULL },
    { "CreateThread", "kernel32.dll", NULL, NTOS2ND_API_CATEGORY_KERNEL, 0, NULL },
    { "TerminateProcess", "kernel32.dll", NULL, NTOS2ND_API_CATEGORY_KERNEL, 0, NULL },
    { "ExitProcess", "kernel32.dll", NULL, NTOS2ND_API_CATEGORY_KERNEL, 0, NULL },
    { "VirtualAlloc", "kernel32.dll", NULL, NTOS2ND_API_CATEGORY_KERNEL, 0, NULL },
    { "VirtualFree", "kernel32.dll", NULL, NTOS2ND_API_CATEGORY_KERNEL, 0, NULL },
    
    /* Undocumented NT APIs */
    { "NtClose", "ntdll.dll", NULL, NTOS2ND_API_CATEGORY_KERNEL, 1, NULL },
    { "NtCreateFile", "ntdll.dll", NULL, NTOS2ND_API_CATEGORY_KERNEL, 1, NULL },
    { "NtOpenProcess", "ntdll.dll", NULL, NTOS2ND_API_CATEGORY_KERNEL, 1, NULL },
    { "NtReadFile", "ntdll.dll", NULL, NTOS2ND_API_CATEGORY_KERNEL, 1, NULL },
    { "NtWriteFile", "ntdll.dll", NULL, NTOS2ND_API_CATEGORY_KERNEL, 1, NULL },
    { "NtAllocateVirtualMemory", "ntdll.dll", NULL, NTOS2ND_API_CATEGORY_KERNEL, 1, NULL },
    { "NtFreeVirtualMemory", "ntdll.dll", NULL, NTOS2ND_API_CATEGORY_KERNEL, 1, NULL },
    
    { NULL, NULL, NULL, 0, 0, NULL }
};

#define NTOS2ND_API_COUNT (sizeof(g_ApiDatabase) / sizeof(NTOS2ND_API_ENTRY) - 1)

/* API bridge functions */
NTSTATUS NTOS2NDInitializeAPIBridge(void);
void NTOS2NDShutdownAPIBridge(void);

/* Resolve API address */
void* NTOS2NDResolveAPI(const char* ApiName);
void* NTOS2NDResolveAPIByNumber(uint32_t ApiNumber);

/* Forward API calls */
NTSTATUS NTOS2NDForwardAPICall(const char* ApiName, uint64_t* Parameters, uint64_t* Result);
NTSTATUS NTOS2NDForwardAPICallByNumber(uint32_t ApiNumber, uint64_t* Parameters, uint64_t* Result);

/* Get API info */
const char* NTOS2NDGetAPIName(uint32_t ApiNumber);
const char* NTOS2NDGetAPIDll(uint32_t ApiNumber);
BOOL NTOS2NDIsAPIUndocumented(uint32_t ApiNumber);

/* Hot-patch support for undocumented APIs */
NTSTATUS NTOS2NDInstallHotPatch(const char* ApiName, void* NewImplementation);
NTSTATUS NTOS2NDRemoveHotPatch(const char* ApiName);

#endif /* NTOS2ND_API_BRIDGE_H */
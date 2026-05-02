/*
    ntos2nd - Undocumented API Passthrough Layer
    
    Forwards unknown/undocumented Windows APIs directly to the host system.
    
    (C) 2025 OakyMacintosh
*/

#ifndef NTOS2ND_UNDOCUMENTED_H
#define NTOS2ND_UNDOCUMENTED_H

#include "kernel.h"
#include "apibridge.h"

/* Undocumented API resolution methods */
#define UNDOC_RESOLVE_LDR              1  /* Use LdrGetProcedureAddress */
#define UNDOC_RESOLVE_EXPORT           2  /* Use Export table */
#define UNDOC_RESOLVE_DIRECT           3  /* Direct syscall */
#define UNDOC_RESOLVE_SYMBOL           4  /* Symbol resolution */

/* API signature types */
#define UNDOC_SIG_STDCALL              1
#define UNDOC_SIG_CDECL                2
#define UNDOC_SIG_FASTCALL             3
#define UNDOC_SIG_VECTORCALL           4

/* Undocumented API entry */
typedef struct _UNDOC_API_ENTRY {
    const char* Name;
    const char* DllName;
    const char* ForwardName;    /* Name in host ntdll/ntoskrnl */
    uint32_t ResolveMethod;
    uint32_t Signature;
    uint32_t ParamCount;
    void* HostAddress;          /* Resolved host address */
    void* KernelAddress;        /* Address in our kernel */
} UNDOC_API_ENTRY;

/* Known undocumented APIs */
static UNDOC_API_ENTRY g_UndocumentedAPIs[] = {
    /* Process and Thread */
    { "NtSetInformationProcess", "ntdll.dll", "NtSetInformationProcess", UNDOC_RESOLVE_LDR, UNDOC_SIG_STDCALL, 4, NULL, NULL },
    { "NtQueryInformationProcess", "ntdll.dll", "NtQueryInformationProcess", UNDOC_RESOLVE_LDR, UNDOC_SIG_STDCALL, 5, NULL, NULL },
    { "NtSetInformationThread", "ntdll.dll", "NtSetInformationThread", UNDOC_RESOLVE_LDR, UNDOC_SIG_STDCALL, 4, NULL, NULL },
    { "NtQueryInformationThread", "ntdll.dll", "NtQueryInformationThread", UNDOC_RESOLVE_LDR, UNDOC_SIG_STDCALL, 5, NULL, NULL },
    
    /* Security and Tokens */
    { "NtOpenProcessToken", "ntdll.dll", "NtOpenProcessToken", UNDOC_RESOLVE_LDR, UNDOC_SIG_STDCALL, 4, NULL, NULL },
    { "NtOpenThreadToken", "ntdll.dll", "NtOpenThreadToken", UNDOC_RESOLVE_LDR, UNDOC_SIG_STDCALL, 5, NULL, NULL },
    { "NtQueryInformationToken", "ntdll.dll", "NtQueryInformationToken", UNDOC_RESOLVE_LDR, UNDOC_SIG_STDCALL, 6, NULL, NULL },
    { "NtSetInformationToken", "ntdll.dll", "NtSetInformationToken", UNDOC_RESOLVE_LDR, UNDOC_SIG_STDCALL, 4, NULL, NULL },
    
    /* File System */
    { "NtCreateSection", "ntdll.dll", "NtCreateSection", UNDOC_RESOLVE_LDR, UNDOC_SIG_STDCALL, 10, NULL, NULL },
    { "NtMapViewOfSection", "ntdll.dll", "NtMapViewOfSection", UNDOC_RESOLVE_LDR, UNDOC_SIG_STDCALL, 10, NULL, NULL },
    { "NtUnmapViewOfSection", "ntdll.dll", "NtUnmapViewOfSection", UNDOC_RESOLVE_LDR, UNDOC_SIG_STDCALL, 2, NULL, NULL },
    
    /* Registry */
    { "NtCreateKey", "ntdll.dll", "NtCreateKey", UNDOC_RESOLVE_LDR, UNDOC_SIG_STDCALL, 10, NULL, NULL },
    { "NtDeleteKey", "ntdll.dll", "NtDeleteKey", UNDOC_RESOLVE_LDR, UNDOC_SIG_STDCALL, 1, NULL, NULL },
    { "NtOpenKey", "ntdll.dll", "NtOpenKey", UNDOC_RESOLVE_LDR, UNDOC_SIG_STDCALL, 5, NULL, NULL },
    
    /* Synchronization */
    { "NtCreateEvent", "ntdll.dll", "NtCreateEvent", UNDOC_RESOLVE_LDR, UNDOC_SIG_STDCALL, 6, NULL, NULL },
    { "NtSetEvent", "ntdll.dll", "NtSetEvent", UNDOC_RESOLVE_LDR, UNDOC_SIG_STDCALL, 2, NULL, NULL },
    { "NtPulseEvent", "ntdll.dll", "NtPulseEvent", UNDOC_RESOLVE_LDR, UNDOC_SIG_STDCALL, 2, NULL, NULL },
    
    /* Memory */
    { "NtProtectVirtualMemory", "ntdll.dll", "NtProtectVirtualMemory", UNDOC_RESOLVE_LDR, UNDOC_SIG_STDCALL, 5, NULL, NULL },
    { "NtWriteVirtualMemory", "ntdll.dll", "NtWriteVirtualMemory", UNDOC_RESOLVE_LDR, UNDOC_SIG_STDCALL, 5, NULL, NULL },
    { "NtReadVirtualMemory", "ntdll.dll", "NtReadVirtualMemory", UNDOC_RESOLVE_LDR, UNDOC_SIG_STDCALL, 5, NULL, NULL },
    
    { NULL, NULL, NULL, 0, 0, 0, NULL, NULL }
};

/* NTOS2ND API passthrough functions */
NTSTATUS NTOS2NDInitializeUndocumentedAPI(void);

/* Resolve undocumented API address */
void* NTOS2NDResolveUndocumentedAPI(const char* ApiName);
void* NTOS2NDResolveUndocumentedAPIEx(const char* ApiName, const char* DllName);

/* Forward unknown API to host */
NTSTATUS NTOS2NDForwardUnknownAPI(const char* ApiName, uint64_t* Parameters, uint64_t* Result);
NTSTATUS NTOS2NDForwardUnknownAPIByNumber(uint32_t ApiNumber, uint64_t* Parameters, uint64_t* Result);

/* Direct host syscall */
NTSTATUS NTOS2NDCallHostSyscall(uint32_t SyscallNumber, uint64_t* Parameters, uint64_t* Result);

/* Dynamic API resolver */
NTSTATUS NTOS2NDResolveDynamicAPI(const char* ModuleName, const char* ApiName, void** ApiAddress);

#endif /* NTOS2ND_UNDOCUMENTED_H */
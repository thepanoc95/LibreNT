/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Native LibreNT system component
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



/*
    ntos2nd - Windows API Bridge Layer Implementation
    
    (C) 2025 OakyMacintosh
*/

#include "apibridge.h"
#include "syscall.h"

static HMODULE g_hKernel32 = NULL;
static HMODULE g_hNtdll = NULL;

/* Initialize the API bridge */
NTSTATUS NTOS2NDInitializeAPIBridge(void) {
    /* Load required DLLs */
    g_hKernel32 = GetModuleHandleA("kernel32.dll");
    g_hNtdll = GetModuleHandleA("ntdll.dll");
    
    if (!g_hKernel32 || !g_hNtdll) {
        return STATUS_DLL_NOT_FOUND;
    }
    
    /* Resolve all API addresses */
    for (uint32_t i = 0; i < NTOS2ND_API_COUNT; i++) {
        NTOS2ND_API_ENTRY* entry = &g_ApiDatabase[i];
        
        /* Determine which module to use */
        HMODULE hModule = (entry->DllName[0] == 'n') ? g_hNtdll : g_hKernel32;
        
        entry->FunctionAddress = GetProcAddress(hModule, entry->Name);
        
        if (!entry->FunctionAddress && entry->Flags & 1) {
            /* For undocumented APIs, try LdrGetProcedureAddress */
            /* This would use undocumented ntdll functions */
            printf("[ntos2nd] Warning: Could not resolve %s\n", entry->Name);
        }
    }
    
    printf("[ntos2nd] API bridge initialized (%llu APIs)\n", (unsigned long long)NTOS2ND_API_COUNT);
    
    return STATUS_SUCCESS;
}

/* Shutdown the API bridge */
void NTOS2NDShutdownAPIBridge(void) {
    /* Clear function addresses */
    for (uint32_t i = 0; i < NTOS2ND_API_COUNT; i++) {
        g_ApiDatabase[i].FunctionAddress = NULL;
    }
    
    printf("[ntos2nd] API bridge shutdown\n");
}

/* Resolve API by name */
void* NTOS2NDResolveAPI(const char* ApiName) {
    for (uint32_t i = 0; i < NTOS2ND_API_COUNT; i++) {
        if (strcmp(g_ApiDatabase[i].Name, ApiName) == 0) {
            return g_ApiDatabase[i].FunctionAddress;
        }
    }
    return NULL;
}

/* Resolve API by number */
void* NTOS2NDResolveAPIByNumber(uint32_t ApiNumber) {
    if (ApiNumber >= NTOS2ND_API_COUNT) {
        return NULL;
    }
    return g_ApiDatabase[ApiNumber].FunctionAddress;
}

/* Forward API call */
NTSTATUS NTOS2NDForwardAPICall(const char* ApiName, uint64_t* Parameters, uint64_t* Result) {
    void* func = NTOS2NDResolveAPI(ApiName);
    if (!func) {
        return STATUS_NOT_FOUND;
    }
    
    /* Call through function pointer */
    /* This is a simplified version - real implementation would need proper calling convention */
    switch (ApiName[0]) {
        case 'G': /* GetVersion */
            *Result = (uint64_t)((PFN_GetVersion)func)();
            return STATUS_SUCCESS;
        case 'C': /* CreateFileA, CloseHandle, CreateThread */
            if (strcmp(ApiName, "CreateFileA") == 0) {
                *Result = (uint64_t)(uintptr_t)((PFN_CreateFileA)func)(
                    (LPCSTR)(uintptr_t)Parameters[0],
                    (DWORD)Parameters[1],
                    (DWORD)Parameters[2],
                    (LPSECURITY_ATTRIBUTES)(uintptr_t)Parameters[3],
                    (DWORD)Parameters[4],
                    (DWORD)Parameters[5],
                    (HANDLE)(uintptr_t)Parameters[6]
                );
            } else if (strcmp(ApiName, "CloseHandle") == 0) {
                *Result = (uint64_t)((PFN_CloseHandle)func)((HANDLE)(uintptr_t)Parameters[0]);
            } else if (strcmp(ApiName, "CreateThread") == 0) {
                DWORD threadId;
                *Result = (uint64_t)(uintptr_t)((PFN_CreateThread)func)(
                    (LPSECURITY_ATTRIBUTES)(uintptr_t)Parameters[0],
                    (SIZE_T)Parameters[1],
                    (LPTHREAD_START_ROUTINE)(uintptr_t)Parameters[2],
                    (LPVOID)(uintptr_t)Parameters[3],
                    (DWORD)Parameters[4],
                    &threadId
                );
            }
            return STATUS_SUCCESS;
        case 'R': /* ReadFile */
            {
                DWORD bytesRead = 0;
                *Result = (uint64_t)((PFN_ReadFile)func)(
                    (HANDLE)(uintptr_t)Parameters[0],
                    (LPVOID)(uintptr_t)Parameters[1],
                    (DWORD)Parameters[2],
                    &bytesRead,
                    (LPOVERLAPPED)(uintptr_t)Parameters[3]
                );
            }
            return STATUS_SUCCESS;
        case 'W': /* WriteFile, WaitForSingleObject */
            if (strcmp(ApiName, "WriteFile") == 0) {
                DWORD bytesWritten = 0;
                *Result = (uint64_t)((PFN_WriteFile)func)(
                    (HANDLE)(uintptr_t)Parameters[0],
                    (LPCVOID)(uintptr_t)Parameters[1],
                    (DWORD)Parameters[2],
                    &bytesWritten,
                    (LPOVERLAPPED)(uintptr_t)Parameters[3]
                );
            } else if (strcmp(ApiName, "WaitForSingleObject") == 0) {
                *Result = (uint64_t)((PFN_WaitForSingleObject)func)(
                    (HANDLE)(uintptr_t)Parameters[0],
                    (DWORD)Parameters[1]
                );
            }
            return STATUS_SUCCESS;
        case 'V': /* VirtualAlloc, VirtualFree */
            if (strcmp(ApiName, "VirtualAlloc") == 0) {
                *Result = (uint64_t)(uintptr_t)((PFN_VirtualAlloc)func)(
                    (LPVOID)(uintptr_t)Parameters[0],
                    (SIZE_T)Parameters[1],
                    (DWORD)Parameters[2],
                    (DWORD)Parameters[3]
                );
            } else if (strcmp(ApiName, "VirtualFree") == 0) {
                *Result = (uint64_t)((PFN_VirtualFree)func)(
                    (LPVOID)(uintptr_t)Parameters[0],
                    (SIZE_T)Parameters[1],
                    (DWORD)Parameters[2]
                );
            }
            return STATUS_SUCCESS;
        case 'T': /* TerminateProcess, Thread APIs */
            if (strcmp(ApiName, "TerminateProcess") == 0) {
                *Result = (uint64_t)((PFN_TerminateProcess)func)(
                    (HANDLE)(uintptr_t)Parameters[0],
                    (UINT)Parameters[1]
                );
            }
            return STATUS_SUCCESS;
        default:
            return STATUS_NOT_IMPLEMENTED;
    }
    
    return STATUS_SUCCESS;
}

/* Forward API call by number */
NTSTATUS NTOS2NDForwardAPICallByNumber(uint32_t ApiNumber, uint64_t* Parameters, uint64_t* Result) {
    if (ApiNumber >= NTOS2ND_API_COUNT) {
        return STATUS_INVALID_PARAMETER;
    }
    
    return NTOS2NDForwardAPICall(g_ApiDatabase[ApiNumber].Name, Parameters, Result);
}

/* Get API name */
const char* NTOS2NDGetAPIName(uint32_t ApiNumber) {
    if (ApiNumber >= NTOS2ND_API_COUNT) {
        return NULL;
    }
    return g_ApiDatabase[ApiNumber].Name;
}

/* Get API DLL */
const char* NTOS2NDGetAPIDll(uint32_t ApiNumber) {
    if (ApiNumber >= NTOS2ND_API_COUNT) {
        return NULL;
    }
    return g_ApiDatabase[ApiNumber].DllName;
}

/* Check if API is undocumented */
BOOL NTOS2NDIsAPIUndocumented(uint32_t ApiNumber) {
    if (ApiNumber >= NTOS2ND_API_COUNT) {
        return FALSE;
    }
    return (g_ApiDatabase[ApiNumber].Flags & 1) != 0;
}

/* Install hot patch for undocumented API */
NTSTATUS NTOS2NDInstallHotPatch(const char* ApiName, void* NewImplementation) {
    for (uint32_t i = 0; i < NTOS2ND_API_COUNT; i++) {
        if (strcmp(g_ApiDatabase[i].Name, ApiName) == 0) {
            g_ApiDatabase[i].ForwardAddress = NewImplementation;
            return STATUS_SUCCESS;
        }
    }
    return STATUS_NOT_FOUND;
}

/* Remove hot patch */
NTSTATUS NTOS2NDRemoveHotPatch(const char* ApiName) {
    for (uint32_t i = 0; i < NTOS2ND_API_COUNT; i++) {
        if (strcmp(g_ApiDatabase[i].Name, ApiName) == 0) {
            g_ApiDatabase[i].ForwardAddress = NULL;
            return STATUS_SUCCESS;
        }
    }
    return STATUS_NOT_FOUND;
}
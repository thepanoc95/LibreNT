/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Windows native helper component
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



/* 
    ntos2nd - Windows API Compatibility Layer
    
    Provides compatibility wrappers for Windows API calls.
    
    (C) 2025 OakyMacintosh
*/

#include <windows.h>
#include <stdio.h>

/* API version info */
DWORD WINAPI GetNtos2ndVersion(void) {
    return 0x00010000; /* v0.1.0 */
}

/* Debug output */
void WINAPI Ntos2ndDebugPrint(const char* msg) {
    OutputDebugStringA(msg);
    OutputDebugStringA("\n");
}
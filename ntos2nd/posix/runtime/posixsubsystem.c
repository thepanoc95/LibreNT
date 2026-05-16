/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     POSIX subsystem compatibility layer
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



#include <windows.h>
#include <stdint.h>
#include <string.h>
#include "../include/posix_abi.h"

__declspec(dllexport) int PosixInitialize(void) {
    return 0;
}

__declspec(dllexport) void PosixShutdown(void) {
}

__declspec(dllexport) int PosixTranslateNtStatusToErrno(int32_t ntstatus) {
    switch ((uint32_t)ntstatus) {
        case 0xC0000008: return 9;   /* EBADF */
        case 0xC0000022: return 13;  /* EACCES */
        case 0xC000000D: return 22;  /* EINVAL */
        case 0xC000000FL: return 2;  /* ENOENT */
        default: return 5;           /* EIO */
    }
}

__declspec(dllexport) int PosixSpawnProcess(const POSIX_CREATE_PROCESS_REQUEST* req, POSIX_CREATE_PROCESS_RESPONSE* resp) {
    if (!req || !resp) return 22;

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    char cmdLine[4096];
    cmdLine[0] = '\0';
    strncat(cmdLine, req->path, sizeof(cmdLine) - 1);
    if (req->argv_blob[0]) {
        strncat(cmdLine, " ", sizeof(cmdLine) - strlen(cmdLine) - 1);
        strncat(cmdLine, req->argv_blob, sizeof(cmdLine) - strlen(cmdLine) - 1);
    }

    BOOL ok = CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    if (!ok) {
        resp->pid = -1;
        resp->ntstatus = (int32_t)GetLastError();
        return PosixTranslateNtStatusToErrno(resp->ntstatus);
    }

    resp->pid = (int32_t)pi.dwProcessId;
    resp->ntstatus = 0;
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}

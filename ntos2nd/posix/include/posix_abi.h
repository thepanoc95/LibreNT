/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     POSIX subsystem compatibility layer
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



#ifndef NTOS2ND_POSIX_ABI_H
#define NTOS2ND_POSIX_ABI_H

#include <stdint.h>

#define POSIX_SUBSYSTEM_DEVICE_NAME "\\\\.\\\\LibreNTPosix"
#define POSIX_SUBSYSTEM_DOS_DEVICE "\\\\DosDevices\\\\LibreNTPosix"

#define POSIX_IOCTL_BASE 0x8000
#define IOCTL_POSIX_CREATE_PROCESS (POSIX_IOCTL_BASE + 1)
#define IOCTL_POSIX_WAIT_PROCESS   (POSIX_IOCTL_BASE + 2)
#define IOCTL_POSIX_TRANSLATE_ERRNO (POSIX_IOCTL_BASE + 3)

typedef struct _POSIX_CREATE_PROCESS_REQUEST {
    char path[260];
    char argv_blob[2048];
} POSIX_CREATE_PROCESS_REQUEST;

typedef struct _POSIX_CREATE_PROCESS_RESPONSE {
    int32_t pid;
    int32_t ntstatus;
} POSIX_CREATE_PROCESS_RESPONSE;

typedef struct _POSIX_ERRNO_TRANSLATE {
    int32_t ntstatus;
    int32_t errno_value;
} POSIX_ERRNO_TRANSLATE;

#endif

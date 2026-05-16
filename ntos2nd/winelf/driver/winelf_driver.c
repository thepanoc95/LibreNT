/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Windows driver or kernel support
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



/*
  NTDDK-based kernel-mode companion for WinELF.
  This driver exposes a simple device interface for requesting ELF execution
  on Windows NT environments.

  Build with the Windows Driver Kit as a standard kernel driver project.
*/

#ifdef _KERNEL_MODE
#include <ntddk.h>
#include "winelf_abi.h"

DRIVER_UNLOAD WinElfDriverUnload;
DRIVER_DISPATCH WinElfCreateClose;
DRIVER_DISPATCH WinElfDeviceControl;

VOID WinElfDriverUnload(PDRIVER_OBJECT DriverObject) {
    UNICODE_STRING dosName = RTL_CONSTANT_STRING(WINELF_DOS_DEVICE_NAME);
    IoDeleteSymbolicLink(&dosName);
    if (DriverObject->DeviceObject) {
        IoDeleteDevice(DriverObject->DeviceObject);
    }
}

NTSTATUS WinElfCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS WinElfDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);

    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
    ULONG info = 0;

    switch (stack->Parameters.DeviceIoControl.IoControlCode) {
        case IOCTL_WINELF_EXECUTE_ELF:
            // TODO: implement ELF execution support in kernel mode.
            status = STATUS_NOT_IMPLEMENTED;
            info = 0;
            break;

        case IOCTL_WINELF_QUERY_STATUS:
            // TODO: return the current ELF launch status.
            status = STATUS_NOT_IMPLEMENTED;
            info = 0;
            break;

        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
            info = 0;
            break;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = info;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(RegistryPath);

    UNICODE_STRING deviceName = RTL_CONSTANT_STRING(WINELF_DEVICE_NAME);
    UNICODE_STRING dosName = RTL_CONSTANT_STRING(WINELF_DOS_DEVICE_NAME);
    PDEVICE_OBJECT deviceObject = NULL;

    NTSTATUS status = IoCreateDevice(
        DriverObject,
        0,
        &deviceName,
        FILE_DEVICE_UNKNOWN,
        0,
        FALSE,
        &deviceObject
    );

    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = IoCreateSymbolicLink(&dosName, &deviceName);
    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(deviceObject);
        return status;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE] = WinElfCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = WinElfCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = WinElfDeviceControl;
    DriverObject->DriverUnload = WinElfDriverUnload;

    return STATUS_SUCCESS;
}
#endif

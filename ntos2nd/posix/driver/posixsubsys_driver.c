/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Windows driver or kernel support
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



/*
  Stub kernel-mode companion for LibreNT POSIX subsystem.
  This is intentionally minimal and avoids unsupported kernel patching.
*/

#ifdef _KERNEL_MODE
#include <ntddk.h>

DRIVER_UNLOAD PosixDrvUnload;
DRIVER_DISPATCH PosixDrvCreateClose;
DRIVER_DISPATCH PosixDrvDeviceControl;

VOID PosixDrvUnload(PDRIVER_OBJECT DriverObject) {
    UNICODE_STRING dosName = RTL_CONSTANT_STRING(L"\\DosDevices\\LibreNTPosix");
    IoDeleteSymbolicLink(&dosName);
    if (DriverObject->DeviceObject) {
        IoDeleteDevice(DriverObject->DeviceObject);
    }
}

NTSTATUS PosixDrvCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS PosixDrvDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(RegistryPath);

    UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\LibreNTPosix");
    UNICODE_STRING dosName = RTL_CONSTANT_STRING(L"\\DosDevices\\LibreNTPosix");
    PDEVICE_OBJECT DeviceObject = NULL;

    NTSTATUS status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
    if (!NT_SUCCESS(status)) return status;

    status = IoCreateSymbolicLink(&dosName, &devName);
    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(DeviceObject);
        return status;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE] = PosixDrvCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = PosixDrvCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = PosixDrvDeviceControl;
    DriverObject->DriverUnload = PosixDrvUnload;
    return STATUS_SUCCESS;
}
#endif

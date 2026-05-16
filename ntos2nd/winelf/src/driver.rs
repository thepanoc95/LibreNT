/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     WinELF ELF loader and runtime
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



//! Windows kernel driver integration for WinELF.

use crate::error::{WinElfError, WinElfResult};
use std::ffi::OsStr;
use std::os::windows::ffi::OsStrExt;
use std::path::Path;
use std::ptr;
use windows::core::PCWSTR;
use windows::Win32::Foundation::{CloseHandle, GetLastError, HANDLE, INVALID_HANDLE_VALUE};
use windows::Win32::Storage::FileSystem::{CreateFileW, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, FILE_SHARE_WRITE, GENERIC_READ, GENERIC_WRITE, OPEN_EXISTING};
use windows::Win32::System::IO::DeviceIoControl;
use windows::Win32::System::WindowsProgramming::FILE_DEVICE_UNKNOWN;

const DEVICE_PATH: &str = r"\\.\\WinElf";
const WINELF_PATH_MAX: usize = 260;

const WINELF_IOCTL_BASE: u32 = 0x800;
const IOCTL_WINELF_EXECUTE_ELF: u32 = CTL_CODE(WINELF_IOCTL_BASE + 0, 0, METHOD_BUFFERED, FILE_ANY_ACCESS);
const IOCTL_WINELF_QUERY_STATUS: u32 = CTL_CODE(WINELF_IOCTL_BASE + 1, 0, METHOD_BUFFERED, FILE_ANY_ACCESS);

const METHOD_BUFFERED: u32 = 0;
const FILE_ANY_ACCESS: u32 = 0;

const fn CTL_CODE(function: u32, method: u32, access: u32, device_type: u32) -> u32 {
    (device_type << 16) | (access << 14) | (function << 2) | method
}

#[repr(C)]
#[derive(Debug, Default, Clone, Copy)]
pub struct WinElfExecuteRequest {
    pub path: [u16; WINELF_PATH_MAX],
    pub flags: u32,
}

#[repr(C)]
#[derive(Debug, Default, Clone, Copy)]
pub struct WinElfExecuteResponse {
    pub process_id: u32,
    pub status: u32,
}

#[repr(C)]
#[derive(Debug, Default, Clone, Copy)]
pub struct WinElfStatusResponse {
    pub process_id: u32,
    pub last_status: u32,
}

/// Handle for the WinElf kernel device.
pub struct WinElfDriver {
    handle: HANDLE,
}

impl WinElfDriver {
    /// Open the WinElf kernel device.
    pub fn open() -> WinElfResult<Self> {
        let path = to_wide_null(DEVICE_PATH);
        let handle = unsafe {
            CreateFileW(
                PCWSTR(path.as_ptr()),
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                ptr::null(),
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                HANDLE(0),
            )
        };

        if handle == INVALID_HANDLE_VALUE {
            let err = unsafe { GetLastError().0 };
            return Err(WinElfError::DriverUnavailable(format!("failed to open {}: {}", DEVICE_PATH, err)));
        }

        Ok(WinElfDriver { handle })
    }

    /// Execute an ELF file through the kernel helper device.
    pub fn execute_elf(&self, path: &Path) -> WinElfResult<WinElfExecuteResponse> {
        let wide_path = to_wide_null(path.as_os_str());
        if wide_path.len() > WINELF_PATH_MAX {
            return Err(WinElfError::DriverIo(format!("path too long: {}", path.display())));
        }

        let mut request = WinElfExecuteRequest::default();
        request.path[..wide_path.len()].copy_from_slice(&wide_path);
        request.flags = 0;

        let mut response = WinElfExecuteResponse::default();
        let mut returned = 0u32;

        let success = unsafe {
            DeviceIoControl(
                self.handle,
                IOCTL_WINELF_EXECUTE_ELF,
                Some(&request as *const _ as *const _),
                std::mem::size_of::<WinElfExecuteRequest>() as u32,
                Some(&mut response as *mut _ as *mut _),
                std::mem::size_of::<WinElfExecuteResponse>() as u32,
                &mut returned,
                ptr::null_mut(),
            )
            .as_bool()
        };

        if !success {
            let err = unsafe { GetLastError().0 };
            return Err(WinElfError::DriverIo(format!("DeviceIoControl failed: {}", err)));
        }

        Ok(response)
    }

    /// Query the last known driver execution status.
    pub fn query_status(&self) -> WinElfResult<WinElfStatusResponse> {
        let mut response = WinElfStatusResponse::default();
        let mut returned = 0u32;

        let success = unsafe {
            DeviceIoControl(
                self.handle,
                IOCTL_WINELF_QUERY_STATUS,
                None,
                0,
                Some(&mut response as *mut _ as *mut _),
                std::mem::size_of::<WinElfStatusResponse>() as u32,
                &mut returned,
                ptr::null_mut(),
            )
            .as_bool()
        };

        if !success {
            let err = unsafe { GetLastError().0 };
            return Err(WinElfError::DriverIo(format!("DeviceIoControl failed: {}", err)));
        }

        Ok(response)
    }
}

impl Drop for WinElfDriver {
    fn drop(&mut self) {
        if self.handle != INVALID_HANDLE_VALUE {
            unsafe {
                let _ = CloseHandle(self.handle);
            }
        }
    }
}

fn to_wide_null(s: &OsStr) -> Vec<u16> {
    let mut wide: Vec<u16> = s.encode_wide().collect();
    wide.push(0);
    wide
}

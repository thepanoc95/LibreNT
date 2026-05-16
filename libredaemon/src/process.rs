/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     LibreNT daemon implementation
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



use windows_sys::Win32::System::Threading::*;
use windows_sys::Win32::Foundation::*;
use crate::logger;

pub struct ProcessHandle {
    pub process: HANDLE,
    pub thread: HANDLE,
}

impl ProcessHandle {
    pub fn new(process: HANDLE, thread: HANDLE) -> Self {
        Self { process, thread }
    }
}

impl Drop for ProcessHandle {
    fn drop(&mut self) {
        if self.process != 0 {
            unsafe {
                CloseHandle(self.process);
            }
        }
        if self.thread != 0 {
            unsafe {
                CloseHandle(self.thread);
            }
        }
    }
}

pub fn spawn_process(exe: &str, args: Option<&str>) -> Result<ProcessHandle, String> {
    // Build the command line
    let mut command_line = exe.to_string();
    if let Some(arguments) = args {
        command_line.push(' ');
        command_line.push_str(arguments);
    }
    
    let command_line_wide: Vec<u16> = command_line.encode_utf16().chain(Some(0)).collect();
    let mut cmd_line_ptr = command_line_wide.clone();
    
    let mut startup_info: STARTUPINFOW = unsafe { std::mem::zeroed() };
    startup_info.cb = std::mem::size_of::<STARTUPINFOW>() as u32;
    
    let mut process_info: PROCESS_INFORMATION = unsafe { std::mem::zeroed() };
    
    let creation_flags = CREATE_SUSPENDED;
    
    let result = unsafe {
        CreateProcessW(
            std::ptr::null(), // Using command line instead of app name
            cmd_line_ptr.as_mut_ptr(),
            std::ptr::null_mut(),
            std::ptr::null_mut(),
            FALSE,
            creation_flags,
            std::ptr::null_mut(),
            std::ptr::null(),
            &startup_info,
            &mut process_info,
        )
    };
    
    if result == 0 {
        return Err(format!("Failed to create process: {}", exe));
    }
    
    // Resume the main thread
    let resume_result = unsafe {
        ResumeThread(process_info.hThread)
    };
    
    if resume_result == u32::MAX {
        // Failed to resume, clean up
        unsafe {
            CloseHandle(process_info.hProcess);
            CloseHandle(process_info.hThread);
        }
        return Err(format!("Failed to resume thread for process: {}", exe));
    }
    
    logger::info(&format!("Spawned process: {} (with args: {:?})", exe, args));
    
    Ok(ProcessHandle::new(process_info.hProcess, process_info.hThread))
}

pub fn start_process(exe: &str) -> Result<ProcessHandle, String> {
    spawn_process(exe, None)
}
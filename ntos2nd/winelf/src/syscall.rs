//! Syscall handling for ELF programs

use crate::error::{WinElfError, WinElfResult};

/// Syscall handler
pub struct SyscallHandler {
    /// Number of syscalls handled
    count: usize,
}

impl SyscallHandler {
    /// Create a new syscall handler
    pub fn new() -> Self {
        SyscallHandler { count: 0 }
    }
    
    /// Handle a Linux syscall
    pub fn handle_syscall(&mut self, nr: u64, args: &[u64; 6]) -> WinElfResult<u64> {
        self.count += 1;
        
        // Map Linux syscalls to Windows equivalents
        match nr {
            // exit - 60
            60 => {
                let exit_code = args[0] as i32;
                log::info!("exit({})", exit_code);
                // Would terminate the process
                Ok(0)
            }
            // write - 1
            1 => {
                let fd = args[0] as i32;
                let buf = args[1] as *const u8;
                let count = args[2] as usize;
                // Would call Windows WriteFile
                Ok(count as u64)
            }
            // read - 0
            0 => {
                let fd = args[0] as i32;
                let buf = args[1] as *mut u8;
                let count = args[2] as usize;
                // Would call Windows ReadFile
                Ok(0)
            }
            // brk - 12
            12 => {
                // Would manage heap
                Ok(0)
            }
            // mmap - 9
            9 => {
                // Would call Windows VirtualAlloc
                Ok(0)
            }
            // Unknown syscall
            _ => {
                log::warn!("Unhandled syscall: {}", nr);
                Ok(0)
            }
        }
    }
}
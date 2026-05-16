/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     WinELF ELF loader and runtime
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



//! ELF loading and execution

use crate::elf::ElfExecutable;
use crate::error::{WinElfError, WinElfResult};
use crate::memory::MemoryManager;
use memmap2::{MmapMut, MmapOptions};
use std::fs::File;
use std::ptr;

/// ELF loader
pub struct ElfLoader {
    executable: ElfExecutable,
    memory: MemoryManager,
}

impl ElfLoader {
    /// Create a new ELF loader
    pub fn new(executable: ElfExecutable) -> WinElfResult<Self> {
        let memory = MemoryManager::new();
        
        Ok(ElfLoader {
            executable,
            memory,
        })
    }
    
    /// Load an ELF executable into memory
    pub fn load(&mut self) -> WinElfResult<()> {
        // Load each segment
        for segment in self.executable.loadable_segments() {
            self.load_segment(segment)?;
        }
        
        Ok(())
    }
    
    /// Load a single segment into memory
    fn load_segment(&mut self, segment: &crate::elf::ElfSegment) -> WinElfResult<()> {
        let vaddr = segment.p_vaddr;
        let memsz = segment.p_memsz;
        let filesz = segment.p_filesz;
        
        let file = File::open(&self.executable.path())
            .map_err(|_| WinElfError::FileNotFound(self.executable.path().to_string()))?;
        
        // Allocate memory
        let mmap = MmapOptions::new()
            .offset(segment.p_offset)
            .len(memsz as usize)
            .map(file)?;
        
        // Zero out BSS (memory beyond file content)
        if memsz > filesz {
            let bss_start = unsafe { mmap.as_ptr().add(filesz as usize) };
            unsafe { ptr::write_bytes(bss_start, 0, (memsz - filesz) as usize) };
        }
        
        self.memory.map_region(vaddr, mmap)?;
        
        Ok(())
    }
    
    /// Run the loaded executable
    pub fn run(&mut self) -> WinElfResult<i32> {
        let entry = self.executable.entry();
        log::info!("Starting execution at 0x{:x}", entry);
        
        // Execution would go here
        Ok(0)
    }
}
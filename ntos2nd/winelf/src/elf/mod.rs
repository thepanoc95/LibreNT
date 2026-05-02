//! ELF binary format support

use crate::error::{WinElfError, WinElfResult};
use goblin::elf::program_header::PT_LOAD;
use goblin::elf::Elf;
use std::fs::File;
use std::path::Path;

/// ELF executable representation
#[derive(Debug)]
pub struct ElfExecutable {
    /// Parsed ELF data
    inner: Elf,
    /// Path to the executable
    path: String,
    /// Entry point address
    entry: u64,
    /// Loadable segments
    segments: Vec<ElfSegment>,
}

/// ELF segment (program header entry)
#[derive(Debug, Clone)]
pub struct ElfSegment {
    /// Segment type
    pub p_type: u32,
    /// File offset
    pub p_offset: u64,
    /// Virtual address
    pub p_vaddr: u64,
    /// Physical address (usually same as vaddr)
    pub p_paddr: u64,
    /// Segment size in file
    pub p_filesz: u64,
    /// Segment size in memory
    pub p_memsz: u64,
    /// Segment flags
    pub p_flags: u32,
    /// Segment alignment
    pub p_align: u64,
    /// Raw segment data
    pub data: Vec<u8>,
}

impl ElfExecutable {
    /// Parse an ELF file
    pub fn parse<P: AsRef<Path>>(path: P) -> WinElfResult<Self> {
        let path_str = path.as_ref().to_string_lossy().to_string();
        let file = File::open(&path).map_err(|_| WinElfError::FileNotFound(path_str.clone()))?;
        
        let mut reader = std::io::BufReader::new(file);
        let inner = Elf::read(&mut reader)
            .map_err(|_| WinElfError::NotElf)?;
        
        // Validate ELF magic
        if !inner.is_elf() {
            return Err(WinElfError::InvalidMagic);
        }
        
        // Extract segments
        let mut segments = Vec::new();
        let data = std::fs::read(&path).unwrap_or_default();
        
        for ph in &inner.program_headers {
            let segment = ElfSegment {
                p_type: ph.p_type,
                p_offset: ph.p_offset,
                p_vaddr: ph.p_vaddr,
                p_paddr: ph.p_paddr,
                p_filesz: ph.p_filesz,
                p_memsz: ph.p_memsz,
                p_flags: ph.p_flags,
                p_align: ph.p_align,
                data: if ph.p_filesz > 0 {
                    let start = ph.p_offset as usize;
                    let end = (ph.p_offset + ph.p_filesz) as usize;
                    if end <= data.len() {
                        data[start..end].to_vec()
                    } else {
                        Vec::new()
                    }
                } else {
                    Vec::new()
                },
            };
            segments.push(segment);
        }
        
        Ok(ElfExecutable {
            inner,
            path: path_str,
            entry: inner.entry,
            segments,
        })
    }
    
    /// Get the entry point address
    pub fn entry(&self) -> u64 {
        self.entry
    }
    
    /// Get all loadable segments (PT_LOAD)
    pub fn loadable_segments(&self) -> impl Iterator<Item = &ElfSegment> {
        self.segments.iter().filter(|s| s.p_type == PT_LOAD)
    }
    
    /// Get the executable path
    pub fn path(&self) -> &str {
        &self.path
    }
    
    /// Check if this is a 64-bit ELF
    pub fn is_64bit(&self) -> bool {
        self.inner.is_64
    }
    
    /// Check if this is a 32-bit ELF
    pub fn is_32bit(&self) -> bool {
        !self.inner.is_64
    }
}
//! Memory management for ELF execution

use crate::error::{WinElfError, WinElfResult};
use memmap2::MmapMut;
use std::collections::BTreeMap;
use std::sync::Mutex;

/// Memory region mapping
#[derive(Debug)]
pub struct MemoryRegion {
    /// Starting virtual address
    pub start: u64,
    /// Ending virtual address
    pub end: u64,
    /// Memory protection flags
    pub prot: u32,
    /// The mapped memory
    pub mmap: Option<MmapMut>,
}

/// Memory manager for ELF processes
pub struct MemoryManager {
    /// Mapped regions
    regions: Mutex<BTreeMap<u64, MemoryRegion>>,
    /// Next available address for allocation
    next_heap: Mutex<u64>,
}

impl MemoryManager {
    /// Create a new memory manager
    pub fn new() -> Self {
        MemoryManager {
            regions: Mutex::new(BTreeMap::new()),
            next_heap: Mutex::new(0x40000000), // Start heap after typical ELF range
        }
    }
    
    /// Map a memory region
    pub fn map_region(&self, addr: u64, mmap: MmapMut) -> WinElfResult<()> {
        let mut regions = self.regions.lock().unwrap();
        
        let region = MemoryRegion {
            start: addr,
            end: addr + mmap.len() as u64,
            prot: 0, // TODO: Set protection
            mmap: Some(mmap),
        };
        
        regions.insert(addr, region);
        Ok(())
    }
    
    /// Allocate heap memory
    pub fn heap_alloc(&self, size: usize) -> WinElfResult<u64> {
        let mut next = self.next_heap.lock().unwrap();
        let addr = *next;
        *next += size as u64;
        
        Ok(addr)
    }
}
//! Error handling for WinELF

use thiserror::Error;

/// WinELF error type
#[derive(Debug, Error)]
pub enum WinElfError {
    /// Invalid ELF magic number
    #[error("Invalid ELF magic number")]
    InvalidMagic,
    
    /// Unsupported ELF class (32-bit vs 64-bit)
    #[error("Unsupported ELF class: {0}")]
    InvalidClass(u8),
    
    /// Unsupported ELF data encoding (endianness)
    #[error("Unsupported ELF data encoding: {0}")]
    InvalidData(u8),
    
    /// Unsupported ELF version
    #[error("Unsupported ELF version: {0}")]
    InvalidVersion(u8),
    
    /// Unsupported architecture
    #[error("Unsupported architecture: {0}")]
    UnsupportedArch(String),
    
    /// File not found
    #[error("File not found: {0}")]
    FileNotFound(String),
    
    /// I/O error
    #[error("I/O error: {0}")]
    Io(String),
    
    /// Invalid section/segment
    #[error("Invalid section/segment: {0}")]
    InvalidSection(String),
    
    /// Memory allocation failed
    #[error("Memory allocation failed")]
    MemoryAllocation,
    
    /// Relocation failed
    #[error("Relocation failed: {0}")]
    Relocation(String),
    
    /// Dynamic linking failed
    #[error("Dynamic linking failed: {0}")]
    DynamicLinking(String),
    
    /// Entry point not found
    #[error("Entry point not found")]
    NoEntryPoint,
    
    /// Interpreter error
    #[error("Interpreter error: {0}")]
    Interpreter(String),
    
    /// TLS initialization failed
    #[error("TLS initialization failed")]
    TlsInit,
    
    /// Not an ELF file
    #[error("Not an ELF file")]
    NotElf,
}

/// WinELF result type
pub type WinElfResult<T> = Result<T, WinElfError>;
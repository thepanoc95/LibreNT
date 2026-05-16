/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     WinELF ELF loader and runtime
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



//! WinELF - ELF Executable Support for Windows
//!
//! A modular ELF loader that enables running Linux ELF executables on Windows
//! through the ntos2nd kernel infrastructure.

#![warn(missing_docs)]
#![allow(dead_code)]

pub mod elf;
pub mod loader;
pub mod emulation;
pub mod error;
pub mod memory;
pub mod syscall;
#[cfg(target_os = "windows")]
pub mod driver;

pub use error::{WinElfError, WinElfResult};
pub use loader::ElfLoader;
pub use elf::ElfExecutable;
#[cfg(target_os = "windows")]
pub use driver::{WinElfDriver, WinElfExecuteResponse, WinElfStatusResponse};

/// WinELF version
pub const WINELF_VERSION: &str = env!("CARGO_PKG_VERSION");

/// Supported architectures
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Arch {
    /// x86_64 (AMD64)
    X86_64,
    /// x86 (IA-32)
    X86,
    /// ARM64
    Aarch64,
}

impl Arch {
    /// Detect host architecture
    pub fn host() -> Self {
        #[cfg(target_arch = "x86_64")]
        { Arch::X86_64 }
        #[cfg(target_arch = "x86")]
        { Arch::X86 }
        #[cfg(target_arch = "aarch64")]
        { Arch::Aarch64 }
        #[cfg(not(any(target_arch = "x86_64", target_arch = "x86", target_arch = "aarch64")))]
        { panic!("Unsupported architecture") }
    }
}
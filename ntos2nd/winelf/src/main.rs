/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     WinELF ELF loader and runtime
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



//! WinELF Loader - Main executable

use simplelog::*;
use std::env;
use std::path::Path;
use std::process;

use winelf::{ElfExecutable, ElfLoader, WinElfResult};
#[cfg(target_os = "windows")]
use winelf::WinElfDriver;

fn main() -> WinElfResult<()> {
    // Initialize logging
    TermLogger::new(
        LevelFilter::Info,
        Config::default(),
        TerminalMode::Mixed,
        ColorChoice::Auto,
    ).init();
    
    let args: Vec<String> = env::args().collect();
    
    if args.len() < 2 {
        eprintln!("Usage: {} <elf-file> [args...]", args[0]);
        process::exit(1);
    }
    
    let elf_path = Path::new(&args[1]);
    let _elf_args = &args[2..];
    
    println!("WinELF v{} - ELF Support for Windows", winelf::WINELF_VERSION);
    println!("Loading: {}", elf_path.display());
    
    #[cfg(target_os = "windows")]
    {
        match WinElfDriver::open() {
            Ok(driver) => match driver.execute_elf(elf_path) {
                Ok(response) => {
                    println!("WinElf driver launched ELF process: {}", response.process_id);
                    return Ok(());
                }
                Err(err) => {
                    log::warn!("WinElf driver execution failed, falling back to direct loader: {}", err);
                }
            },
            Err(err) => {
                log::info!("WinElf driver unavailable: {}", err);
            }
        }
    }

    // Parse the ELF
    let executable = ElfExecutable::parse(elf_path)?;
    
    println!("Entry point: 0x{:x}", executable.entry());
    println!("64-bit: {}", executable.is_64bit());
    println!("Loadable segments: {}", executable.loadable_segments().count());
    
    // Load the ELF
    let mut loader = ElfLoader::new(executable)?;
    loader.load()?;
    
    println!("ELF loaded successfully");
    
    // Run the executable
    let exit_code = loader.run()?;
    
    println!("Process exited with code: {}", exit_code);
    
    Ok(())
}
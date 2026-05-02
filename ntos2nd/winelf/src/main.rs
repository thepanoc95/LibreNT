//! WinELF Loader - Main executable

use simplelog::*;
use std::env;
use std::process;

use winelf::{ElfExecutable, ElfLoader, WinElfResult};

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
    
    let elf_path = &args[1];
    let _elf_args = &args[2..];
    
    println!("WinELF v{} - ELF Support for Windows", winelf::WINELF_VERSION);
    println!("Loading: {}", elf_path);
    
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
mod environment;
mod init;
mod logger;
mod process;
mod registry;

use std::env as std_env;
use std::path::Path;

fn main() {
    logger::info("LibreNT Daemon starting...");
    
    // Get the init file path from command line or use default
    let init_path = get_init_path();
    
    match init::parse_and_execute(&init_path) {
        Ok(()) => {
            logger::info("Init script executed successfully");
        }
        Err(e) => {
            logger::err(&format!("Failed to execute init script: {}", e));
            std::process::exit(1);
        }
    }
    
    logger::flush().ok();
}

fn get_init_path() -> std::path::PathBuf {
    let args: Vec<String> = std_env::args().collect();
    
    if args.len() > 1 {
        Path::new(&args[1]).to_path_buf()
    } else {
        // Default init file
        std::env::current_dir()
            .unwrap_or_else(|_| std::path::PathBuf::from("."))
            .join("init.rc")
    }
}
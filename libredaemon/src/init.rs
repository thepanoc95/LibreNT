/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     LibreNT daemon implementation
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



use std::fs::File;
use std::io::{BufRead, BufReader};
use std::path::Path;
use crate::logger;
use crate::environment;
use crate::registry;
use crate::process;

#[derive(Debug)]
pub enum Command {
    Start(String),
    Spawn(String, Vec<String>),
    SetEnv(String, String),
    RegSet(String, String),
    Log(String),
}

pub fn parse_line(line: &str) -> Option<Command> {
    let line = line.trim();
    
    // Skip empty lines and comments
    if line.is_empty() || line.starts_with('#') {
        return None;
    }
    
    let parts: Vec<&str> = line.splitn(2, ' ').collect();
    if parts.is_empty() {
        return None;
    }
    
    let command = parts[0].to_lowercase();
    
    match command.as_str() {
        "start" => {
            if parts.len() < 2 {
                logger::err("Invalid 'start' command: missing executable");
                return None;
            }
            Some(Command::Start(parts[1].to_string()))
        }
        "spawn" => {
            if parts.len() < 2 {
                logger::err("Invalid 'spawn' command: missing executable");
                return None;
            }
            let mut spawn_args: Vec<String> = parts[1..].iter().map(|s| s.to_string()).collect();
            let exe = spawn_args.first().cloned().unwrap_or_default();
            spawn_args.remove(0);
            Some(Command::Spawn(exe, spawn_args))
        }
        "setenv" => {
            if parts.len() < 3 {
                logger::err("Invalid 'setenv' command: missing key or value");
                return None;
            }
            Some(Command::SetEnv(parts[1].to_string(), parts[2].to_string()))
        }
        "regset" => {
            if parts.len() < 2 {
                logger::err("Invalid 'regset' command: missing path or value");
                return None;
            }
            // Parse: regset HKCU\Software\LibreNT\Mode debug
            // Split the second part into path and value
            let rest: Vec<&str> = parts[1].splitn(2, ' ').collect();
            if rest.len() < 2 {
                logger::err("Invalid 'regset' command: path and value not properly separated");
                return None;
            }
            Some(Command::RegSet(rest[0].to_string(), rest[1].to_string()))
        }
        "log" => {
            if parts.len() < 2 {
                logger::err("Invalid 'log' command: missing message");
                return None;
            }
            Some(Command::Log(parts[1].to_string()))
        }
        _ => {
            logger::err(&format!("Unknown command: {}", command));
            None
        }
    }
}

pub fn execute_command(cmd: &Command) -> Result<(), String> {
    match cmd {
        Command::Start(exe) => {
            process::start_process(exe)?;
        }
        Command::Spawn(exe, args) => {
            let args_str = if args.is_empty() {
                None
            } else {
                Some(args.join(" "))
            };
            process::spawn_process(exe, args_str.as_deref())?;
        }
        Command::SetEnv(key, value) => {
            environment::set_env_var(key, value)?;
        }
        Command::RegSet(path, value) => {
            registry::set_registry_value(path, value)?;
        }
        Command::Log(message) => {
            logger::log(message);
        }
    }
    Ok(())
}

pub fn parse_and_execute(path: &Path) -> Result<(), String> {
    let file = File::open(path)
        .map_err(|e| format!("Failed to open init file: {}", e))?;
    
    let reader = BufReader::new(file);
    
    for line_result in reader.lines() {
        let line = line_result
            .map_err(|e| format!("Failed to read line: {}", e))?;
        
        if let Some(cmd) = parse_line(&line) {
            execute_command(&cmd)?;
        }
    }
    
    Ok(())
}
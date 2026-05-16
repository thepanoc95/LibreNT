/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     LibreNT daemon implementation
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



use std::io::{self, Write};

pub fn info(message: &str) {
    println!("[INFO] {}", message);
}

pub fn err(message: &str) {
    eprintln!("[ERR] {}", message);
}

pub fn log(message: &str) {
    println!("[LOG] {}", message);
}

pub fn flush() -> io::Result<()> {
    io::stdout().flush()
}
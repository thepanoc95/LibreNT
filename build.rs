/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Cargo build script for native components
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



use std::env;
use std::path::{Path, PathBuf};

fn main() {
    let target_os = env::var("CARGO_CFG_TARGET_OS").unwrap_or_default();
    if target_os != "windows" {
        return;
    }

    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());

    compile_windows_components(&manifest_dir);
    compile_ntos2nd_components(&manifest_dir);

    println!("cargo:rustc-link-lib=user32");
    println!("cargo:rustc-link-lib=shell32");
    println!("cargo:rustc-link-lib=advapi32");
    println!("cargo:rustc-link-lib=kernel32");
}

fn compile_windows_components(root: &Path) {
    let sources = [
        root.join("mswindows/desktop.launcher.dk.c"),
        root.join("mswindows/fallback.explorer.c"),
    ];

    for file in &sources {
        println!("cargo:rerun-if-changed={}", file.display());
    }

    cc::Build::new()
        .files(sources)
        .include(root.join("mswindows"))
        .warnings(true)
        .compile("libre_nt_mswindows_components");
}

fn compile_ntos2nd_components(root: &Path) {
    let sources = [
        root.join("ntos2nd/NTCall.c"),
        root.join("ntos2nd/MSWindows.c"),
    ];

    for file in &sources {
        println!("cargo:rerun-if-changed={}", file.display());
    }

    println!("cargo:rerun-if-changed={}", root.join("ntos2nd/ntos2nd/syscall.h").display());
    println!("cargo:rerun-if-changed={}", root.join("ntos2nd/ntos2nd/kernel.h").display());

    cc::Build::new()
        .files(sources)
        .include(root.join("ntos2nd"))
        .include(root.join("ntos2nd/ntos2nd"))
        .warnings(true)
        .compile("libre_nt_ntos2nd_components");
}

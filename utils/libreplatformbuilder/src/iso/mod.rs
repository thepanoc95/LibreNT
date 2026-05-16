/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     LibreNT platform builder utility
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



//! ISO handling module
//! 
//! Handles extraction and repacking of Windows ISO files

use anyhow::{Result, anyhow};
use log::{info, debug};
use std::path::{Path, PathBuf};
use std::fs;
use std::process::Command;

/// ISO handler for Windows installation media
pub struct IsoHandler {
    work_dir: PathBuf,
}

impl IsoHandler {
    pub fn new(work_dir: PathBuf) -> Self {
        Self { work_dir }
    }

    /// Extract Windows ISO to a working directory
    pub fn extract(&self, iso_path: &Path) -> Result<PathBuf> {
        let extract_dir = self.work_dir.join("iso_extracted");
        fs::create_dir_all(&extract_dir)?;
        
        info!("Extracting ISO: {:?}", iso_path);
        
        let output = Command::new("7z")
            .args(["x", "-y", format!("-o{}", extract_dir.display()).as_str(), iso_path.to_str().unwrap()])
            .output()
            .or_else(|_| {
                // Fallback to bsdtar if 7z not available
                Command::new("bsdtar")
                    .args(["-xf", iso_path.to_str().unwrap(), "-C", extract_dir.to_str().unwrap()])
                    .output()
            })?;
        
        if !output.status.success() {
            return Err(anyhow!("Failed to extract ISO: {}", String::from_utf8_lossy(&output.stderr)));
        }
        
        Ok(extract_dir)
    }

    /// Get paths to WIM files in the extracted ISO
    pub fn get_wim_paths(&self, extract_dir: &Path) -> Result<IsoWimPaths> {
        let sources_dir = extract_dir.join("sources");
        
        Ok(IsoWimPaths {
            install_wim: sources_dir.join("install.wim"),
            install_esd: sources_dir.join("install.esd"),
            boot_wim: sources_dir.join("boot.wim"),
        })
    }

    /// Repack the modified files back into an ISO
    pub fn repack(&self, extract_dir: &Path, output_path: &Path, version: &VersionInfo) -> Result<()> {
        info!("Repacking ISO to {:?}", output_path);
        
        // Ensure output directory exists
        if let Some(parent) = output_path.parent() {
            fs::create_dir_all(parent)?;
        }
        
        // Use mkisofs or similar to create ISO
        let iso_name = format!(
            "LibreNT-{}-{}-{}.iso",
            version.version,
            version.build_num,
            version.build_type
        );
        
        let output = Command::new("mkisofs")
            .args([
                "-iso-level", "3",
                "-J", "-r",
                "-o", &output_path.to_string_lossy(),
                "-b", "boot/etfsboot.com",
                "-no-emul-boot",
                "-boot-load-size", "8",
                "-boot-info-table",
                "-eltorito-alt-boot",
                "-e", "efi/microsoft/boot/efisys.bin",
                "-no-emul-boot",
                "-isohybrid-gpt-basdat",
                extract_dir.to_str().unwrap()
            ])
            .output()
            .or_else(|_| {
                // Fallback to xorriso
                Command::new("xorriso")
                    .args([
                        "-as", "mkisofs",
                        "-iso-level", "3",
                        "-J", "-r",
                        "-o", &output_path.to_string_lossy(),
                        extract_dir.to_str().unwrap()
                    ])
                    .output()
            })?;
        
        if !output.status.success() {
            return Err(anyhow!("Failed to create ISO: {}", String::from_utf8_lossy(&output.stderr)));
        }
        
        // Rename to proper name if needed
        if output_path.file_name().map(|n| n != iso_name.as_str()).unwrap_or(true) {
            let final_path = output_path.parent().unwrap().join(&iso_name);
            fs::rename(output_path, &final_path)?;
        }
        
        info!("ISO created successfully: {:?}", output_path);
        Ok(())
    }
}

/// Paths to WIM files in ISO
pub struct IsoWimPaths {
    pub install_wim: PathBuf,
    pub install_esd: PathBuf,
    pub boot_wim: PathBuf,
}

/// Version information for ISO naming
#[derive(Debug, Clone)]
pub struct VersionInfo {
    pub version: String,
    pub build_num: String,
    pub build_type: String,
}

impl VersionInfo {
    pub fn new(version: &str, build_num: &str, build_type: &str) -> Self {
        Self {
            version: version.to_string(),
            build_num: build_num.to_string(),
            build_type: build_type.to_string(),
        }
    }
}

/// ISO metadata
#[derive(Debug, Clone)]
pub struct IsoMetadata {
    pub label: String,
    pub publisher: String,
    pub application: String,
    pub volume_id: String,
}
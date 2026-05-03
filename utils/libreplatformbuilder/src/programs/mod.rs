//! Program injector module
//! 
//! Handles adding custom programs to Windows images

use anyhow::{Result, anyhow};
use log::{info, debug, warn};
use std::path::{Path, PathBuf};
use std::fs;
use std::collections::HashMap;

/// Program injector for Windows images
pub struct ProgramInjector {
    programs_source: PathBuf,
}

impl ProgramInjector {
    pub fn new(programs_source: PathBuf) -> Self {
        Self { programs_source }
    }

    /// Get available programs from the source directory
    pub fn get_available_programs(&self) -> Result<Vec<AvailableProgram>> {
        let mut programs = Vec::new();
        
        if !self.programs_source.exists() {
            warn!("Programs source directory does not exist: {}", self.programs_source.display());
            return Ok(programs);
        }
        
        for entry in fs::read_dir(&self.programs_source)? {
            let entry = entry?;
            if entry.path().is_dir() {
                let program = self.read_program_metadata(&entry.path())?;
                programs.push(program);
            }
        }
        
        Ok(programs)
    }

    fn read_program_metadata(&self, program_dir: &Path) -> Result<AvailableProgram> {
        let name = program_dir.file_name()
            .and_then(|n| n.to_str())
            .unwrap_or("Unknown")
            .to_string();
        
        // Try to read program metadata from a manifest file
        let manifest_path = program_dir.join("manifest.json");
        
        if manifest_path.exists() {
            let content = fs::read_to_string(&manifest_path)?;
            let manifest: ProgramManifest = serde_json::from_str(&content)?;
            
            Ok(AvailableProgram {
                name: manifest.name,
                description: manifest.description,
                version: manifest.version,
                identifier: manifest.identifier,
                source_path: program_dir.to_path_buf(),
                install_type: manifest.install_type,
                size: self.calculate_dir_size(program_dir)?,
            })
        } else {
            // Use directory name as program info
            Ok(AvailableProgram {
                name,
                description: "No description available".to_string(),
                version: "1.0.0".to_string(),
                identifier: program_dir.file_name().unwrap().to_string_lossy().to_string(),
                source_path: program_dir.to_path_buf(),
                install_type: InstallType::Copy,
                size: self.calculate_dir_size(program_dir)?,
            })
        }
    }

    fn calculate_dir_size(&self, dir: &Path) -> Result<u64> {
        let mut size = 0;
        for entry in walkdir::WalkDir::new(dir) {
            let entry = entry?;
            if entry.path().is_file() {
                size += entry.metadata()?.len();
            }
        }
        Ok(size)
    }

    /// Install programs to the Windows image
    pub fn inject_programs(&self, image_dir: &Path, selected_programs: &[&str]) -> Result<()> {
        let available = self.get_available_programs()?;
        let program_map: HashMap<String, &AvailableProgram> = available.iter()
            .map(|p| (p.identifier.clone(), p))
            .collect();
        
        for identifier in selected_programs {
            if let Some(program) = program_map.get(*identifier) {
                info!("Installing program: {}", program.name);
                self.install_program(image_dir, program)?;
            } else {
                warn!("Program not found: {}", identifier);
            }
        }
        
        Ok(())
    }

    fn install_program(&self, image_dir: &Path, program: &AvailableProgram) -> Result<()> {
        let dest_dir = image_dir.join("Program Files\\LibreNT\\Apps").join(&program.identifier);
        fs::create_dir_all(&dest_dir)?;
        
        match program.install_type {
            InstallType::Copy => {
                self.copy_program_files(&program.source_path, &dest_dir)?;
            }
            InstallType::Msix => {
                self.install_msix(&program.source_path, &dest_dir)?;
            }
            InstallType::Msi => {
                self.install_msi(&program.source_path, &dest_dir)?;
            }
        }
        
        Ok(())
    }

    fn copy_program_files(&self, source: &Path, dest: &Path) -> Result<()> {
        for entry in walkdir::WalkDir::new(source) {
            let entry = entry?;
            let relative = entry.path().strip_prefix(source)?;
            let dest_path = dest.join(relative);
            
            if entry.path().is_dir() {
                fs::create_dir_all(&dest_path)?;
            } else {
                if let Some(parent) = dest_path.parent() {
                    fs::create_dir_all(parent)?;
                }
                fs::copy(entry.path(), &dest_path)?;
            }
        }
        Ok(())
    }

    fn install_msix(&self, source: &Path, _dest: &Path) -> Result<()> {
        // MSIX installation would require special handling
        // For now, just copy the files
        warn!("MSIX installation not fully implemented, copying files");
        self.copy_program_files(source, _dest)?;
        Ok(())
    }

    fn install_msi(&self, source: &Path, _dest: &Path) -> Result<()> {
        // MSI installation would require Windows Installer
        warn!("MSI installation not fully implemented, copying files");
        self.copy_program_files(source, _dest)?;
        Ok(())
    }

    /// Check if programs source directory exists and has content
    pub fn validate_source(&self) -> Result<()> {
        if !self.programs_source.exists() {
            return Err(anyhow!("Programs source directory does not exist: {}", self.programs_source.display()));
        }
        
        let programs = self.get_available_programs()?;
        if programs.is_empty() {
            warn!("No programs found in source directory");
        }
        
        Ok(())
    }
}

/// Available program information
#[derive(Debug, Clone)]
pub struct AvailableProgram {
    pub name: String,
    pub description: String,
    pub version: String,
    pub identifier: String,
    pub source_path: PathBuf,
    pub install_type: InstallType,
    pub size: u64,
}

/// Program manifest file format
#[derive(Debug, Clone, serde::Deserialize)]
pub struct ProgramManifest {
    pub name: String,
    pub description: String,
    pub version: String,
    #[serde(default = "default_identifier")]
    pub identifier: String,
    #[serde(default)]
    pub install_type: InstallType,
}

fn default_identifier() -> String {
    "default".to_string()
}

/// Installation type for programs
#[derive(Debug, Clone, Copy, PartialEq, serde::Deserialize)]
#[serde(rename_all = "lowercase")]
pub enum InstallType {
    Copy,
    Msix,
    Msi,
}

impl Default for InstallType {
    fn default() -> Self {
        Self::Copy
    }
}
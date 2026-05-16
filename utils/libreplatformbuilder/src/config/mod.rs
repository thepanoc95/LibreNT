/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     LibreNT platform builder utility
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



//! Configuration module
//! 
//! Handles build configuration and settings

use anyhow::Result;
use serde::{Deserialize, Serialize};
use std::path::PathBuf;

/// Build configuration
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BuildConfig {
    pub version: String,
    pub build_num: String,
    pub build_type: String,
    pub iso_path: Option<PathBuf>,
    pub work_dir: PathBuf,
    pub programs_dir: PathBuf,
    pub custom_installer: Option<PathBuf>,
    
    #[serde(default)]
    pub components_to_remove: Vec<String>,
    
    #[serde(default)]
    pub components_to_add: Vec<String>,
    
    #[serde(default)]
    pub programs_to_install: Vec<String>,
    
    #[serde(default = "default_true")]
    pub apply_registry_mods: bool,
    
    #[serde(default = "default_true")]
    pub add_winui3: bool,
}

fn default_true() -> bool { true }

impl Default for BuildConfig {
    fn default() -> Self {
        Self {
            version: "1.0".to_string(),
            build_num: "001".to_string(),
            build_type: "dev".to_string(),
            iso_path: None,
            work_dir: PathBuf::from("./work"),
            programs_dir: PathBuf::from("../build"),
            custom_installer: None,
            components_to_remove: Vec::new(),
            components_to_add: Vec::new(),
            programs_to_install: Vec::new(),
            apply_registry_mods: true,
            add_winui3: true,
        }
    }
}

impl BuildConfig {
    /// Load configuration from file
    pub fn load(path: &PathBuf) -> Result<Self> {
        let content = std::fs::read_to_string(path)?;
        let config: Self = toml::from_str(&content)?;
        Ok(config)
    }

    /// Save configuration to file
    pub fn save(&self, path: &PathBuf) -> Result<()> {
        let content = toml::to_string_pretty(self)?;
        std::fs::write(path, content)?;
        Ok(())
    }

    /// Generate ISO filename
    pub fn iso_filename(&self) -> String {
        format!(
            "LibreNT-{}-{}-{}.iso",
            self.version,
            self.build_num,
            self.build_type
        )
    }
}

/// Configuration file structure
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ConfigFile {
    pub build: BuildConfig,
    pub paths: PathsConfig,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PathsConfig {
    pub default_work_dir: PathBuf,
    pub default_programs_dir: PathBuf,
}
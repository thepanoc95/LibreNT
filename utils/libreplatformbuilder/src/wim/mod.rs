/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     LibreNT platform builder utility
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



//! WIM (Windows Imaging Format) operations module
//! 
//! This module handles all WIM-related operations including extraction,
//! modification, and repacking using wimlib.

use anyhow::{Result, anyhow};
use log::{info, warn, debug};
use std::path::{Path, PathBuf};
use std::collections::HashMap;

pub mod install_wim;
pub mod boot_wim;

/// Supported Windows editions
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum WindowsEdition {
    Home,
    Pro,
    Enterprise,
    Education,
    Server,
}

impl WindowsEdition {
    pub fn from_name(name: &str) -> Option<Self> {
        let name_lower = name.to_lowercase();
        match name_lower.as_str() {
            "home" | "homeedition" => Some(WindowsEdition::Home),
            "pro" | "professional" => Some(WindowsEdition::Pro),
            "enterprise" => Some(WindowsEdition::Enterprise),
            "education" => Some(WindowsEdition::Education),
            "server" => Some(WindowsEdition::Server),
            _ => None,
        }
    }

    pub fn display_name(&self) -> &'static str {
        match self {
            WindowsEdition::Home => "Windows 11 Home",
            WindowsEdition::Pro => "Windows 11 Pro",
            WindowsEdition::Enterprise => "Windows 11 Enterprise",
            WindowsEdition::Education => "Windows 11 Education",
            WindowsEdition::Server => "Windows Server",
        }
    }
}

/// Information about a WIM file
#[derive(Debug, Clone)]
pub struct WimInfo {
    pub path: PathBuf,
    pub image_count: usize,
    pub compression: String,
    pub total_size: u64,
    pub images: Vec<ImageInfo>,
}

/// Information about an individual image in a WIM
#[derive(Debug, Clone)]
pub struct ImageInfo {
    pub index: usize,
    pub name: String,
    pub description: String,
    pub edition: Option<WindowsEdition>,
    pub size: u64,
}

/// Configuration for WIM operations
#[derive(Debug, Clone)]
pub struct WimConfig {
    pub compression: WimCompression,
    pub chunk_size: Option<u32>,
    pub solid_compression: bool,
}

impl Default for WimConfig {
    fn default() -> Self {
        Self {
            compression: WimCompression::Lzx,
            chunk_size: Some(32768),
            solid_compression: true,
        }
    }
}

/// WIM compression types
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum WimCompression {
    None,
    Xpress,
    Lzx,
    Msip,
}

impl Default for WimCompression {
    fn default() -> Self {
        Self::Lzx
    }
}

/// Component to remove from Windows
#[derive(Debug, Clone)]
pub struct ComponentToRemove {
    pub name: String,
    pub description: String,
    pub files: Vec<String>,
    pub registry_keys: Vec<String>,
    pub packages: Vec<String>,
}

impl ComponentToRemove {
    pub fn new(name: &str, description: &str) -> Self {
        Self {
            name: name.to_string(),
            description: description.to_string(),
            files: Vec::new(),
            registry_keys: Vec::new(),
            packages: Vec::new(),
        }
    }

    pub fn with_files(mut self, files: Vec<&str>) -> Self {
        self.files = files.iter().map(|s| s.to_string()).collect();
        self
    }

    pub fn with_registry(mut self, keys: Vec<&str>) -> Self {
        self.registry_keys = keys.iter().map(|s| s.to_string()).collect();
        self
    }

    pub fn with_packages(mut self, packages: Vec<&str>) -> Self {
        self.packages = packages.iter().map(|s| s.to_string()).collect();
        self
    }
}

/// Predefined components to remove from LibreNT
pub fn get_microsoft_components_to_remove() -> Vec<ComponentToRemove> {
    vec![
        // Explorer shell and related
        ComponentToRemove::new("explorer.exe", "Windows Explorer shell")
            .with_files(vec!["Windows\\explorer.exe"])
            .with_registry(vec!["SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Shell"]),

        // Internet Explorer
        ComponentToRemove::new("Internet Explorer", "Legacy web browser")
            .with_files(vec!["Program Files\\Internet Explorer\\iexplore.exe"])
            .with_packages(vec!["Internet-Explorer-Optional-amd64"]),

        // Edge browser
        ComponentToRemove::new("Edge", "Microsoft Edge browser")
            .with_files(vec!["Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe"])
            .with_registry(vec!["SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\msedge.exe"]),

        // Windows Media Player
        ComponentToRemove::new("Windows Media Player", "Legacy media player")
            .with_files(vec!["Program Files\\Windows Media Player\\wmplayer.exe"])
            .with_packages(vec!["WindowsMediaPlayer"]),

        // Cortana
        ComponentToRemove::new("Cortana", "Virtual assistant")
            .with_files(vec!["Windows\\SystemApps\\Microsoft Cortana MobileUI_...\\Cortana.exe"])
            .with_packages(vec!["Cortana"]),

        // Xbox components
        ComponentToRemove::new("Xbox Apps", "Xbox gaming services")
            .with_files(vec!["Windows\\SystemApps\\Microsoft.XboxGamingOverlay_...\\XboxGameOverlay.exe"])
            .with_packages(vec!["XboxGamingOverlay", "XboxIdentityProvider", "XboxSpeechToText"]),

        // Microsoft Store
        ComponentToRemove::new("Microsoft Store", "Windows Store app")
            .with_files(vec!["Windows\\SystemApps\\Microsoft.WindowsStore_...\\WinStore.App.exe"])
            .with_packages(vec!["Microsoft.Store"]),

        // Candy Crush and other bloatware
        ComponentToRemove::new("Bloatware", "Pre-installed games and apps")
            .with_packages(vec!["CandyCrushSaga", "FarmVille3", "DisneyMagicKingdoms"]),

        // Windows Hello
        ComponentToRemove::new("Windows Hello", "Biometric authentication")
            .with_files(vec!["Windows\\SystemApps\\Microsoft.BioEnrollment_...\\BiometricFramework.dll"])
            .with_registry(vec!["SOFTWARE\\Microsoft\\Biometrics"]),

        // OneDrive
        ComponentToRemove::new("OneDrive", "Microsoft cloud storage")
            .with_files(vec!["Windows\\SysWOW64\\OneDriveSetup.exe", "Windows\\System32\\OneDriveSetup.exe"])
            .with_packages(vec!["Microsoft.OneDrive"]),
    ]
}

/// Components to add to LibreNT (open-source alternatives)
pub fn get_opensource_components_to_add() -> Vec<ComponentToAdd> {
    vec![
        ComponentToAdd::new("Firefox", "Mozilla Firefox web browser", "firefox"),
        ComponentToAdd::new("LibreOffice", "Open-source office suite", "libreoffice"),
        ComponentToAdd::new("VLC", "Media player", "vlc"),
        ComponentToAdd::new("7-Zip", "File archiver", "7zip"),
        ComponentToAdd::new("GIMP", "Image editor", "gimp"),
        ComponentToAdd::new("Notepad++", "Text editor", "notepadpp"),
        ComponentToAdd::new("SumatraPDF", "PDF reader", "sumatrapdf"),
    ]
}

/// Component to add to Windows
#[derive(Debug, Clone)]
pub struct ComponentToAdd {
    pub name: String,
    pub description: String,
    pub identifier: String,
    pub source_path: Option<PathBuf>,
}

impl ComponentToAdd {
    pub fn new(name: &str, description: &str, identifier: &str) -> Self {
        Self {
            name: name.to_string(),
            description: description.to_string(),
            identifier: identifier.to_string(),
            source_path: None,
        }
    }

    pub fn with_source(mut self, path: PathBuf) -> Self {
        self.source_path = Some(path);
        self
    }
}

/// Error types for WIM operations
#[derive(Debug, thiserror::Error)]
pub enum WimError {
    #[error("WIM file not found: {0}")]
    WimNotFound(String),
    
    #[error("Failed to extract WIM: {0}")]
    ExtractFailed(String),
    
    #[error("Failed to update WIM: {0}")]
    UpdateFailed(String),
    
    #[error("Invalid WIM structure: {0}")]
    InvalidStructure(String),
    
    #[error("Image index out of bounds")]
    ImageIndexOutOfBounds,
    
    #[error("Compression error: {0}")]
    CompressionError(String),
}

pub type WimResult<T> = Result<T, WimError>;

/// Core WIM operations trait
pub trait WimOperations {
    /// Get information about a WIM file
    fn get_info(&self, path: &Path) -> WimResult<WimInfo>;
    
    /// Extract a specific image from the WIM
    fn extract_image(&self, wim_path: &Path, image_index: usize, output_dir: &Path) -> WimResult<()>;
    
    /// Update a WIM file with modifications
    fn update_wim(&self, wim_path: &Path, config: &WimConfig) -> WimResult<()>;
    
    /// Add a file to a WIM image
    fn add_file(&self, wim_path: &Path, image_index: usize, source: &Path, dest: &Path) -> WimResult<()>;
    
    /// Remove a file from a WIM image
    fn remove_file(&self, wim_path: &Path, image_index: usize, path: &Path) -> WimResult<()>;
    
    /// List all images in a WIM
    fn list_images(&self, wim_path: &Path) -> WimResult<Vec<ImageInfo>>;
}
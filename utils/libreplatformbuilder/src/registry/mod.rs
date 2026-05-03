//! Registry modifications module
//! 
//! Handles applying registry changes to Windows images

use anyhow::{Result, anyhow};
use log::info;
use std::path::{Path, PathBuf};
use std::fs;

/// Registry manager for Windows images
pub struct RegistryManager {
    config_dir: PathBuf,
}

impl RegistryManager {
    pub fn new(config_dir: PathBuf) -> Self {
        Self { config_dir }
    }

    /// Apply all registry modifications to the image
    pub fn apply_all_modifications(&self, image_dir: &Path) -> Result<()> {
        info!("Applying registry modifications");
        
        let modifications = self.get_registry_modifications();
        
        for modification in modifications {
            self.apply_modification(image_dir, &modification)?;
        }
        
        Ok(())
    }

    /// Get all registry modifications to apply
    fn get_registry_modifications(&self) -> Vec<RegistryModification> {
        vec![
            // Disable Cortana
            RegistryModification::new(
                "SOFTWARE\\Policies\\Microsoft\\Windows\\Windows Search",
                "AllowCortana",
                RegistryValue::Dword(0),
            ),
            
            // Disable Windows Consumer Features (shopping apps etc.)
            RegistryModification::new(
                "SOFTWARE\\Policies\\Microsoft\\Windows\\CloudContent",
                "DisableWindowsConsumerFeatures",
                RegistryValue::Dword(1),
            ),
            
            // Disable telemetry
            RegistryModification::new(
                "SOFTWARE\\Policies\\Microsoft\\Windows\\DataCollection",
                "AllowTelemetry",
                RegistryValue::Dword(0),
            ),
            
            // Set custom shell (replace explorer with our shell)
            RegistryModification::new(
                "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
                "Shell",
                RegistryValue::String("libre-shell.exe".to_string()),
            ),
            
            // Disable Windows Tips
            RegistryModification::new(
                "SOFTWARE\\Policies\\Microsoft\\Windows\\CloudContent",
                "DisableSoftLanding",
                RegistryValue::Dword(1),
            ),
            
            // Disable lock screen
            RegistryModification::new(
                "SOFTWARE\\Policies\\Microsoft\\Windows\\Personalization",
                "NoLockScreen",
                RegistryValue::Dword(1),
            ),
            
            // Set LibreNT branding
            RegistryModification::new(
                "SOFTWARE\\LibreNT",
                "ProductName",
                RegistryValue::String("LibreNT".to_string()),
            ),
        ]
    }

    /// Apply a single registry modification
    fn apply_modification(&self, image_dir: &Path, modification: &RegistryModification) -> Result<()> {
        // In a real implementation, we would parse and modify the registry hive
        // For now, we create a .reg file that can be imported during installation
        let formatted_value = self.format_registry_value(&modification.value);
        let reg_file_content = format!(
            "[HKEY_{}\\{}]\n\"{}\"={}\n",
            modification.key,
            modification.value_name,
            modification.value_name,
            formatted_value
        );
        
        // Write to a temporary reg file
        let reg_file = image_dir.join("Windows\\Temp\\librent_registry.reg");
        if let Some(parent) = reg_file.parent() {
            fs::create_dir_all(parent)?;
        }
        fs::write(&reg_file, &reg_file_content)?;
        
        Ok(())
    }

    fn get_hive_path(&self, image_dir: &Path, key: &str) -> Result<PathBuf> {
        let parts: Vec<&str> = key.split('\\').collect();
        
        let hive = match parts.first().copied() {
            Some("SOFTWARE") => "SOFTWARE",
            Some("SYSTEM") => "SYSTEM",
            Some("SECURITY") => "SECURITY",
            Some("SAM") => "SAM",
            Some("DEFAULT") => "DEFAULT",
            _ => return Err(anyhow!("Unknown registry hive")),
        };
        
        Ok(image_dir.join("Windows\\System32\\Config").join(hive))
    }

    fn format_registry_value(&self, value: &RegistryValue) -> String {
        match value {
            RegistryValue::Dword(v) => format!("dword:{:08x}", v),
            RegistryValue::String(v) => format!("\"{}\"", v),
            RegistryValue::Binary(v) => format!("hex:{}", v.iter().map(|b| format!("{:02x}", b)).collect::<Vec<_>>().join(",")),
            RegistryValue::Qword(v) => format!("hex(b):{:016x}", v),
        }
    }

    /// Load a registry hive for modification
    pub fn load_hive(&self, image_dir: &Path, hive_name: &str) -> Result<RegistryHive> {
        let hive_path = image_dir.join("Windows\\System32\\Config").join(hive_name);
        
        if !hive_path.exists() {
            return Err(anyhow!("Registry hive not found: {}", hive_path.display()));
        }
        
        Ok(RegistryHive::new(hive_path))
    }

    /// Export registry modifications to a .reg file
    pub fn export_reg_file(&self, output_path: &Path) -> Result<()> {
        let modifications = self.get_registry_modifications();
        
        let mut content = String::from("Windows Registry Editor Version 5.00\n\n");
        
        for modification in modifications {
            let formatted = self.format_registry_value(&modification.value);
            content.push_str(&format!(
                "[HKEY_{}\\{}]\n\"{}\"={}\n\n",
                modification.key,
                modification.value_name,
                modification.value_name,
                formatted
            ));
        }
        
        fs::write(output_path, content)?;
        Ok(())
    }
}

/// Registry modification to apply
#[derive(Debug, Clone)]
pub struct RegistryModification {
    pub key: String,
    pub value_name: String,
    pub value: RegistryValue,
}

impl RegistryModification {
    pub fn new(key: &str, value_name: &str, value: RegistryValue) -> Self {
        Self {
            key: key.to_string(),
            value_name: value_name.to_string(),
            value,
        }
    }
}

/// Registry value types
#[derive(Debug, Clone)]
pub enum RegistryValue {
    Dword(u32),
    String(String),
    Binary(Vec<u8>),
    Qword(u64),
}

/// Registry hive wrapper
pub struct RegistryHive {
    path: PathBuf,
}

impl RegistryHive {
    pub fn new(path: PathBuf) -> Self {
        Self { path }
    }

    pub fn path(&self) -> &Path {
        &self.path
    }
}
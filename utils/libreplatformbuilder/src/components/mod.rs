/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     LibreNT platform builder utility
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



//! Component manager module
//! 
//! Manages Windows components - adding open-source alternatives and removing proprietary components

use anyhow::Result;
use log::{info, debug};
use std::path::{Path, PathBuf};
use std::fs;

/// Component manager for Windows images
pub struct ComponentManager {
    /// Path to open-source programs to add
    programs_dir: PathBuf,
}

impl ComponentManager {
    pub fn new(programs_dir: PathBuf) -> Self {
        Self { programs_dir }
    }

    /// Get list of components to remove
    pub fn get_removal_list(&self) -> Vec<super::wim::ComponentToRemove> {
        super::wim::get_microsoft_components_to_remove()
    }

    /// Get list of open-source components to add
    pub fn get_addition_list(&self) -> Vec<super::wim::ComponentToAdd> {
        let mut components = super::wim::get_opensource_components_to_add();
        
        // Set source path for each component if available
        for component in &mut components {
            let source_path = self.programs_dir.join(&component.identifier);
            if source_path.exists() {
                component.source_path = Some(source_path);
            }
        }
        
        components
    }

    /// Remove Microsoft components from extracted image
    pub fn remove_components(&self, image_dir: &Path) -> Result<()> {
        let components = self.get_removal_list();
        
        for component in components {
            info!("Removing component: {}", component.name);
            self.remove_component_files(image_dir, &component)?;
            self.remove_component_registry(image_dir, &component)?;
        }
        
        Ok(())
    }

    /// Remove files associated with a component
    fn remove_component_files(&self, image_dir: &Path, component: &super::wim::ComponentToRemove) -> Result<()> {
        for file_pattern in &component.files {
            // Handle wildcards in paths
            let pattern = if file_pattern.contains('*') {
                file_pattern.replace('*', "**")
            } else {
                file_pattern.clone()
            };
            
            let full_pattern = image_dir.join(&pattern);
            let glob_pattern = full_pattern.to_string_lossy().to_string();
            
            for entry in glob::glob(&glob_pattern)? {
                if let Ok(path) = entry {
                    if path.is_file() {
                        info!("Removing file: {}", path.display());
                        fs::remove_file(&path)?;
                    }
                }
            }
        }
        
        Ok(())
    }

    /// Remove registry entries for a component
    fn remove_component_registry(&self, image_dir: &Path, component: &super::wim::ComponentToRemove) -> Result<()> {
        // Registry hives are in Windows\System32\Config\
        let config_dir = image_dir.join("Windows\\System32\\Config");
        
        // In a real implementation, we would modify the registry hives
        // This requires parsing the registry binary format or using a tool
        for key in &component.registry_keys {
            debug!("Would remove registry key: {}", key);
        }
        
        Ok(())
    }

    /// Add open-source components to the image
    pub fn add_components(&self, image_dir: &Path) -> Result<()> {
        let components = self.get_addition_list();
        
        for component in components {
            if component.source_path.is_some() {
                info!("Adding component: {}", component.name);
                self.install_component(image_dir, &component)?;
            } else {
                debug!("Skipping component (no source): {}", component.name);
            }
        }
        
        Ok(())
    }

    /// Install a single component
    fn install_component(&self, image_dir: &Path, component: &super::wim::ComponentToAdd) -> Result<()> {
        let source = component.source_path.as_ref()
            .ok_or_else(|| anyhow::anyhow!("No source path for component"))?;
        
        let dest_dir = image_dir.join("Program Files\\LibreNT\\Components").join(&component.identifier);
        fs::create_dir_all(&dest_dir)?;
        
        // Copy all files from source to destination
        for entry in walkdir::WalkDir::new(source) {
            let entry = entry?;
            let relative = entry.path().strip_prefix(source)?;
            let dest = dest_dir.join(relative);
            
            if entry.path().is_dir() {
                fs::create_dir_all(&dest)?;
            } else {
                if let Some(parent) = dest.parent() {
                    fs::create_dir_all(parent)?;
                }
                fs::copy(entry.path(), &dest)?;
            }
        }
        
        Ok(())
    }

    /// Add WinUI 3 runtime to the image
    pub fn add_winui3(&self, image_dir: &Path) -> Result<()> {
        info!("Adding WinUI 3 runtime");
        
        // WinUI 3 files go to System32
        let system32 = image_dir.join("Windows\\System32");
        fs::create_dir_all(&system32)?;
        
        // Windows App Runtime bootstrap
        let bootstrap_dir = image_dir.join("Windows\\AppReadiness");
        fs::create_dir_all(&bootstrap_dir)?;
        
        // Copy WinUI 3 DLLs from programs directory if available
        let winui3_source = self.programs_dir.join("winui3");
        if winui3_source.exists() {
            self.copy_winui3_files(&winui3_source, &system32)?;
        } else {
            info!("WinUI 3 source not found, creating placeholders");
            self.create_winui3_placeholders(&system32)?;
        }
        
        Ok(())
    }

    fn copy_winui3_files(&self, source: &Path, dest: &Path) -> Result<()> {
        for entry in walkdir::WalkDir::new(source) {
            let entry = entry?;
            if entry.path().is_file() {
                let relative = entry.path().strip_prefix(source)?;
                let dest_path = dest.join(relative);
                if let Some(parent) = dest_path.parent() {
                    fs::create_dir_all(parent)?;
                }
                fs::copy(entry.path(), &dest_path)?;
            }
        }
        Ok(())
    }

    fn create_winui3_placeholders(&self, dest: &Path) -> Result<()> {
        // Create placeholder entries for WinUI 3 DLLs
        let winui_dlls = [
            "Microsoft.UI.Composition.dll",
            "Microsoft.UI.Xaml.dll",
            "Microsoft.UI.Xaml.Controls.dll",
            "Microsoft.UI.Xaml.Markup.dll",
            "Microsoft.UI.Xaml.Media.dll",
            "Microsoft.UI.Xaml.Input.dll",
            "Microsoft.UI.Xaml.Primitives.dll",
            "Microsoft.UI.Xaml.Shapes.dll",
            "Microsoft.UI.Xaml.Hosting.dll",
            "Microsoft.WindowsAppRuntime.dll",
            "Microsoft.WindowsAppRuntime.Bootstrap.dll",
        ];
        
        for dll in &winui_dlls {
            let dll_path = dest.join(dll);
            if !dll_path.exists() {
                fs::write(&dll_path, "")?; // Placeholder
            }
        }
        
        Ok(())
    }
}
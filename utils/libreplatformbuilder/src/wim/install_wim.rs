//! Operations for install.wim files
//! 
//! Handles extraction and modification of Windows installation images

use super::*;
use anyhow::Result;
use std::process::Command;
use std::fs;

/// Installer WIM handler
pub struct InstallWim {
    config: WimConfig,
    work_dir: PathBuf,
}

impl InstallWim {
    pub fn new(work_dir: PathBuf) -> Self {
        Self {
            config: WimConfig::default(),
            work_dir,
        }
    }

    /// Extract install.wim to a working directory
    pub fn extract(&self, wim_path: &Path, image_index: usize) -> Result<PathBuf> {
        let extract_dir = self.work_dir.join("install_extracted");
        fs::create_dir_all(&extract_dir)?;
        
        info!("Extracting install.wim image {} to {:?}", image_index, extract_dir);
        
        let output = Command::new("wimlib-imagex")
            .args(["apply", wim_path.to_str().unwrap(), &image_index.to_string(), extract_dir.to_str().unwrap()])
            .output()?;
        
        if !output.status.success() {
            return Err(anyhow!("Failed to extract WIM: {}", String::from_utf8_lossy(&output.stderr)));
        }
        
        Ok(extract_dir)
    }

    /// Remove Microsoft components from the extracted image
    pub fn remove_microsoft_components(&self, extract_dir: &Path) -> Result<()> {
        let components = super::get_microsoft_components_to_remove();
        
        for component in components {
            info!("Removing component: {}", component.name);
            
            for file in &component.files {
                let file_path = extract_dir.join(file);
                if file_path.exists() {
                    fs::remove_file(&file_path)?;
                    debug!("Removed file: {}", file);
                }
            }
        }
        
        Ok(())
    }

    /// Add WinUI 3 to the extracted image
    pub fn add_winui3(&self, extract_dir: &Path) -> Result<()> {
        info!("Adding WinUI 3 components");
        
        // WinUI 3 is part of Windows App SDK
        // We need to add the necessary DLLs and runtime files
        
        let winui3_dir = extract_dir.join("Windows\\System32");
        fs::create_dir_all(&winui3_dir)?;
        
        // Placeholder for actual WinUI 3 files
        // In production, these would come from the Windows App SDK
        let winui_files = [
            "Microsoft.UI.Composition.dll",
            "Microsoft.UI.Xaml.dll",
            "Microsoft.UI.Xaml.Input.dll",
            "Microsoft.UI.Xaml.Media.dll",
            "Microsoft.UI.Xaml.Shell.dll",
            "Microsoft.WindowsAppRuntime.dll",
            "Microsoft.WindowsAppRuntime.Bootstrap.dll",
        ];
        
        for file in &winui_files {
            let dest = winui3_dir.join(file);
            // In production, copy actual files from Windows App SDK
            fs::write(&dest, "")?; // Placeholder
        }
        
        Ok(())
    }

    /// Repack the modified image back to a WIM
    pub fn repack(&self, extract_dir: &Path, output_path: &Path) -> Result<()> {
        info!("Repacking install.wim from {:?}", extract_dir);
        
        let output = Command::new("wimlib-imagex")
            .args(["capture", extract_dir.to_str().unwrap(), output_path.to_str().unwrap(), 
                   "LibreNT", "--compress", "lzx", "--solid"])
            .output()?;
        
        if !output.status.success() {
            return Err(anyhow!("Failed to capture WIM: {}", String::from_utf8_lossy(&output.stderr)));
        }
        
        Ok(())
    }
}
//! Operations for boot.wim files
//! 
//! Handles extraction and modification of Windows PE boot images

use super::*;
use anyhow::Result;
use std::process::Command;
use std::fs;

/// Boot WIM handler
pub struct BootWim {
    config: WimConfig,
    work_dir: PathBuf,
}

impl BootWim {
    pub fn new(work_dir: PathBuf) -> Self {
        Self {
            config: WimConfig::default(),
            work_dir,
        }
    }

    /// Extract boot.wim to a working directory
    pub fn extract(&self, wim_path: &Path, image_index: usize) -> Result<PathBuf> {
        let extract_dir = self.work_dir.join("boot_extracted");
        fs::create_dir_all(&extract_dir)?;
        
        info!("Extracting boot.wim image {} to {:?}", image_index, extract_dir);
        
        let output = Command::new("wimlib-imagex")
            .args(["apply", wim_path.to_str().unwrap(), &image_index.to_string(), extract_dir.to_str().unwrap()])
            .output()?;
        
        if !output.status.success() {
            return Err(anyhow!("Failed to extract boot.wim: {}", String::from_utf8_lossy(&output.stderr)));
        }
        
        Ok(extract_dir)
    }

    /// Inject custom installer into the boot image
    pub fn inject_installer(&self, extract_dir: &Path, installer_path: &Path) -> Result<()> {
        info!("Injecting custom installer into boot image");
        
        // Create directory for the installer
        let sources_dir = extract_dir.join("sources");
        fs::create_dir_all(&sources_dir)?;
        
        // Copy the installer (typically setup.exe or custom installer)
        let dest = sources_dir.join("setup.exe");
        fs::copy(installer_path, &dest)?;
        
        // Update boot configuration
        let bcdedit_path = extract_dir.join("boot");
        fs::create_dir_all(&bcdedit_path)?;
        
        // Modify boot configuration to use our installer
        self.modify_boot_config(&bcdedit_path)?;
        
        Ok(())
    }

    /// Modify boot configuration
    fn modify_boot_config(&self, boot_dir: &Path) -> Result<()> {
        // Create a custom BCD (Boot Configuration Data)
        let bcd_content = r#"[bootmgr]
timeout=30
default=LibreNT

[LibreNT]
bootsequence=vga,ntdetect
path=\windows\system32\winload.efi
"#;
        
        fs::write(boot_dir.join("bcd"), bcd_content)?;
        Ok(())
    }

    /// Repack the modified boot image
    pub fn repack(&self, extract_dir: &Path, output_path: &Path) -> Result<()> {
        info!("Repacking boot.wim from {:?}", extract_dir);
        
        let output = Command::new("wimlib-imagex")
            .args(["capture", extract_dir.to_str().unwrap(), output_path.to_str().unwrap(),
                   "LibreNT Boot", "--compress", "lzx"])
            .output()?;
        
        if !output.status.success() {
            return Err(anyhow!("Failed to capture boot.wim: {}", String::from_utf8_lossy(&output.stderr)));
        }
        
        Ok(())
    }
}
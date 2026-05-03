//! Builder module
//! 
//! Main build orchestration logic

use anyhow::{Result, anyhow};
use log::info;
use std::path::{Path, PathBuf};
use std::fs;

use crate::config::BuildConfig;
use crate::wim::install_wim::InstallWim;
use crate::wim::boot_wim::BootWim;
use crate::iso::{IsoHandler, VersionInfo};
use crate::components::ComponentManager;
use crate::registry::RegistryManager;
use crate::programs::ProgramInjector;

/// Main builder orchestrator
pub struct Builder {
    config: BuildConfig,
    iso_handler: IsoHandler,
    install_wim: InstallWim,
    boot_wim: BootWim,
    component_manager: ComponentManager,
    registry_manager: RegistryManager,
    program_injector: ProgramInjector,
}

impl Builder {
    pub fn new(config: BuildConfig) -> Result<Self> {
        let work_dir = config.work_dir.clone();
        
        Ok(Self {
            config,
            iso_handler: IsoHandler::new(work_dir.join("iso_work")),
            install_wim: InstallWim::new(work_dir.join("install_work")),
            boot_wim: BootWim::new(work_dir.join("boot_work")),
            component_manager: ComponentManager::new(work_dir.join("programs")),
            registry_manager: RegistryManager::new(work_dir.join("registry")),
            program_injector: ProgramInjector::new(work_dir.join("programs")),
        })
    }

    /// Run the complete build process
    pub fn build(&self) -> Result<()> {
        info!("Starting LibreNT build process");
        
        // Step 1: Extract ISO
        let iso_path = self.config.iso_path.as_ref()
            .ok_or_else(|| anyhow!("No ISO path specified"))?;
        
        let extract_dir = self.iso_handler.extract(iso_path)?;
        
        // Step 2: Get WIM paths
        let wim_paths = self.iso_handler.get_wim_paths(&extract_dir)?;
        
        // Step 3: Process install.wim
        self.process_install_wim(&wim_paths.install_wim, &extract_dir)?;
        
        // Step 4: Process boot.wim
        self.process_boot_wim(&wim_paths.boot_wim, &extract_dir)?;
        
        // Step 5: Repack ISO
        let output_path = self.config.work_dir.join("output").join(self.config.iso_filename());
        fs::create_dir_all(output_path.parent().unwrap())?;
        
        let version = VersionInfo::new(
            &self.config.version,
            &self.config.build_num,
            &self.config.build_type,
        );
        
        self.iso_handler.repack(&extract_dir, &output_path, &version)?;
        
        info!("Build complete: {:?}", output_path);
        Ok(())
    }

    fn process_install_wim(&self, wim_path: &Path, extract_dir: &Path) -> Result<()> {
        info!("Processing install.wim");
        
        // Extract the first image (typically Windows 11 Pro)
        let extracted = self.install_wim.extract(wim_path, 1)?;
        
        // Remove Microsoft components
        self.component_manager.remove_components(&extracted)?;
        
        // Add WinUI 3
        if self.config.add_winui3 {
            self.component_manager.add_winui3(&extracted)?;
        }
        
        // Apply registry modifications
        if self.config.apply_registry_mods {
            self.registry_manager.apply_all_modifications(&extracted)?;
        }
        
        // Add programs
        let program_refs: Vec<&str> = self.config.programs_to_install.iter().map(String::as_str).collect();
        self.program_injector.inject_programs(&extracted, &program_refs)?;
        
        // Repack
        let output_wim = extract_dir.join("sources").join("install.wim");
        self.install_wim.repack(&extracted, &output_wim)?;
        
        Ok(())
    }

    fn process_boot_wim(&self, wim_path: &Path, extract_dir: &Path) -> Result<()> {
        info!("Processing boot.wim");
        
        // Extract boot image
        let extracted = self.boot_wim.extract(wim_path, 1)?;
        
        // Inject custom installer if specified
        if let Some(installer) = &self.config.custom_installer {
            self.boot_wim.inject_installer(&extracted, installer)?;
        }
        
        // Repack
        let output_wim = extract_dir.join("sources").join("boot.wim");
        self.boot_wim.repack(&extracted, &output_wim)?;
        
        Ok(())
    }
}
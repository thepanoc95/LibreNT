//! GUI (Graphical User Interface) module
//! 
//! Desktop interface using eframe/egui

use anyhow::{Result, anyhow};

/// Run the GUI application
pub fn run_gui() -> Result<()> {
    #[cfg(feature = "gui")]
    {
        let native_options = eframe::NativeOptions {
            initial_window_size: Some(egui::vec2(1024.0, 720.0)),
            initial_window_title: "LibreNT Platform Builder".to_string(),
            ..Default::default()
        };
        
        let app = LibreNtBuilderApp::default();
        let result = eframe::run_native(
            "LibreNT Platform Builder",
            native_options,
            Box::new(|_cc| Ok(Box::new(app))),
        );
        
        result?;
        Ok(())
    }
    
    #[cfg(not(feature = "gui"))]
    {
        Err(anyhow!("GUI feature not enabled. Rebuild with --features gui"))
    }
}

/// Main GUI application
#[cfg(feature = "gui")]
#[derive(Default)]
struct LibreNtBuilderApp {
    state: GuiState,
}

#[cfg(feature = "gui")]
impl eframe::App for LibreNtBuilderApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        egui::CentralPanel::default().show(ctx, |ui| {
            ui.heading("LibreNT Platform Builder");
            ui.add_space(20.0);
            
            ui.horizontal(|ui| {
                ui.label("Windows 11 ISO:");
                ui.label(self.state.iso_path.as_ref()
                    .map(|p| p.display().to_string())
                    .unwrap_or_else(|| "Not selected".to_string()));
                if ui.button("Browse").clicked() {
                    // File dialog would open here
                }
            });
            
            ui.add_space(10.0);
            
            ui.horizontal(|ui| {
                ui.label("Build Directory:");
                ui.label(self.state.build_dir.as_ref()
                    .map(|p| p.display().to_string())
                    .unwrap_or_else(|| "Not selected".to_string()));
                if ui.button("Browse").clicked() {
                    // File dialog would open here
                }
            });
            
            ui.add_space(20.0);
            
            ui.collapsing("Build Configuration", |ui| {
                ui.text_edit_singleline(&mut self.state.version);
                ui.text_edit_singleline(&mut self.state.build_num);
                ui.checkbox(&mut self.state.winui3, "Add WinUI 3");
                ui.checkbox(&mut self.state.registry_mods, "Apply Registry Modifications");
            });
            
            ui.add_space(20.0);
            
            if ui.button("Build LibreNT ISO").clicked() {
                self.state.building = true;
            }
            
            if self.state.building {
                ui.add(egui::ProgressBar::new(self.state.progress).text("Building..."));
            }
        });
    }
}

/// GUI application state
#[derive(Default)]
struct GuiState {
    iso_path: Option<std::path::PathBuf>,
    build_dir: Option<std::path::PathBuf>,
    programs_dir: Option<std::path::PathBuf>,
    version: String,
    build_num: String,
    build_type: String,
    winui3: bool,
    registry_mods: bool,
    building: bool,
    progress: f32,
}
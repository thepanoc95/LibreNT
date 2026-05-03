//! LibreNT Platform Builder
//! 
//! Build custom Windows ISOs with open-source alternatives

use anyhow::Result;
use clap::Parser;
use log::{info, LevelFilter};
use std::path::PathBuf;

use libreplatformbuilder::ui::UIMode;
use libreplatformbuilder::config::BuildConfig;
use libreplatformbuilder::builder::Builder;

/// LibreNT Platform Builder CLI
#[derive(Parser, Debug)]
#[command(name = "libreplatformbuilder")]
#[command(author = "LibreNT Project")]
#[command(version = "0.1.0")]
#[command(about = "Build custom Windows ISOs with open-source alternatives", long_about = None)]
struct Args {
    /// Path to Windows 11 ISO
    #[arg(short, long)]
    iso: Option<PathBuf>,

    /// Output directory for the built ISO
    #[arg(short, long, default_value = "./output")]
    output: PathBuf,

    /// Version number for the build
    #[arg(short, long, default_value = "1.0")]
    version: String,

    /// Build number
    #[arg(long, default_value = "001")]
    build_num: String,

    /// Build type (dev, beta, stable, release)
    #[arg(long, default_value = "dev")]
    build_type: String,

    /// Programs directory to inject
    #[arg(long, default_value = "../build")]
    programs_dir: PathBuf,

    /// Custom installer to inject into boot.wim
    #[arg(long)]
    custom_installer: Option<PathBuf>,

    /// UI mode (auto, tui, gui)
    #[arg(short, long, default_value = "auto")]
    ui: UIMode,

    /// Apply registry modifications
    #[arg(long, default_value = "true")]
    registry_mods: bool,

    /// Add WinUI 3
    #[arg(long, default_value = "true")]
    winui3: bool,

    /// Run without UI (CLI mode)
    #[arg(long)]
    headless: bool,
}

fn main() -> Result<()> {
    // Initialize logging
    env_logger::Builder::new()
        .filter_level(LevelFilter::Info)
        .init();

    let args = Args::parse();

    if args.headless || args.iso.is_some() {
        // CLI mode - run build directly
        let config = BuildConfig {
            version: args.version,
            build_num: args.build_num,
            build_type: args.build_type,
            iso_path: args.iso,
            work_dir: args.output.join("work"),
            programs_dir: args.programs_dir,
            custom_installer: args.custom_installer,
            apply_registry_mods: args.registry_mods,
            add_winui3: args.winui3,
            ..Default::default()
        };

        let builder = Builder::new(config)?;
        builder.build()?;
    } else {
        // UI mode
        match args.ui {
            UIMode::Tui | UIMode::Auto => {
                libreplatformbuilder::ui::tui::run_tui()?;
            }
            UIMode::Gui => {
                libreplatformbuilder::ui::gui::run_gui()?;
            }
        }
    }

    Ok(())
}
//! TUI binary entry point
//! 
//! Run `lpb-tui` to start the terminal interface

fn main() -> anyhow::Result<()> {
    env_logger::init();
    libreplatformbuilder::ui::tui::run_tui()?;
    Ok(())
}
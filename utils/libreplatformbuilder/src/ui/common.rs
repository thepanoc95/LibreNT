//! Common UI functions

use anyhow::Result;

use super::UIMode;

/// Run the appropriate UI based on mode
pub fn run_ui(mode: UIMode) -> Result<()> {
    match mode {
        UIMode::Tui | UIMode::Auto => {
            crate::ui::tui::run_tui()?;
        }
        UIMode::Gui => {
            crate::ui::gui::run_gui()?;
        }
    }
    Ok(())
}
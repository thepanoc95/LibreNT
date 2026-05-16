/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     LibreNT platform builder utility
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



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
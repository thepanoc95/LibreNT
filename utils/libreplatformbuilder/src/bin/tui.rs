/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     LibreNT platform builder utility
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



//! TUI binary entry point
//! 
//! Run `lpb-tui` to start the terminal interface

fn main() -> anyhow::Result<()> {
    env_logger::init();
    libreplatformbuilder::ui::tui::run_tui()?;
    Ok(())
}
/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     LibreNT platform builder utility
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



//! GUI binary entry point
//! 
//! Run `lpb-gui` to start the graphical interface (requires `--features gui`)

fn main() -> anyhow::Result<()> {
    env_logger::init();
    libreplatformbuilder::ui::gui::run_gui()?;
    Ok(())
}
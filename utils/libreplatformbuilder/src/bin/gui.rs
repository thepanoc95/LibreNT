//! GUI binary entry point
//! 
//! Run `lpb-gui` to start the graphical interface (requires `--features gui`)

fn main() -> anyhow::Result<()> {
    env_logger::init();
    libreplatformbuilder::ui::gui::run_gui()?;
    Ok(())
}
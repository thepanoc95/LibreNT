//! TUI (Terminal User Interface) module
//! 
//! Text-based interface using ratatui

use anyhow::Result;
use crossterm::{
    event::{self, DisableMouseCapture, EnableMouseCapture, Event, KeyCode, KeyEventKind},
    execute,
    terminal::{disable_raw_mode, enable_raw_mode, EnterAlternateScreen, LeaveAlternateScreen},
};
use ratatui::{
    backend::CrosstermBackend,
    layout::{Constraint, Direction, Layout, Rect},
    style::{Color, Style, Stylize},
    text::{Line, Text},
    widgets::{Block, Borders, List, ListItem, Paragraph, Wrap},
    Terminal,
};
use std::io;
use std::path::PathBuf;

use crate::ui::UIBuildConfig;

/// TUI application state
#[derive(Debug, Clone, Default)]
pub struct TuiState {
    pub iso_path: Option<PathBuf>,
    pub build_dir: Option<PathBuf>,
    pub programs_dir: Option<PathBuf>,
    pub current_screen: Screen,
    pub build_config: UIBuildConfig,
    pub message: Option<String>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum Screen {
    #[default]
    Main,
    SelectISO,
    SelectBuildDir,
    BuildConfig,
    Building,
    Complete,
}

/// Run the TUI application
pub fn run_tui() -> Result<Option<UIBuildConfig>> {
    enable_raw_mode()?;
    let mut stdout = io::stdout();
    execute!(stdout, EnterAlternateScreen, EnableMouseCapture)?;
    let backend = CrosstermBackend::new(stdout);
    let mut terminal = Terminal::new(backend)?;

    let mut state = TuiState::default();
    let res = run_app(&mut terminal, &mut state);

    // Restore terminal
    disable_raw_mode()?;
    execute!(
        terminal.backend_mut(),
        LeaveAlternateScreen,
        DisableMouseCapture
    )?;
    terminal.show_cursor()?;

    res
}

fn run_app<B: ratatui::backend::Backend>(
    terminal: &mut Terminal<B>,
    state: &mut TuiState,
) -> Result<Option<UIBuildConfig>> {
    loop {
        terminal.draw(|f| ui(f, state))?;

        if let Event::Key(key) = event::read()? {
            if key.kind == KeyEventKind::Press {
                match state.current_screen {
                    Screen::Main => handle_main_input(key, state)?,
                    Screen::SelectISO => handle_file_input(key, state)?,
                    Screen::SelectBuildDir => handle_file_input(key, state)?,
                    Screen::BuildConfig => handle_config_input(key, state)?,
                    Screen::Building => {
                        if key.code == KeyCode::Esc {
                            return Ok(None);
                        }
                    }
                    Screen::Complete => {
                        if key.code == KeyCode::Enter {
                            return Ok(Some(state.build_config.clone()));
                        } else if key.code == KeyCode::Esc {
                            return Ok(None);
                        }
                    }
                }
            }
        }
    }
}

fn handle_main_input(key: event::KeyEvent, state: &mut TuiState) -> Result<()> {
    match key.code {
        KeyCode::Char('1') | KeyCode::Char('i') => {
            state.current_screen = Screen::SelectISO;
        }
        KeyCode::Char('2') | KeyCode::Char('b') => {
            state.current_screen = Screen::SelectBuildDir;
        }
        KeyCode::Char('3') | KeyCode::Char('c') => {
            state.current_screen = Screen::BuildConfig;
        }
        KeyCode::Char('4') | KeyCode::Char('s') => {
            if state.iso_path.is_some() {
                state.current_screen = Screen::Building;
            } else {
                state.message = Some("Please select an ISO first".to_string());
            }
        }
        KeyCode::Char('q') | KeyCode::Esc => {
            return Ok(());
        }
        _ => {}
    }
    Ok(())
}

fn handle_file_input(key: event::KeyEvent, state: &mut TuiState) -> Result<()> {
    match key.code {
        KeyCode::Esc => {
            state.current_screen = Screen::Main;
        }
        KeyCode::Enter => {
            // For demo, just use a placeholder path
            let path = match state.current_screen {
                Screen::SelectISO => PathBuf::from("/path/to/windows11.iso"),
                Screen::SelectBuildDir => PathBuf::from("/path/to/build"),
                _ => PathBuf::from("."),
            };
            match state.current_screen {
                Screen::SelectISO => state.iso_path = Some(path),
                Screen::SelectBuildDir => state.build_dir = Some(path),
                _ => {}
            }
            state.current_screen = Screen::Main;
        }
        _ => {}
    }
    Ok(())
}

fn handle_config_input(key: event::KeyEvent, state: &mut TuiState) -> Result<()> {
    match key.code {
        KeyCode::Esc => {
            state.current_screen = Screen::Main;
        }
        KeyCode::Enter => {
            state.current_screen = Screen::Main;
        }
        _ => {}
    }
    Ok(())
}

fn ui(f: &mut ratatui::Frame, state: &TuiState) {
    let chunks = Layout::default()
        .direction(Direction::Vertical)
        .margin(2)
        .constraints([
            Constraint::Length(3),
            Constraint::Min(0),
            Constraint::Length(3),
        ])
        .split(f.area());

    // Title
    let title = Paragraph::new("LibreNT Platform Builder")
        .style(Style::default().fg(Color::Cyan))
        .block(Block::default().borders(Borders::ALL));
    f.render_widget(title, chunks[0]);

    // Main content based on screen
    match state.current_screen {
        Screen::Main => render_main(f, chunks[1], state),
        Screen::SelectISO => render_file_select(f, chunks[1], "Select ISO", state),
        Screen::SelectBuildDir => render_file_select(f, chunks[1], "Select Build Dir", state),
        Screen::BuildConfig => render_config(f, chunks[1], state),
        Screen::Building => render_building(f, chunks[1], state),
        Screen::Complete => render_complete(f, chunks[1], state),
    }

    // Status bar
    let status = if let Some(msg) = &state.message {
        msg.as_str()
    } else {
        "Press 1-4 for options, q to quit"
    };
    let status_bar = Paragraph::new(status).style(Style::default().fg(Color::Yellow));
    f.render_widget(status_bar, chunks[2]);
}

fn render_main(f: &mut ratatui::Frame, area: Rect, state: &TuiState) {
    let iso_status = if let Some(p) = &state.iso_path {
        format!("ISO: {}", p.display())
    } else {
        "ISO: Not selected".to_string()
    };

    let items = vec![
        ListItem::new(Line::from(vec![
            "1) ".into(),
            "Select ISO".blue().bold(),
            format!(" - {}", iso_status).into(),
        ])),
        ListItem::new(Line::from(vec![
            "2) ".into(),
            "Select Build Directory".blue().bold(),
        ])),
        ListItem::new(Line::from(vec![
            "3) ".into(),
            "Build Configuration".blue().bold(),
        ])),
        ListItem::new(Line::from(vec![
            "4) ".into(),
            "Start Build".green().bold(),
        ])),
        ListItem::new(Line::from("q) Quit".red())),
    ];

    let list = List::new(items).block(Block::default().borders(Borders::NONE));
    f.render_widget(list, area);
}

fn render_file_select(f: &mut ratatui::Frame, area: Rect, title: &str, _state: &TuiState) {
    let text = Text::from(vec![
        Line::from(""),
        Line::from(format!("{} - Press Enter to confirm or Esc to cancel", title)),
        Line::from(""),
        Line::from("[ File browser would appear here ]"),
    ]);
    let para = Paragraph::new(text).block(Block::default().title("Select File").borders(Borders::ALL));
    f.render_widget(para, area);
}

fn render_config(f: &mut ratatui::Frame, area: Rect, state: &TuiState) {
    let config = &state.build_config;
    let text = Text::from(vec![
        Line::from(""),
        Line::from(vec!["Version: ".bold(), config.version.as_str().into()]),
        Line::from(vec!["Build Number: ".bold(), config.build_num.as_str().into()]),
        Line::from(vec!["Build Type: ".bold(), config.build_type.as_str().into()]),
        Line::from(""),
        Line::from(vec![
            "WinUI 3: ".bold(),
            if config.winui3 { "Enabled".green() } else { "Disabled".red() },
        ]),
        Line::from(vec![
            "Registry Mods: ".bold(),
            if config.registry_mods { "Enabled".green() } else { "Disabled".red() },
        ]),
        Line::from(""),
        Line::from("Press Enter to save, Esc to cancel"),
    ]);
    let para = Paragraph::new(text).block(Block::default().title("Build Configuration").borders(Borders::ALL));
    f.render_widget(para, area);
}

fn render_building(f: &mut ratatui::Frame, area: Rect, _state: &TuiState) {
    let text = Text::from(vec![
        Line::from(""),
        Line::from("Building LibreNT ISO..."),
        Line::from(""),
        Line::from("[ Progress bar would appear here ]"),
        Line::from(""),
        Line::from("This may take a while..."),
    ]);
    let para = Paragraph::new(text).block(Block::default().title("Building").borders(Borders::ALL)).wrap(Wrap { trim: true });
    f.render_widget(para, area);
}

fn render_complete(f: &mut ratatui::Frame, area: Rect, _state: &TuiState) {
    let text = Text::from(vec![
        Line::from(""),
        Line::from("Build Complete!").green().bold(),
        Line::from(""),
        Line::from("Press Enter to exit or Esc to return to main menu"),
    ]);
    let para = Paragraph::new(text).block(Block::default().title("Complete").borders(Borders::ALL));
    f.render_widget(para, area);
}
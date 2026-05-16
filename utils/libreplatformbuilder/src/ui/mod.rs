/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     LibreNT platform builder utility
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



//! UI module
//! 
//! Provides both TUI (Terminal) and GUI interfaces for LibreNT Platform Builder

pub mod tui;
pub mod gui;
pub mod common;

use clap::ValueEnum;

/// UI mode selection
#[derive(Debug, Clone, ValueEnum, PartialEq, Eq, Default)]
pub enum UIMode {
    #[default]
    Auto,
    Tui,
    Gui,
}

/// Build configuration from UI
#[derive(Debug, Clone)]
pub struct UIBuildConfig {
    pub version: String,
    pub build_num: String,
    pub build_type: String,
    pub keep_components: Vec<String>,
    pub add_components: Vec<String>,
    pub inject_programs: Vec<String>,
    pub registry_mods: bool,
    pub winui3: bool,
}

impl Default for UIBuildConfig {
    fn default() -> Self {
        Self {
            version: "1.0".to_string(),
            build_num: "001".to_string(),
            build_type: "dev".to_string(),
            keep_components: Vec::new(),
            add_components: Vec::new(),
            inject_programs: Vec::new(),
            registry_mods: true,
            winui3: true,
        }
    }
}
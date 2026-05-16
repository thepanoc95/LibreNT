/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Windows installer or deployment component
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



#include "RegistryPatcher.h"
#include <windows.h>
#include <filesystem>

namespace fs = std::filesystem;

RegistryPatcher::RegistryPatcher() {}

RegistryPatcher::~RegistryPatcher() {}

bool RegistryPatcher::LoadHive(const std::string& hivePath, const std::string& mountPath) {
    // Load registry hive from mounted image
    // reg load HKLM\LibreNTHive <mountPath>\Windows\System32\Config\SOFTWARE
    std::string cmd = "reg load HKLM\\LibreNTHive \"" + mountPath + "\\Windows\\System32\\Config\\SOFTWARE\"";
    return RunRegCommand(cmd);
}

bool RegistryPatcher::UnloadHive(const std::string& mountPath) {
    std::string cmd = "reg unload HKLM\\LibreNTHive";
    return RunRegCommand(cmd);
}

bool RegistryPatcher::SetShell(const std::string& shellPath) {
    // Set LibreNT Explorer as the shell
    // HKCU\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon\Shell
    m_pendingValues.push_back({
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
        "Shell",
        shellPath,
        REG_SZ
    });
    return true;
}

bool RegistryPatcher::DisableWindowsDefender() {
    // Disable Windows Defender
    m_pendingValues.push_back({
        "SOFTWARE\\Policies\\Microsoft\\Windows Defender",
        "DisableAntiSpyware",
        "1",
        REG_DWORD
    });
    return true;
}

bool RegistryPatcher::EnableVerboseBoot() {
    // Enable verbose boot status messages
    m_pendingValues.push_back({
        "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\BootExecute",
        "",
        "autocheck autochk *",
        REG_SZ
    });
    return true;
}

bool RegistryPatcher::DisableTelemetry() {
    // Disable telemetry services
    m_pendingValues.push_back({
        "SOFTWARE\\Policies\\Microsoft\\Windows\\DataCollection",
        "AllowTelemetry",
        "0",
        REG_DWORD
    });
    return true;
}

bool RegistryPatcher::SetPerformanceOptions() {
    // Performance tweaks
    m_pendingValues.push_back({
        "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management",
        "ClearPageFileAtShutdown",
        "0",
        REG_DWORD
    });
    return true;
}

bool RegistryPatcher::ApplyPatches(const std::string& mountPath) {
    // Load hive
    if (!LoadHive("", mountPath)) {
        return false;
    }
    
    // Apply all pending values
    for (const auto& val : m_pendingValues) {
        WriteRegistryValue(val.key, val.name, val.value, val.type);
    }
    
    // Unload hive
    return UnloadHive(mountPath);
}

bool RegistryPatcher::WriteRegistryValue(const std::string& key, const std::string& name,
                                          const std::string& value, int type) {
    // Write registry value using reg add command
    std::string cmd = "reg add \"HKLM\\LibreNTHive\\" + key + "\" /v " + name + 
                      " /t REG_SZ /d \"" + value + "\" /f";
    return RunRegCommand(cmd);
}

bool RegistryPatcher::RunRegCommand(const std::string& args) {
    // Execute registry command
    // In production, this would use CreateProcess with proper error handling
    return true;
}
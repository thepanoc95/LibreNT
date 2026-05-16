/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Windows installer or deployment component
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



#include "InstallerUI.h"
#include "imgui.h"
#include "VerboseBoot.h"
#include "DeploymentEngine.h"
#include <windows.h>
#include <cstdio>
#include <algorithm>

InstallerUI* g_InstallerUI = nullptr;

InstallerUI::InstallerUI() {
    InitializeComponents();
    RefreshDrives();
    g_InstallerUI = this;
}

InstallerUI::~InstallerUI() {
    g_InstallerUI = nullptr;
}

void InstallerUI::InitializeComponents() {
    m_components = {
        {"btrfs-drivers", "BTRFS Filesystem Drivers", true, true},
        {"quibble-bootloader", "Quibble EFI Bootloader", true, true},
        {"librent-explorer", "LibreNT Explorer (WinUI 3 Shell)", true, false},
        {"registry-patches", "System Registry Modifications", true, false},
        {"performance-tweaks", "Performance Optimizations", true, false}
    };
}

void InstallerUI::RefreshDrives() {
    m_availableDrives.clear();
    
    char drives[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (char drive : drives) {
        char path[4] = {drive, ':', '\\', '\0'};
        UINT type = GetDriveTypeA(path);
        if (type == DRIVE_FIXED) {
            char volumeName[MAX_PATH];
            char filesystem[MAX_PATH];
            DWORD serialNumber, maxComponentLen, fileSystemFlags;
            
            if (GetVolumeInformationA(path, volumeName, MAX_PATH, &serialNumber,
                &maxComponentLen, &fileSystemFlags, filesystem, MAX_PATH)) {
                char displayName[MAX_PATH];
                snprintf(displayName, MAX_PATH, "%c:\\ - %s (%s)", 
                    drive, volumeName[0] ? volumeName : "Local Disk", filesystem);
                m_availableDrives.push_back(std::string(displayName));
            }
        }
    }
}

void InstallerUI::ShowInstallerWindow(bool* p_open) {
    ImGuiIO& io = ImGui::GetIO();
    
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), 
                           ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | 
                                     ImGuiWindowFlags_NoResize |
                                     ImGuiWindowFlags_NoScrollbar;
    
    ImGui::Begin("LibreNT Installer", p_open, window_flags);
    
    if (!m_installing) {
        DrawDriveSelection();
        ImGui::Spacing();
        DrawComponentList();
        ImGui::Spacing();
        DrawButtonBar();
    } else {
        DrawInstallationProgress();
    }
    
    ImGui::End();
}

void InstallerUI::DrawDriveSelection() {
    ImGui::Text("Target Installation Drive:");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##drive_combo", m_selectedDrive.c_str())) {
        for (const auto& drive : m_availableDrives) {
            bool is_selected = (m_selectedDrive == drive);
            if (ImGui::Selectable(drive.c_str(), is_selected)) {
                m_selectedDrive = drive;
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

void InstallerUI::DrawComponentList() {
    ImGui::Text("Components to Install:");
    ImGui::Separator();
    
    ImGui::BeginChild("##components", ImVec2(0, 200), true);
    for (auto& component : m_components) {
        ImGui::CheckboxFlags(component.name.c_str(), 
            reinterpret_cast<int*>(&component.enabled), 
            component.enabled ? 1 : 0);
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", component.description.c_str());
        }
    }
    ImGui::EndChild();
}

void InstallerUI::DrawButtonBar() {
    float buttonWidth = 120;
    
    if (ImGui::Button("Refresh Drives", ImVec2(buttonWidth, 0))) {
        RefreshDrives();
    }
    ImGui::SameLine(0, 10);
    
    if (ImGui::Button("Start Installation", ImVec2(buttonWidth, 0)) && !m_installing) {
        m_installing = true;
        m_progress = 0.0f;
        m_statusMessage = "Preparing installation...";
    }
    ImGui::SameLine(0, 10);
    
    if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
        m_installing = false;
        m_progress = 0.0f;
    }
}

void InstallerUI::DrawInstallationProgress() {
    ImGui::Text("%s", m_statusMessage.c_str());
    ImGui::Spacing();
    
    ImGui::ProgressBar(m_progress, ImVec2(-1, 0), 
        (m_progress < 1.0f) ? (std::to_string((int)(m_progress * 100)) + "%").c_str() : "Complete!");
    
    if (m_progress >= 1.0f) {
        if (ImGui::Button("Finish")) {
            m_installing = false;
            m_progress = 0.0f;
            m_statusMessage = "";
        }
    }
}

void InstallerUI::AddVerboseMessage(const std::string& message) {
    m_verboseMessages.push_back(message);
}

void InstallerUI::ShowVerboseBootWindow(bool* p_open) {
    VerboseBoot::Show(m_verboseMessages, p_open);
}
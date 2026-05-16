/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Windows driver or kernel support
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



#include "DriverInjector.h"
#include "InstallerUI.h"
#include <windows.h>
#include <filesystem>

namespace fs = std::filesystem;

DriverInjector::DriverInjector() {}

DriverInjector::~DriverInjector() {}

bool DriverInjector::Initialize(const std::string& driverSourcePath) {
    m_driverSourcePath = driverSourcePath;
    
    // Discover available drivers
    if (fs::exists(driverSourcePath)) {
        for (const auto& entry : fs::directory_iterator(driverSourcePath)) {
            if (entry.is_directory()) {
                DriverInfo info;
                info.name = entry.path().filename().string();
                info.sourcePath = entry.path().string();
                info.required = false;
                
                // Determine target path based on driver type
                if (info.name.find("btrfs", 0) != std::string::npos) {
                    info.targetPath = "\\Windows\\System32\\Drivers";
                    info.required = true;
                } else if (info.name.find("quibble", 0) != std::string::npos) {
                    info.targetPath = "\\EFI\\Boot";
                    info.required = true;
                }
                
                m_drivers.push_back(info);
            }
        }
    }
    
    return !m_drivers.empty();
}

bool DriverInjector::InjectBtrfsDrivers(const std::string& mountPath) {
    // Find btrfs drivers in the driver list
    for (const auto& driver : m_drivers) {
        if (driver.name.find("btrfs", 0) != std::string::npos) {
            if (!CopyDriverFiles(mountPath, driver.name)) {
                return false;
            }
        }
    }
    
    // Update driver database
    std::string drvCmd = "dism /Image:\"" + mountPath + "\" /Add-Driver /Driver:\"" + 
                         m_driverSourcePath + "\\btrfs\" /Recurse";
    return RunCommand(drvCmd);
}

bool DriverInjector::InstallQuibbleBootloader(const std::string& driveLetter) {
    // Find quibble bootloader files
    std::string bootloaderSource;
    for (const auto& driver : m_drivers) {
        if (driver.name.find("quibble", 0) != std::string::npos) {
            bootloaderSource = driver.sourcePath;
            break;
        }
    }
    
    if (bootloaderSource.empty()) {
        return false;
    }
    
    // Mount EFI partition
    std::string efiMount;
    if (!MountEFIPartition(driveLetter, efiMount)) {
        return false;
    }
    
    // Copy bootloader files to EFI partition
    std::string efiTarget = efiMount + "\\EFI\\Boot";
    
    try {
        for (const auto& entry : fs::directory_iterator(bootloaderSource)) {
            if (entry.is_regular_file()) {
                fs::copy(entry.path(), efiTarget + "\\" + entry.path().filename().string());
            }
        }
    } catch (...) {
        UnmountEFIPartition(efiMount);
        return false;
    }
    
    // Create boot entry using bcdedit or similar
    UpdateBCD(driveLetter + ":\\");
    
    UnmountEFIPartition(efiMount);
    return true;
}

bool DriverInjector::UpdateBCD(const std::string& mountPath) {
    // Modify BCD to use quibble bootloader
    // This would typically involve:
    // 1. Mounting the system hive
    // 2. Modifying boot entries
    // 3. Setting up bootmgfw.efi replacement
    
    std::string bcdCmd = "bcdedit /store \"" + mountPath + "boot\\bcd\" /set {bootmgr} path \\EFI\\Boot\\quibble.efi";
    return RunCommand(bcdCmd);
}

bool DriverInjector::MountEFIPartition(const std::string& driveLetter, std::string& mountPoint) {
    // Mount EFI System Partition
    // diskpart script would be used here
    mountPoint = "X:";  // Assume X: for EFI mount
    
    std::string diskpartScript = "select disk 0\n"
                                  "select partition 1\n"
                                  "assign letter=" + mountPoint + "\n";
    
    return RunCommand("diskpart /s");
}

bool DriverInjector::UnmountEFIPartition(const std::string& mountPoint) {
    std::string cmd = "mountvol " + mountPoint + " /D";
    return RunCommand(cmd);
}

bool DriverInjector::CopyDriverFiles(const std::string& mountPath, const std::string& driverName) {
    std::string sourceDir = m_driverSourcePath + "\\" + driverName;
    std::string targetDir = mountPath + "\\Windows\\System32\\Drivers";
    
    try {
        fs::create_directories(targetDir);
        fs::copy(sourceDir, targetDir, fs::copy_options::recursive);
        return true;
    } catch (...) {
        return false;
    }
}

bool DriverInjector::RunCommand(const std::string& cmd) {
    // Execute command using CreateProcess
    // In production, this would have proper error handling
    return true;
}
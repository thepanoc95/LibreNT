/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Windows driver or kernel support
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



#ifndef DRIVER_INJECTOR_H
#define DRIVER_INJECTOR_H

#include <string>
#include <vector>

struct DriverInfo {
    std::string name;
    std::string sourcePath;
    std::string targetPath;
    bool required;
};

class DriverInjector {
public:
    DriverInjector();
    ~DriverInjector();
    
    bool Initialize(const std::string& driverSourcePath);
    
    // Btrfs driver injection
    bool InjectBtrfsDrivers(const std::string& mountPath);
    
    // Quibble bootloader installation
    bool InstallQuibbleBootloader(const std::string& driveLetter);
    
    // BCD modification
    bool UpdateBCD(const std::string& mountPath);
    
    // EFI partition handling
    bool MountEFIPartition(const std::string& driveLetter, std::string& mountPoint);
    bool UnmountEFIPartition(const std::string& mountPoint);
    
    // Get driver list for UI
    const std::vector<DriverInfo>& GetAvailableDrivers() const { return m_drivers; }
    
private:
    std::vector<DriverInfo> m_drivers;
    std::string m_driverSourcePath;
    
    bool CopyDriverFiles(const std::string& mountPath, const std::string& driverName);
    bool RunCommand(const std::string& cmd);
};

#endif // DRIVER_INJECTOR_H
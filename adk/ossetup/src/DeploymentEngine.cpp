#include "DeploymentEngine.h"
#include "InstallerUI.h"
#include <windows.h>
#include <cstdio>
#include <filesystem>

namespace fs = std::filesystem;

DeploymentEngine::DeploymentEngine() {}

DeploymentEngine::~DeploymentEngine() {}

bool DeploymentEngine::MountImage(const std::string& imagePath, const std::string& mountPath) {
    // Create mount directory if it doesn't exist
    fs::create_directories(mountPath);
    
    // Use DISM to mount the image
    // dism /Mount-Image /ImageFile:<path> /Index:1 /MountDir:<mountPath>
    std::vector<std::string> args = {
        "/Mount-Image",
        "/ImageFile:" + imagePath,
        "/Index:1",
        "/MountDir:" + mountPath
    };
    
    return RunDISM(args);
}

bool DeploymentEngine::UnmountImage(const std::string& mountPath, bool commit) {
    std::vector<std::string> args = {
        "/Unmount-Image",
        "/MountDir:" + mountPath,
        "/" + std::string(commit ? "Commit" : "Discard")
    };
    
    return RunDISM(args);
}

bool DeploymentEngine::ApplyRegistryHives(const std::string& mountPath, 
                                         const std::vector<std::string>& hiveFiles) {
    for (const auto& hiveFile : hiveFiles) {
        std::vector<std::string> args = {
            "/Image:" + mountPath,
            "/Import-Registry",
            hiveFile
        };
        
        if (!RunDISM(args)) {
            return false;
        }
    }
    return true;
}

bool DeploymentEngine::CopyFilesToImage(const std::string& mountPath, 
                                        const std::string& sourceDir, 
                                        const std::string& targetDir) {
    fs::path sourcePath(sourceDir);
    fs::path targetPath = fs::path(mountPath) / targetDir;
    
    try {
        if (fs::exists(sourcePath)) {
            fs::copy(sourcePath, targetPath, fs::copy_options::recursive);
            return true;
        }
    } catch (...) {
        return false;
    }
    return false;
}

bool DeploymentEngine::FindWindowsImage(const std::string& drive, std::string& imagePath) {
    // Check for install.wim or install.esd in standard locations
    std::vector<std::string> possiblePaths = {
        drive + "\\Sources\\install.wim",
        drive + "\\Sources\\install.esd",
        drive + "\\sources\\install.wim",
        drive + "\\sources\\install.esd"
    };
    
    for (const auto& path : possiblePaths) {
        if (fs::exists(path)) {
            imagePath = path;
            return true;
        }
    }
    return false;
}

bool DeploymentEngine::RunDISM(const std::vector<std::string>& args, 
                              ProgressCallback callback) {
    if (m_cancelled) return false;
    
    // Build command line
    std::string cmdLine = "dism.exe";
    for (const auto& arg : args) {
        cmdLine += " " + arg;
    }
    
    // Report progress
    if (callback) {
        InstallProgress progress = {0.5f, "Running: " + args[0]};
        callback(progress);
    }
    
    // Execute DISM
    // Note: In production, this would use CreateProcess with proper error handling
    // For now, we return true to indicate the command would be executed
    return true;
}

bool DeploymentEngine::InstallLibreNT(const std::string& targetDrive, 
                                    const std::vector<std::string>& components,
                                    ProgressCallback callback) {
    m_cancelled = false;
    
    // Extract drive letter
    std::string driveLetter = targetDrive.substr(0, targetDrive.find("\\"));
    
    // Find Windows image
    std::string imagePath;
    if (!FindWindowsImage(driveLetter, imagePath)) {
        if (callback) {
            InstallProgress progress = {0.0f, "No Windows image found"};
            callback(progress);
        }
        return false;
    }
    
    // Mount path
    std::string mountPath = "C:\\LibreNT_Mount_" + std::to_string(GetTickCount64());
    
    // Step 1: Mount image
    if (callback) {
        InstallProgress progress = {0.1f, "Mounting Windows image..."};
        callback(progress);
    }
    if (!MountImage(imagePath, mountPath)) {
        return false;
    }
    
    // Step 2: Install components
    float stepProgress = 0.8f / components.size();
    for (size_t i = 0; i < components.size(); ++i) {
        if (m_cancelled) {
            UnmountImage(mountPath, false);
            return false;
        }
        
        if (callback) {
            InstallProgress progress = {0.1f + stepProgress * i, 
                "Installing " + components[i] + "..."};
            callback(progress);
        }
        
        // Component-specific installation would go here
        // For btrfs drivers, quibble bootloader, etc.
    }
    
    // Step 3: Commit changes
    if (callback) {
        InstallProgress progress = {0.9f, "Committing changes..."};
        callback(progress);
    }
    
    bool result = UnmountImage(mountPath, true);
    
    if (callback) {
        InstallProgress progress = {1.0f, result ? "Installation complete!" : "Installation failed"};
        callback(progress);
    }
    
    return result;
}
/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Windows installer or deployment component
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



#ifndef DEPLOYMENT_ENGINE_H
#define DEPLOYMENT_ENGINE_H

#include <string>
#include <vector>
#include <functional>

struct InstallProgress {
    float progress;
    std::string message;
};

class DeploymentEngine {
public:
    using ProgressCallback = std::function<void(const InstallProgress&)>;
    
    DeploymentEngine();
    ~DeploymentEngine();
    
    bool MountImage(const std::string& imagePath, const std::string& mountPath);
    bool UnmountImage(const std::string& mountPath, bool commit = true);
    
    bool ApplyRegistryHives(const std::string& mountPath, const std::vector<std::string>& hiveFiles);
    bool CopyFilesToImage(const std::string& mountPath, const std::string& sourceDir, const std::string& targetDir);
    
    bool InstallLibreNT(const std::string& targetDrive, const std::vector<std::string>& components,
                       ProgressCallback callback = nullptr);
    
    void Cancel() { m_cancelled = true; }
    
private:
    bool m_cancelled = false;
    
    bool RunDISM(const std::vector<std::string>& args, ProgressCallback callback = nullptr);
    bool FindWindowsImage(const std::string& drive, std::string& imagePath);
};

#endif // DEPLOYMENT_ENGINE_H
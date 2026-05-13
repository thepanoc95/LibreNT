#ifndef REGISTRY_PATCHER_H
#define REGISTRY_PATCHER_H

#include <string>
#include <vector>

struct RegistryValue {
    std::string key;
    std::string name;
    std::string value;
    int type;  // REG_SZ, REG_DWORD, etc.
};

class RegistryPatcher {
public:
    RegistryPatcher();
    ~RegistryPatcher();
    
    bool LoadHive(const std::string& hivePath, const std::string& mountPath);
    bool UnloadHive(const std::string& mountPath);
    
    // Registry modifications
    bool SetShell(const std::string& shellPath);
    bool DisableWindowsDefender();
    bool EnableVerboseBoot();
    bool DisableTelemetry();
    bool SetPerformanceOptions();
    
    // Apply patches to mounted image
    bool ApplyPatches(const std::string& mountPath);
    
private:
    std::vector<RegistryValue> m_pendingValues;
    
    bool WriteRegistryValue(const std::string& key, const std::string& name, 
                           const std::string& value, int type);
    bool RunRegCommand(const std::string& args);
};

#endif // REGISTRY_PATCHER_H
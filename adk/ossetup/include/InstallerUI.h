#ifndef INSTALLER_UI_H
#define INSTALLER_UI_H

#include <windows.h>
#include <string>
#include <vector>

struct InstallComponent {
    std::string name;
    std::string description;
    bool enabled;
    bool required;
};

class InstallerUI {
public:
    InstallerUI();
    ~InstallerUI();

    void ShowInstallerWindow(bool* p_open);
    void ShowVerboseBootWindow(bool* p_open);
    
    // Installation state
    bool IsInstalling() const { return m_installing; }
    float GetProgress() const { return m_progress; }
    std::string GetStatusMessage() const { return m_statusMessage; }

    // Drive selection
    void RefreshDrives();
    const std::vector<std::string>& GetAvailableDrives() const { return m_availableDrives; }
    std::string GetSelectedDrive() const { return m_selectedDrive; }
    void SetSelectedDrive(const std::string& drive) { m_selectedDrive = drive; }

    // Components
    const std::vector<InstallComponent>& GetComponents() const { return m_components; }

    // Verbose boot messages
    void AddVerboseMessage(const std::string& message);
    const std::vector<std::string>& GetVerboseMessages() const { return m_verboseMessages; }
    void ClearVerboseMessages() { m_verboseMessages.clear(); }

private:
    bool m_installing = false;
    float m_progress = 0.0f;
    std::string m_statusMessage;
    
    std::vector<std::string> m_availableDrives;
    std::string m_selectedDrive;
    
    std::vector<InstallComponent> m_components;
    std::vector<std::string> m_verboseMessages;
    
    bool m_showVerboseBoot = false;
    
    void InitializeComponents();
    void DrawInstallerContent();
    void DrawDriveSelection();
    void DrawComponentList();
    void DrawInstallationProgress();
    void DrawButtonBar();
};

// Global installer UI instance
extern InstallerUI* g_InstallerUI;

#endif // INSTALLER_UI_H
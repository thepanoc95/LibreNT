using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace LibreNT.Shell.Components.StartMenu;

public sealed partial class StartMenu : UserControl
{
    public ObservableCollection<AppItem> PinnedApps { get; } = new();

    public StartMenu()
    {
        InitializeComponent();
        LoadPinnedApps();
    }

    private void LoadPinnedApps()
    {
        PinnedApps.Add(new AppItem 
        { 
            Id = "explorer", 
            Name = "File Explorer", 
            Icon = "\uE720" 
        });
        PinnedApps.Add(new AppItem 
        { 
            Id = "terminal", 
            Name = "Terminal", 
            Icon = "\uE756" 
        });
        PinnedApps.Add(new AppItem 
        { 
            Id = "browser", 
            Name = "Browser", 
            Icon = "\uE75B" 
        });
        PinnedApps.Add(new AppItem 
        { 
            Id = "settings", 
            Name = "Settings", 
            Icon = "\uE713" 
        });
        PinnedApps.Add(new AppItem 
        { 
            Id = "documents", 
            Name = "Documents", 
            Icon = "\uE8A5" 
        });
        PinnedApps.Add(new AppItem 
        { 
            Id = "pictures", 
            Name = "Pictures", 
            Icon = "\uE8A7" 
        });
    }

    private void OnAppClick(object sender, RoutedEventArgs e)
    {
        if (sender is Button button && button.Tag is string appId)
        {
            LaunchApp(appId);
        }
    }

    private void LaunchApp(string appId)
    {
        // Launch the application - in a real implementation,
        // this would use Windows.ApplicationModel.Store or Process.Start
        switch (appId)
        {
            case "explorer":
                Windows.System.ProcessLauncher.TryLaunch("explorer.exe", "");
                break;
            case "terminal":
                Windows.System.ProcessLauncher.TryLaunch("wt.exe", "");
                break;
            case "browser":
                Windows.System.ProcessLauncher.TryLaunch("msedge.exe", "");
                break;
            case "settings":
                Windows.System.ProcessLauncher.TryLaunch("ms-settings:", "");
                break;
            default:
                // For demo purposes, show a message
                // In production, use proper file associations
                break;
        }
    }

    private void OnShutdownClick(object sender, RoutedEventArgs e)
    {
        // Shutdown logic
    }

    private void OnRestartClick(object sender, RoutedEventArgs e)
    {
        // Restart logic
    }
}

public class AppItem
{
    public string Id { get; set; } = string.Empty;
    public string Name { get; set; } = string.Empty;
    public string Icon { get; set; } = "\uE700";
}
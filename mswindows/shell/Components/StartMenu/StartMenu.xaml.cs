using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;

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
        switch (appId)
        {
            case "explorer":
                TryLaunch("explorer.exe");
                break;
            case "terminal":
                TryLaunch("wt.exe");
                break;
            case "browser":
                TryLaunch("msedge.exe");
                break;
            case "settings":
                TryLaunch("ms-settings:");
                break;
            default:
                break;
        }
    }

    private static void TryLaunch(string target)
    {
        try
        {
            Process.Start(new ProcessStartInfo
            {
                FileName = target,
                UseShellExecute = true
            });
        }
        catch
        {
            // TODO: wire into shell notifications/logging.
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
/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Windows shell component
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



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

    private async void LaunchApp(string appId)
    {
        // Launch the application using desktop process APIs and URI launch for shell URIs.
        // ProcessLauncher.TryLaunch is not available in this SDK version.
        if (appId == "settings")
        {
            try
            {
                await Windows.System.Launcher.LaunchUriAsync(new Uri("ms-settings:"));
            }
            catch
            {
                // Ignore launch failure for demo purposes.
            }

            return;
        }

        ProcessStartInfo? startInfo = appId switch
        {
            "explorer" => new ProcessStartInfo("explorer.exe") { UseShellExecute = true },
            "terminal" => new ProcessStartInfo("wt.exe") { UseShellExecute = true },
            "browser" => new ProcessStartInfo("msedge.exe") { UseShellExecute = true },
            _ => null,
        };

        if (startInfo != null)
        {
            try
            {
                Process.Start(startInfo);
            }
            catch
            {
                // Ignore launch failure for demo purposes.
            }
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
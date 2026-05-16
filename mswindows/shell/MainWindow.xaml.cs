/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Windows shell component
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Input;
using LibreNT.Shell.Components.SystemTray;
using LibreNT.Shell.Services;

namespace LibreNT.Shell;

public sealed partial class MainWindow : Window
{
    private readonly WindowManager _windowManager = new();

    public MainWindow()
    {
        InitializeComponent();
        ExtendsContentIntoTitleBar = true;
        SetTitleBar(null); // Custom title bar handling
        Activated += OnActivated;

        // Initialize system tray
        var systemTray = new SystemTray();
        MainTaskbar.SetSystemTray(systemTray);

        // Hook up start button
        MainTaskbar.StartButtonClicked += (s, e) => ToggleStartMenu();
    }

    private void OnActivated(object sender, WindowActivatedEventArgs args)
    {
        // Handle window activation
    }

    public void ToggleStartMenu()
    {
        if (StartMenuPopup.IsOpen)
        {
            StartMenuPopup.IsOpen = false;
        }
        else
        {
            StartMenuPopup.IsOpen = true;
        }
    }

    public void CloseStartMenu()
    {
        StartMenuPopup.IsOpen = false;
    }

    public void SetWallpaperHost(UIElement wallpaperHost)
    {
        WallpaperHost.Content = wallpaperHost;
    }

    public void ClearWallpaperHost()
    {
        WallpaperHost.Content = null;
    }
}
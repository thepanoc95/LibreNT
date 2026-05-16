/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Windows shell component
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Hosting;
using Microsoft.UI.Xaml.Media;
using Microsoft.Windows.AppLifecycle;
using System;
using System.Runtime.InteropServices;
using WinRT;

namespace LibreNT.Shell;

public partial class App : Application
{
    private Window? _window;
    private static readonly string LogoPath = "../../librentlogo.svg";

    public static Window? MainWindow { get; private set; }

    public App()
    {
        InitializeComponent();
        UnhandledException += OnUnhandledException;
    }

    protected override void OnLaunched(LaunchActivatedEventArgs args)
    {
        MainWindow = new MainWindow();
        MainWindow.Activate();
    }

    private void OnUnhandledException(object sender, Microsoft.UI.Xaml.UnhandledExceptionEventArgs e)
    {
        // Log error and continue
    }

    public static string GetLogoPath() => LogoPath;
}
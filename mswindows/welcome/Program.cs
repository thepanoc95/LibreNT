/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Windows welcome application
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



#nullable enable
using Microsoft.UI.Xaml;
using Microsoft.Windows.AppLifecycle;
using WinRT;

namespace LibreNT.Shell;

public class Program
{
    [STAThread]
    static int Main(string[] args)
    {
        WinRT.ComWrappersSupport.InitializeComWrappers();

        var app = new App();
        
        var window = new MainWindow();
        window.Activate();
        
        return 0;
    }
}
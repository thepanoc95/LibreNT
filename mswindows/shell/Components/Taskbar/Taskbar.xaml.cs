/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Windows shell component
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media;

namespace LibreNT.Shell.Components.Taskbar;

public sealed partial class Taskbar : UserControl
{
    public event EventHandler? StartButtonClicked;

    public Taskbar()
    {
        InitializeComponent();
    }

    private void OnStartButtonClick(object sender, RoutedEventArgs e)
    {
        StartButtonClicked?.Invoke(this, EventArgs.Empty);
    }

    public void AddTaskItem(string title, string appId, Action? onClick = null)
    {
        var button = new Button
        {
            Content = title,
            Tag = appId,
            Style = Application.Current.Resources["ButtonRevealStyle"] as Style,
            MinWidth = 120,
            Height = 36,
            Margin = new Thickness(2)
        };

        if (onClick != null)
        {
            button.Click += (s, e) => onClick();
        }

        TaskListPanel.Children.Add(button);
    }

    public void ClearTasks()
    {
        TaskListPanel.Children.Clear();
    }

    public void SetSystemTray(UIElement trayElement)
    {
        SystemTrayArea.Children.Clear();
        SystemTrayArea.Children.Add(trayElement);
    }
}
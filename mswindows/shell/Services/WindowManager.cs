/*
 * PROJECT:     LibreNT
 * LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 * PURPOSE:     Windows shell component
 * COPYRIGHT:   Copyright 2026 Panoc95
 */



using Microsoft.UI.Xaml;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace LibreNT.Shell.Services;

public class WindowInfo : INotifyPropertyChanged
{
    private string _title = string.Empty;
    private bool _isActive;
    private string _appId = string.Empty;

    public string Title
    {
        get => _title;
        set => SetProperty(ref _title, value);
    }

    public bool IsActive
    {
        get => _isActive;
        set => SetProperty(ref _isActive, value);
    }

    public string AppId
    {
        get => _appId;
        set => SetProperty(ref _appId, value);
    }

    public IntPtr HWND { get; set; }

    public event PropertyChangedEventHandler? PropertyChanged;

    private bool SetProperty<T>(ref T storage, T value, [CallerMemberName] string? propertyName = null)
    {
        if (Equals(storage, value)) return false;
        storage = value;
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        return true;
    }
}

public class WindowManager : INotifyPropertyChanged
{
    private readonly ObservableCollection<WindowInfo> _windows = new();
    private WindowInfo? _activeWindow;

    public ObservableCollection<WindowInfo> Windows => _windows;

    public WindowInfo? ActiveWindow
    {
        get => _activeWindow;
        set => SetProperty(ref _activeWindow, value);
    }

    public event PropertyChangedEventHandler? PropertyChanged;
    public event EventHandler<WindowInfo>? WindowAdded;
    public event EventHandler<WindowInfo>? WindowRemoved;

    public WindowManager()
    {
        // Start monitoring windows
        StartWindowMonitoring();
    }

    private void StartWindowMonitoring()
    {
        // In a real implementation, this would use Win32 APIs
        // to enumerate and monitor windows
    }

    public void AddWindow(WindowInfo window)
    {
        _windows.Add(window);
        WindowAdded?.Invoke(this, window);
        OnPropertyChanged(nameof(Windows));
    }

    public void RemoveWindow(WindowInfo window)
    {
        _windows.Remove(window);
        WindowRemoved?.Invoke(this, window);
        OnPropertyChanged(nameof(Windows));
    }

    public void UpdateActiveWindow(WindowInfo window)
    {
        ActiveWindow = window;
    }

    private bool SetProperty<T>(ref T storage, T value, [CallerMemberName] string? propertyName = null)
    {
        if (Equals(storage, value)) return false;
        storage = value;
        OnPropertyChanged(propertyName);
        return true;
    }

    protected virtual void OnPropertyChanged([CallerMemberName] string? propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}
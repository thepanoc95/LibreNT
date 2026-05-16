using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media;
using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Timers;

namespace LibreNT.Shell.Components.SystemTray;

public sealed partial class SystemTray : UserControl, INotifyPropertyChanged
{
    private System.Timers.Timer? _timer;
    private string _currentTime = string.Empty;

    public string CurrentTime
    {
        get => _currentTime;
        private set => SetProperty(ref _currentTime, value);
    }

    public event PropertyChangedEventHandler? PropertyChanged;

    public SystemTray()
    {
        InitializeComponent();
        InitializeTimer();
        UpdateTime();
    }

    private void InitializeTimer()
    {
        _timer = new System.Timers.Timer(1000);
        _timer.Elapsed += (s, e) => UpdateTime();
        _timer.Start();
    }

    private void UpdateTime()
    {
        CurrentTime = DateTime.Now.ToString("h:mm tt");
    }

    private bool SetProperty<T>(ref T storage, T value, [CallerMemberName] string? propertyName = null)
    {
        if (Equals(storage, value)) return false;
        storage = value;
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        return true;
    }

    private void NetworkButton_Click(object sender, RoutedEventArgs e)
    {
        // Show network settings
    }

    private void VolumeButton_Click(object sender, RoutedEventArgs e)
    {
        // Show volume mixer
    }

    private void BatteryButton_Click(object sender, RoutedEventArgs e)
    {
        // Show battery settings
    }

    private void ClockButton_Click(object sender, RoutedEventArgs e)
    {
        // Show calendar/notification center
    }
}
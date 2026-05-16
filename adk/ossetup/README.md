# LibreNT Installer

A C++ ImGui-based installer (patcher) for LibreNT, a Windows Core OS customization.

## Overview

The LibreNT Installer provides a modern, streamlined installation experience for LibreNT. It uses ImGui for the graphical interface and Windows Deployment tools (DISM) for system modifications.

## Features

- **ImGui-based UI**: Modern, native Windows interface
- **Drive Selection**: Choose target installation drive from available fixed drives
- **Component Selection**: Choose which LibreNT components to install
- **BTRFS Driver Injection**: Install BTRFS filesystem support
- **Quibble Bootloader**: Install the Quibble EFI bootloader
- **Registry Patches**: Apply system-wide registry modifications
- **macOS-style Verbose Boot**: Diagnostic mode showing detailed boot messages

## Components

| Component | Description | Required |
|-----------|-------------|----------|
| BTRFS Drivers | BTRFS filesystem drivers | Yes |
| Quibble Bootloader | EFI bootloader for LibreNT | Yes |
| LibreNT Explorer | WinUI 3 shell replacement | No |
| Registry Patches | System configuration changes | No |
| Performance Tweaks | System optimizations | No |

## Building

### Requirements

- Visual Studio 2022 or MSVC compiler, **or**
- mingw-w64 cross-compiler (Linux)
- CMake 3.16+
- Windows SDK 10.0+ (native Windows only)
- DirectX 11 (native Windows only)

### Windows Build (Visual Studio/MSVC)

```powershell
cd adk/ossetup
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Linux Build (mingw-w64 Cross-Compilation)

```bash
# Install mingw-w64 toolchain (Ubuntu/Debian)
sudo apt-get install mingw-w64 g++-mingw-w64-x86-64

# Install DirectX headers for mingw (required for D3D11)
# Note: On some distributions, you may need to copy d3d11.h and dxgi.h
# from Windows SDK or use a package that provides Windows-compatible headers

# Build
cd adk/ossetup
mkdir build-win64
cd build-win64
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-w64-x86_64.cmake
cmake --build . --config Release
```

### DirectX Headers for mingw-w64

For mingw-w64 cross-compilation, you need DirectX 11 headers:
```bash
# On Ubuntu/Debian, you can copy from Windows SDK or use:
# The mingw-w64 package includes basic DX headers, but for full D3D11 support:
# 1. Download Windows SDK headers
# 2. Extract d3d11.h, dxgi.h to /usr/x86_64-w64-mingw32/include/
```

## Usage

1. Run `LibreNTInstaller.exe` as Administrator
2. Select target installation drive
3. Choose components to install
4. Click "Start Installation"
5. Follow progress in the installer window

## Directory Structure

```
adk/ossetup/
├── src/                    # Source files
│   ├── main.cpp           # Entry point
│   ├── ImGuiImpl.cpp/h    # DirectX/Win32 ImGui backend
│   ├── InstallerUI.cpp/h  # Main installer UI
│   ├── VerboseBoot.cpp/h  # Verbose boot window
│   ├── DeploymentEngine.cpp/h  # Windows image deployment
│   ├── RegistryPatcher.cpp/h   # Registry modifications
│   └── DriverInjector.cpp/h    # Driver injection
├── include/               # Header files
├── assets/                # UI assets and logo
└── CMakeLists.txt         # Build configuration
```

## Technical Details

### ImGui Implementation

The installer uses Dear ImGui with DirectX 11 backend for rendering:
- Custom dark theme matching LibreNT branding
- Native Windows message loop integration
- Fullscreen verbose boot overlay

### Deployment Engine

Uses Windows DISM API for:
- Mounting Windows images (install.wim/install.esd)
- Applying registry hives
- Copying files to mounted image
- Committing changes and unmounting

### Registry Modifications

- Shell replacement (explorer.exe → LibreNT Explorer)
- Windows Defender disable
- Telemetry disable
- Performance optimizations

### Driver Injection

- Copies BTRFS driver files to System32\Drivers
- Installs Quibble bootloader to EFI partition
- Updates BCD boot configuration

## License

Part of the LibreNT project. See main LICENSE file.
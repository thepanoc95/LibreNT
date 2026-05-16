<!--
 PROJECT:     LibreNT
 LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
 PURPOSE:     Project documentation
 COPYRIGHT:   Copyright 2026 Panoc95
-->



<img width="150" height="150" align="left" style="float: left; margin: 0 10px 0 0;" alt="LibreNT logo" src="./librentlogo.png">   

<br>
<br>
<br>
<br>

# LibreNT
WindowsNT in the hands of freedom

## Development

### Nix Flake

This project includes a Nix flake for development environments:

```bash
# Enter development shell
nix develop

# Or with specific shell
nix develop .#windows  # Windows cross-compilation environment
```

### Building

```bash
# Build using make
make build-release

# Build installer
nix run .#build-installer
```

### Structure

- `adk/ossetup/` - Windows installer (ImGui + DirectX 11)
- `libredaemon/` - Rust daemon for LibreNT
- `drivers/` - BTRFS and other Windows drivers 

### Utility Scripts

- `utils/Enable-ConsoleLogon.ps1` - toggles classic/console-style logon fallback by disabling or restoring `authui.dll` (supports online and offline Windows roots).
- `tools/insert_file_banner.py` - insert the standard file banner header into supported source files, with `PURPOSE` selected based on file type and location.

Example:

```powershell
# Apply on current system (Admin shell)
.\utils/Enable-ConsoleLogon.ps1

# Apply to offline image mounted at D:\
.\utils/Enable-ConsoleLogon.ps1 -WindowsRoot 'D:\Windows'

# Restore
.\utils/Enable-ConsoleLogon.ps1 -WindowsRoot 'D:\Windows' -Restore
```

To preview banner insertion before modifying files:

```bash
python3 tools/insert_file_banner.py --dry-run
```

To apply the banner to all detected files:

```bash
python3 tools/insert_file_banner.py --apply
```

> [!NOTE]
> I will accept **any** contributions to this project, even if it's just a typo fix, a driver update, or a new feature.

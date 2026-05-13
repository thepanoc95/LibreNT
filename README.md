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
> [!NOTE]
> I will accept **any** contributions to this project, even if it's just a typo fix, a driver update, or a new feature.

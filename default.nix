# default.nix - Non-flake Nix build for LibreNT
# Can be used with `nix-build` or `nix develop`

{ pkgs ? import <nixpkgs> { } }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    # Rust
    (rust-bin.stable.latest.default.override {
      extensions = [ "rust-src" "rust-analyzer" ];
    })
    cargo-edit
    cargo-watch
    cmake
    ninja

    # MinGW for Windows cross-compilation
    mingw-w64

    # Development tools
    git
    ripgrep
    jq
    curl
  ];

  shellHook = ''
    echo "LibreNT Development Environment"
    echo "================================"
    rustc --version
    echo ""
    echo "Windows cross-compilation:"
    x86_64-w64-mingw32-gcc --version | head -1
  '';
}

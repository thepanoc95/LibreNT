{
  description = "LibreNT - WindowsNT in the hands of freedom";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    rust-overlay.url = "github:oxalica/rust-overlay";
  };

  outputs = { self, nixpkgs, flake-utils, rust-overlay }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        overlays = [ rust-overlay.overlays.default ];
        pkgs = import nixpkgs {
          inherit system overlays;
        };

        # Rust toolchain for the project
        rustToolchain = pkgs.rust-bin.fromRustupToolchainFile ./rust-toolchain.toml;

        # MinGW-w64 for Windows cross-compilation
        mingwPkgs = pkgs.pkgsCross.mingwW64;

        # Native packages for development
        nativePkgs = with pkgs; [
          rustToolchain
          rust-analyzer
          cargo-edit
          cargo-watch
          cmake
          ninja
          pkg-config
        ];

        # MinGW packages for Windows cross-compilation
        mingwPkgsList = with mingwPkgs; [
          stdenv.cc
          pkg-config
        ];

        # Development shell packages
        devShellPackages = nativePkgs ++ (with pkgs; [
          git
          ripgrep
          jq
          curl
          wget
          gnumake
          clang-tools
          python3
        ]);

        # LibreNT installer build (requires MinGW)
        libreNTInstaller = mingwPkgs.stdenv.mkDerivation {
          pname = "librent-installer";
          version = "0.1.0";
          src = pkgs.lib.cleanSource ./adk/ossetup;

          nativeBuildInputs = [
            mingwPkgs.cmake
            mingwPkgs.stdenv.cc
            pkgs.ninja
          ];

          dontWrapQtApps = true;

          buildPhase = ''
            cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64-x86_64.cmake -G Ninja
            cmake --build build
          '';

          installPhase = ''
            mkdir -p $out/bin
            if [ -f build/LibreNTInstaller.exe ]; then
              cp build/LibreNTInstaller.exe $out/bin/
            fi
          '';

          meta = with pkgs.lib; {
            description = "LibreNT Windows installer";
            platforms = platforms.windows ++ platforms.linux;
          };
        };

      in
      {
        # Development shells
        devShells.default = pkgs.mkShell {
          buildInputs = devShellPackages;

          shellHook = ''
            echo "LibreNT Development Environment"
            echo "================================"
            echo "Rust toolchain: $(rustc --version)"
            echo ""
            echo "Available commands:"
            echo "  cargo build    - Build Rust workspace"
            echo "  cargo test     - Run Rust tests"
            echo "  cargo clippy   - Run linter"
            echo ""
            echo "Windows cross-compilation:"
            echo "  x86_64-w64-mingw32-gcc --version"
            echo ""
          '';

          # Environment variables
          RUST_SRC_PATH = "${rustToolchain}/lib/rustlib/src/rust/library";
          RUST_LOG = "debug";
        };

        devShells.windows = pkgs.mkShell {
          buildInputs = devShellPackages ++ mingwPkgsList;
          shellHook = ''
            echo "LibreNT Windows Cross-Compilation Environment"
            echo "============================================="
            echo "MinGW-w64: $(x86_64-w64-mingw32-gcc --version | head -1)"
          '';
        };

        # Packages
        packages = {
          default = libreNTInstaller;
          installer = libreNTInstaller;

          # Rust workspace
          rust-workspace = pkgs.rustPlatform.buildRustPackage {
            pname = "librent";
            version = "0.1.0";
            src = ./.;
            cargoLock = {
              lockFile = ./Cargo.lock;
            };
            postPatch = "rm -f Cargo.lock";
          };
        };

        # Formatter
        formatter = pkgs.nixfmt-rfc-style;

        # Apps
        apps = {
          # Build installer app
          build-installer = {
            type = "app";
            program = "${pkgs.writeShellScriptBin "build-installer" ''
              set -e
              cd ${self}
              mkdir -p adk/ossetup/build
              ${mingwPkgs.cmake}/bin/cmake -B adk/ossetup/build \
                -DCMAKE_TOOLCHAIN_FILE=adk/ossetup/cmake/mingw-w64-x86_64.cmake \
                -G Ninja adk/ossetup
              ${mingwPkgs.cmake}/bin/cmake --build adk/ossetup/build
            ''}/bin/build-installer";
          };
        };
      });
}

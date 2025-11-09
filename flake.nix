{
  description = "A Nix-flake-based C/C++ development environment";

  inputs.nixpkgs.url = "https://flakehub.com/f/NixOS/nixpkgs/0";

  outputs = { self, nixpkgs, ... }@inputs:
    let
      supportedSystems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];

      forEachSupportedSystem = f:
        nixpkgs.lib.genAttrs supportedSystems (system:
          let
            pkgs = import nixpkgs { inherit system; };
          in
          f { inherit system pkgs; });
    in
    {
      devShells = forEachSupportedSystem ({ pkgs, system }: {
        default = pkgs.mkShell {
          # Uncomment to switch toolchain:
          # stdenv = pkgs.clangStdenv;

          # Tools needed at configure/build time
          nativeBuildInputs = with pkgs; [
            pkg-config
            cmake
            clang-tools
            codespell
            cppcheck
            doxygen
            gtest
            lcov
            vcpkg
            vcpkg-tool
          ];

          # Libraries available for compilation/linking
          buildInputs = with pkgs; [
            # OpenGL
            libGL
            libGLU
            mesa

            # X11 stack (for GLFW/Xwayland, etc.)
            xorg.libX11
            xorg.libXrandr
            xorg.libXinerama
            xorg.libXcursor
            xorg.libXi
            xorg.libXext
            xorg.libXxf86vm
            xorg.xorgproto

            # Wayland stack
            wayland
            wayland-protocols
            wayland-scanner
            libxkbcommon
          ] ++ (if system == "aarch64-darwin" then [ ] else [ pkgs.gdb ]);
        };
      });
    };
}

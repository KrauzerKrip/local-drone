{
  description = "A Nix-flake-based C/C++ development environment";

  inputs.nixpkgs.url = "https://flakehub.com/f/NixOS/nixpkgs/0"; # stable Nixpkgs

  outputs =
    { self, ... }@inputs:

    let
      system = "x86_64-linux";
      pkgs = import inputs.nixpkgs { inherit system; };

      supportedSystems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];
      forEachSupportedSystem =
        f:
        inputs.nixpkgs.lib.genAttrs supportedSystems (
          system:
          f {
            pkgs = import inputs.nixpkgs { inherit system; };
          }
        );
    in
    {
      packages.${system}.game = pkgs.stdenv.mkDerivation {
        pname = "game";
        version = "0.1.0";

         src = ./.;

         nativeBuildInputs = with pkgs; [
           clang-tools
           cmake
           cppcheck
           ninja
           pkg-config
         ];
         buildInputs = with pkgs; [
           clang-tools
           cmake
           codespell
           conan
           cppcheck
           doxygen
           gtest
           lcov
           vcpkg
           vcpkg-tool
           ninja
           pkg-config

           lua
           glfw
           assimp
           entt
           freetype
           glm
           imgui
           nlohmann_json
           pugixml
           luabridge

           xorg.libX11
           xorg.libXext
           xorg.libXrandr
           xorg.libXrender
           xorg.libXfixes
           xorg.libXcursor
           xorg.libXi

           mesa
           libGL
           libGLU
         ];
      };

      devShells = forEachSupportedSystem (
        { pkgs }:
        {
          default =
            pkgs.mkShell.override
              {
                # Override stdenv in order to change compiler:
                stdenv = pkgs.clangStdenv;
              }
              {
                packages =
                  with pkgs;
                  [
                    clang-tools
                    cmake
                    codespell
                    conan
                    cppcheck
                    doxygen
                    gtest
                    lcov
                    vcpkg
                    vcpkg-tool
                    ninja
                    pkg-config

                    lua
                    glfw
                    assimp
                    entt
                    freetype
                    glm
                    imgui
                    nlohmann_json
                    pugixml
                    luabridge
                    spdlog

                    xorg.libX11
                    xorg.libXext
                    xorg.libXrandr
                    xorg.libXrender
                    xorg.libXfixes
                    xorg.libXcursor
                    xorg.libXi

                    mesa
                    libGL
                    libGLU
                  ]
                  ++ (if system == "aarch64-darwin" then [ ] else [ gdb ]);
              };
        }
      );
    };
}

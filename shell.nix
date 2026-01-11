{ pkgs ? import <nixpkgs> {} }:
  pkgs.mkShell {
    # nativeBuildInputs is usually what you want -- tools you need to run
    nativeBuildInputs = with pkgs.buildPackages; [
    
    bullet
    pkgconf
    pkg-config
    meson
    cmake
    vulkan-headers
    vulkan-tools
    vulkan-loader
    vulkan-validation-layers
    vulkan-extension-layer
    glfw
    glm
    assimp
    ninja
  ];
}


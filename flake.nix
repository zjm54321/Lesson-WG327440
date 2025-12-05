{
  description = "A Nix-flake-based 8051 development environment";

  inputs.nixpkgs.url = "https://flakehub.com/f/NixOS/nixpkgs/0.1";

  outputs =
    inputs:
    let
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
      devShells = forEachSupportedSystem (
        { pkgs }:
        let
          stcflash = pkgs.callPackage ./nix/stcflash.nix { };
          stcgal = pkgs.callPackage ./nix/stcgal.nix { };
          naken_asm = pkgs.callPackage ./nix/naken_asm.nix { };
          emu8051 = pkgs.callPackage ./nix/emu8051.nix { };
        in
        {
          default = pkgs.mkShell {

            packages = [
              # tools
              pkgs.xmake
              pkgs.clang-tools

              # compiler
              naken_asm
              pkgs.sdcc

              # emulator
              emu8051

              # programmer
              stcflash
              stcgal

            ];
          };
        }
      );
    };
}

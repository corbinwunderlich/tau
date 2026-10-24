{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";

    flake-parts.url = "github:hercules-ci/flake-parts";

    systems.url = "github:nix-systems/default";

    git-hooks = {
      url = "github:cachix/git-hooks.nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = {
    flake-parts,
    systems,
    git-hooks,
    ...
  } @ inputs:
    flake-parts.lib.mkFlake {inherit inputs;} {
      imports = [git-hooks.flakeModule];

      systems = import systems;

      perSystem = {
        pkgs,
        config,
        self',
        ...
      }: {
        packages = {
          tau = pkgs.callPackage ./default.nix {};
          default = self'.packages.tau;
        };

        pre-commit.settings = {
          hooks = {
            alejandra.enable = true;

            deadnix.enable = true;

            statix.enable = true;

            mdformat.enable = true;

            clang-format.enable = true;

            cmake-format = {
              enable = true;
              settings = {
                inPlace = true;
              };
            };
          };
        };

        devShells.default = pkgs.mkShell {
          packages = with pkgs; [clang cmake ninja pkg-config catch2_3] ++ config.pre-commit.settings.enabledPackages;

          CMAKE_GENERATOR = "Ninja";

          ASAN_OPTIONS = "detect_leaks=1:exitcode=1";

          shellHook = ''
            export CC=${pkgs.clang}/bin/clang
            export CXX=${pkgs.clang}/bin/clang++

            ${config.pre-commit.shellHook}
          '';
        };
      };
    };
}

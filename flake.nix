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

            clang-tidy = let
              build = pkgs.runCommandCC "cmake-build" {buildInputs = [pkgs.cmake pkgs.catch2_3];} ''
                cmake ${./.} -B $out
              '';
            in {
              enable = true;
              entry = "${pkgs.clang-tools}/bin/clang-tidy -p ${build} --fix";
            };

            cmake-format = {
              enable = true;
              settings = {
                inPlace = true;
              };
            };

            tests = let
              script = let
                cmake = "${pkgs.cmake}/bin/cmake";
                ctest = "${pkgs.cmake}/bin/ctest";
              in
                pkgs.writeShellScript "tests" ''
                  ${cmake} -B build
                  ${cmake} --build build
                  ${ctest} --test-dir build
                '';
            in {
              enable = true;
              entry = toString script;
              pass_filenames = false;
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

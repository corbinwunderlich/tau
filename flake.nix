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

              script = pkgs.writeShellScript "clang-tidy" ''
                ${pkgs.libclang}/bin/run-clang-tidy -j $(${pkgs.getconf}/bin/getconf _NPROCESSORS_ONLN) -fix -p ${build}
              '';
            in {
              enable = true;
              entry = toString script;
              require_serial = true;
            };

            gersemi = let
              script = pkgs.writeShellScript "gersemi" ''
                catch2_defs=$(find ${pkgs.catch2_3}/lib/cmake/Catch2)

                ${pkgs.gersemi}/bin/gersemi --in-place --list-expansion favour-expansion --definitions $catch2_defs $@
              '';
            in {
              enable = true;
              entry = toString script;
              files = "\\.cmake$|CMakeLists.txt";
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
          CTEST_OUTPUT_ON_FAILURE = 1;

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

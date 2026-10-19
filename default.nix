{
  stdenv,
  cmake,
  ...
}: let
  version = builtins.readFile ./.version;
in
  stdenv.mkDerivation {
    pname = "tau";
    inherit version;

    src = ./.;

    nativeBuildInputs = [cmake];

    installPhase = ''
      mkdir -p $out/bin

      cp -ar ./tau $out/bin/tau
    '';
  }

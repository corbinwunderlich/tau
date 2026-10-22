{
  stdenv,
  cmake,
  catch2_3,
  ...
}: let
  version = builtins.readFile ./.version;
in
  stdenv.mkDerivation {
    pname = "tau";
    inherit version;

    src = ./.;

    nativeBuildInputs = [cmake];
    buildInputs = [catch2_3];

    installPhase = ''
      mkdir -p $out/bin

      cp -ar ./apps/tau $out/bin/tau
    '';
  }

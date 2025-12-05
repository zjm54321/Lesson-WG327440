{
  stdenv,
  fetchFromGitHub,
}:

stdenv.mkDerivation rec {
  pname = "naken_asm";
  version = "b6e83f1976a5fa0b1a371bd4d6db935a386b95ef";

  src = fetchFromGitHub {
    owner = "mikeakohn";
    repo = "naken_asm";
    rev = version;
    hash = "sha256-ZINfqz3M6zMVHQ/YJuthB3uOULxQWsqsp4lwoZ22sQ0=";
    fetchSubmodules = false;
  };

  # 让 shebang 从 /usr/bin/env 改写到 Nix store 中的解释器
  postPatch = ''
    patchShebangs .
  '';

  # 显式执行上游的 configure（它不是 GNU autoconf）
  configurePhase = ''
    runHook preConfigure
    ./configure --install-prefix=$out --include-path=$out/share/naken_asm/include
    runHook postConfigure
  '';

  enableParallelBuilding = true;

  buildPhase = ''
    runHook preBuild
    make -j$NIX_BUILD_CORES
    runHook postBuild
  '';

  installPhase = ''
    runHook preInstall
    make install
    runHook postInstall
  '';

  meta = {
    description = "A simple assembler, disassembler, and simulator for many CPUs. For full description, usage, and a list of supported CPUs";
    homepage = "https://www.mikekohn.net/micro/naken_asm.php";
  };
}

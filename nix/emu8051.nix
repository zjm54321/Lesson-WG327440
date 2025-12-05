{
  lib,
  stdenv,
  pkgs,
  fetchFromGitHub,
}:

stdenv.mkDerivation rec {
  pname = "emu8051";
  version = "ce9c4d0623bd6b141185a152cf9f3fa4ecedd6d9";

  src = fetchFromGitHub {
    owner = "cjacker";
    repo = "emu8051";
    rev = version;
    hash = "sha256-6WUZ+glI3Q5bA7CFc/F+5OhpuUpjVpwiDaem1IYcuVs=";
    fetchSubmodules = false;
  };

  enableParallelBuilding = true;
  buildInputs = [ pkgs.ncurses5 ];

  postPatch = ''
    # 上游链接 -lcurses，在 Nix 中不存在该别名；改为 -lncurses（需要宽字符可用 -lncursesw）
    substituteInPlace Makefile --replace "-lcurses" "-lncurses"
  '';

  installPhase = ''
    runHook preInstall
    if [ -f emu8051 ]; then
      install -Dm755 emu8051 -t $out/bin
    elif [ -f emu ]; then
      install -Dm755 emu -T $out/bin/emu8051
    else
      echo "no built binary found" >&2
      exit 1
    fi
    runHook postInstall
  '';

  meta = with lib; {
    description = "8051/8052 emulator with curses-based UI ";
    homepage = "https://github.com/cjacker/emu8051";
    license = licenses.mit;
  };
}

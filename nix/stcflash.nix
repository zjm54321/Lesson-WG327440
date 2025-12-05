{
  lib,
  stdenv,
  fetchFromGitHub,
  python3,
}:

let
  # 带 pyserial 的解释器，用于运行 stcflash
  py = python3.withPackages (ps: [ ps.pyserial ]);
in
stdenv.mkDerivation rec {
  pname = "stcflash";
  version = "2df4600d67e33ec4d2faaba869b011d8b36566b8";

  src = fetchFromGitHub {
    owner = "sms-wyt";
    repo = "stcflash";
    rev = version;
    hash = "sha256-r/20gQCXD0Wov50UKGCLUdu8G4qkzUEO5wak1e1INqI=";
  };

  nativeBuildInputs = [ python3 ];

  postPatch = ''
    patchShebangs .
  '';

  installPhase = ''
    runHook preInstall
    install -Dm0755 stcflash.py $out/bin/stcflash
    # 写入/覆盖 shebang,使用带 pyserial 的解释器
    if head -n1 $out/bin/stcflash | grep -q '^#!'; then
      sed -i '1s|^#!.*|#!${py}/bin/python|' $out/bin/stcflash
    else
      sed -i '1i #!${py}/bin/python' $out/bin/stcflash
    fi
    runHook postInstall
  '';

  meta = with lib; {
    description = "A command line programmer for STC 8051 microcontroller.";
    homepage = "https://github.com/sms-wyt/stcflash";
    license = licenses.gpl3Plus;
  };
}

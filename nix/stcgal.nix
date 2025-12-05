{ pkgs }:
with pkgs;
with python3Packages;

buildPythonPackage rec {
  pname = "stcgal";
  version = "1.10";
  format = "setuptools";

  src = fetchPypi {
    inherit pname version;
    sha256 = "31db4f7ee1a1eb193941c6e4728fbb47874a467b02128a7d3915aaf60da658f7";
  };

  # PyPI 源缺 doc/PyPI.md，这里从 GitHub 补上

  preBuild =
    let
      md = fetchurl {
        url = "https://raw.githubusercontent.com/grigorig/stcgal/refs/heads/master/doc/PyPI.md";
        sha256 = "b0NoOoyoDrPLTLA/0qxsStEvvsQbKhW0HggXSaVTY9I=";
      };
    in
    ''
      mkdir -p doc
      cp ${md} doc/PyPI.md
    '';

  propagatedBuildInputs = [
    pyserial
    tqdm
    pyusb
  ];

  pythonImportsCheck = [ "stcgal" ];
  doCheck = false;

  meta = {
    description = "STC 8051/15 microcontroller flash tool";
    homepage = "https://github.com/grigorig/stcgal";
  };
}

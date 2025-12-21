{
  description = "My App with modified FTXUI";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in {
        # 1. Ваше главное приложение — 'default'
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "my-app";
          version = "0.1.0";
          src = ./.;

          nativeBuildInputs = [ pkgs.cmake pkgs.pkg-config ];
          buildInputs = [ pkgs.fmt.dev pkgs.crow pkgs.boost.dev pkgs.opencv ];

          cmakeFlags = [ "-DCMAKE_BUILD_TYPE=Release" ];
          installPhase = "mkdir -p $out/bin && cp my-app $out/bin/";
        };

        # 2. Окружение для разработки
        devShells.default = pkgs.mkShell {
          nativeBuildInputs = with pkgs; [ cmake pkg-config clang-tools ];
          buildInputs = [ pkgs.fmt.dev pkgs.crow pkgs.boost.dev pkgs.opencv ];
          CXXFLAGS = "-DCROW_USE_BOOST";

          shellHook = '' '';
        };

        apps.default = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/my-app";
        };
      }
    );
}


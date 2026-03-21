{
  description = "FWUI - Declarative HTML generation (Ruby DSL + C++20 core)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        lib  = pkgs.lib;
        isDarwin = pkgs.stdenv.isDarwin;

        cleanSrc = lib.cleanSourceWith {
          src = ./.;
          filter = path: type:
            let b = builtins.baseNameOf path; in
            !(b == "build" || b == "result" || b == ".git" || b == ".direnv");
        };

        buildDeps  = with pkgs; [ fmt nlohmann_json ];
        nativeDeps = with pkgs; [ cmake pkg-config ];
        ncpuCmd = if isDarwin then "sysctl -n hw.ncpu" else "nproc";

        fwui = pkgs.stdenv.mkDerivation {
          pname   = "fwui";
          version = "1.0.0";
          src     = cleanSrc;

          nativeBuildInputs = nativeDeps;
          buildInputs       = buildDeps;

          cmakeFlags = [ "-DCMAKE_BUILD_TYPE=Release" ];

          meta = {
            description = "Declarative HTML generation - Ruby DSL + C++20 core";
            license     = lib.licenses.asl20;
          };
        };

        # --- nix run apps ---

        fwui-server = pkgs.writeShellApplication {
          name = "fwui-server";
          runtimeInputs = [ pkgs.ruby ];
          text = ''exec ruby ${./examples/ruby/server.rb} "$@"'';
        };

        fwui-build = pkgs.writeShellApplication {
          name = "fwui-build";
          runtimeInputs = nativeDeps ++ buildDeps;
          text = ''
            BUILD_DIR="build"
            if [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
              echo ">>> Configuring Release build..."
              cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
            fi
            cmake --build "$BUILD_DIR" -j"$(${ncpuCmd})"
            echo "=== Build complete ==="
          '';
        };

        fwui-test = pkgs.writeShellApplication {
          name = "fwui-test";
          runtimeInputs = nativeDeps ++ buildDeps;
          text = ''
            BUILD_DIR="build"
            if [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
              echo ">>> Configuring build with tests..."
              cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DFWUI_BUILD_TESTS=ON
            fi
            cmake --build "$BUILD_DIR" -j"$(${ncpuCmd})"
            ctest --test-dir "$BUILD_DIR" --output-on-failure
          '';
        };

      in {
        packages.default = fwui;

        apps = {
          default = {
            type    = "app";
            program = "${fwui-server}/bin/fwui-server";
          };
          dev = {
            type    = "app";
            program = toString (pkgs.writeShellScript "fwui-dev" ''
              exec ${pkgs.ruby}/bin/ruby ${./examples/ruby/server.rb} --dev
            '');
          };
          build = {
            type    = "app";
            program = "${fwui-build}/bin/fwui-build";
          };
          test = {
            type    = "app";
            program = "${fwui-test}/bin/fwui-test";
          };
          demo = {
            type    = "app";
            program = toString (pkgs.writeShellScript "fwui-demo" ''
              exec ${pkgs.ruby}/bin/ruby ${./examples/ruby/demo.rb}
            '');
          };
        };

        devShells.default = pkgs.mkShell {
          inputsFrom = [ fwui ];
          packages   = [ pkgs.ruby ]
            ++ lib.optionals (!isDarwin) [ pkgs.gdb pkgs.valgrind ];

          shellHook = ''
            echo ""
            echo "FWUI devShell (${if isDarwin then "macOS" else "Linux"})"
            echo "  nix run           - Ruby server"
            echo "  nix run .#dev     - Ruby server (hot-reload)"
            echo "  nix run .#build   - cmake build"
            echo "  nix run .#test    - cmake build + tests"
            echo "  nix run .#demo    - Ruby demo"
            echo ""
          '';
        };
      }
    );
}

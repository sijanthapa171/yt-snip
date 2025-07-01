{
  description = "yt-snip - Fast YouTube video/audio downloader and trimmer";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "yt-snip";
          version = "1.0.0";

          src = ./.;

          nativeBuildInputs = with pkgs; [
            gcc
            gnumake
          ];

          buildInputs = with pkgs; [
            yt-dlp
            ffmpeg
          ];

          buildPhase = ''
            $CXX -std=c++17 -Wall -Wextra -O2 -o yt-snip main.cc
          '';

          installPhase = ''
            mkdir -p $out/bin
            install -D -m 755 yt-snip $out/bin/yt-snip
            install -D -m 644 README.md $out/share/doc/yt-snip/README.md
            install -D -m 644 LICENSE $out/share/licenses/yt-snip/LICENSE
          '';

          meta = with pkgs.lib; {
            description = "Fast YouTube video/audio downloader and trimmer - a yt-dlp wrapper";
            homepage = "https://github.com/melqtx/yt-snip";
            license = licenses.mit;
            maintainers = [ "melqtx" ];
            platforms = platforms.all;
          };
        };

        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            gcc
            gnumake
            yt-dlp
            ffmpeg
          ];
        };

        apps.default = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/yt-snip";
        };
      }
    );
}

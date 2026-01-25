{
  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
    pebble.url = "github:anna-oake/pebble.nix/sdk-core";
  };

  outputs =
    {
      self,
      pebble,
      flake-utils,
      ...
    }:
    let
      packageJson = builtins.fromJSON (builtins.readFile ./package.json);
      inherit (pebble) mkAppInstallPbw buildPebbleApp pebbleEnv;
    in
    flake-utils.lib.eachDefaultSystem (system: {
      apps.default = mkAppInstallPbw.${system} {
        pbwPackage = self.packages.${system}.default;
        withLogs = true;
      };

      packages.default = buildPebbleApp.${system} {
        inherit (packageJson) name version;
        src = ./.;
      };

      devShell = pebbleEnv.${system} { };
    });
}

#!/usr/bin/env sh
set -eu
test -f release/provenance/manifest.json
sh scripts/package_release.sh --root . --out dist

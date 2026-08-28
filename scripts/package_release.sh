#!/bin/sh
set -eu
ROOT=.
OUT=dist
while [ $# -gt 0 ]; do
  case "$1" in
    --root) ROOT="$2"; shift 2 ;;
    --out) OUT="$2"; shift 2 ;;
    *) echo "usage: $0 [--root ROOT] [--out OUT]" >&2; exit 2 ;;
  esac
done
VERSION="$(cat "$ROOT/VERSION" 2>/dev/null || printf unknown)"
case "$VERSION" in
  *cfrontend*) NAME="cmotive-$VERSION" ;;
  *) NAME="cmotive-${VERSION}-cfrontend" ;;
esac
mkdir -p "$OUT"
OUT_ABS="$(cd "$OUT" && pwd)"
TMP="${TMPDIR:-/tmp}/$NAME"
rm -rf "$TMP"
mkdir -p "$TMP"
( cd "$ROOT" && tar --exclude='./build' --exclude='./dist' --exclude='./.git' -cf - . ) | ( cd "$TMP" && tar -xf - )
( cd "$(dirname "$TMP")" && tar -czf "$OUT_ABS/$NAME.tar.gz" "$NAME" )
if command -v zip >/dev/null 2>&1; then
  ( cd "$(dirname "$TMP")" && zip -qr "$OUT_ABS/$NAME.zip" "$NAME" )
fi
rm -rf "$TMP"
echo "$OUT_ABS/$NAME.tar.gz"

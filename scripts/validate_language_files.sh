#!/bin/sh
set -eu
BIN_ARG="${1:-build/bin}"
EXEEXT="${2:-}"
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
case "$BIN_ARG" in
  /*) BIN_DIR="$BIN_ARG" ;;
  *) BIN_DIR="$ROOT/$BIN_ARG" ;;
esac
CMOTIVE="$BIN_DIR/cmotive$EXEEXT"
CMOTIVEPP="$BIN_DIR/cmotivepp$EXEEXT"
OUT="$ROOT/build/all_language_validate"
if [ ! -x "$CMOTIVE" ]; then
  echo "missing executable: $CMOTIVE" >&2
  exit 2
fi
if [ ! -x "$CMOTIVEPP" ]; then
  echo "missing executable: $CMOTIVEPP" >&2
  exit 2
fi
rm -rf "$OUT"
mkdir -p "$OUT/pp" "$OUT/obj" "$OUT/log"
(
  cd "$ROOT"
  find . \
    -path './build' -prune -o \
    -path './dist' -prune -o \
    -path './legacy' -prune -o \
    -path './.git' -prune -o \
    -type f \( -iname '*.cmot' -o -iname '*.cmtv' -o -iname '*.hmot' -o -iname '*.hmtv' \) -print | sort
) > "$OUT/language-files.list"
count=0
while IFS= read -r rel; do
  [ -n "$rel" ] || continue
  count=$((count + 1))
  f="$ROOT/${rel#./}"
  safe=$(printf '%04d_%s' "$count" "${rel#./}" | sed 's#[^A-Za-z0-9_.-]#_#g')
  "$CMOTIVEPP" -I "$ROOT/examples/headers" -I "$ROOT/examples/packages" -I "$ROOT/lib" "$f" -o "$OUT/pp/$safe.pp.CMOT" >"$OUT/log/$safe.pp.out" 2>"$OUT/log/$safe.pp.err"
  "$CMOTIVE" -c -I "$ROOT/examples/headers" -I "$ROOT/examples/packages" -I "$ROOT/lib" "$f" -o "$OUT/obj/$safe.o" >"$OUT/log/$safe.cc.out" 2>"$OUT/log/$safe.cc.err"
  if [ $((count % 50)) -eq 0 ]; then
    echo "language files checked: $count"
  fi
done < "$OUT/language-files.list"
echo "CMotive language files: PASS ($count files)"

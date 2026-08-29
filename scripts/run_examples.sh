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
EXDIR="$ROOT/examples"
OUT="$ROOT/build/examples"
if [ ! -x "$CMOTIVE" ]; then
  echo "missing executable: $CMOTIVE" >&2
  exit 2
fi
if [ ! -x "$CMOTIVEPP" ]; then
  echo "missing executable: $CMOTIVEPP" >&2
  exit 2
fi
rm -rf "$OUT"
mkdir -p "$OUT/pp" "$OUT/bin" "$OUT/log" "$OUT/run"
find "$EXDIR" -maxdepth 1 -type f \( -iname '*.cmot' -o -iname '*.cmtv' \) -print | sort > "$OUT/examples.list"
count=0
while IFS= read -r f; do
  [ -n "$f" ] || continue
  count=$((count + 1))
  base=$(basename -- "$f")
  stem=${base%.*}
  safe=$(printf '%s' "$stem" | sed 's#[^A-Za-z0-9_.-]#_#g')
  "$CMOTIVEPP" -I "$EXDIR/headers" -I "$EXDIR/packages" -I "$ROOT/lib" "$f" -o "$OUT/pp/$safe.pp.CMOT" >"$OUT/log/$safe.pp.out" 2>"$OUT/log/$safe.pp.err"
  "$CMOTIVE" -I "$EXDIR/headers" -I "$EXDIR/packages" -I "$ROOT/lib" "$f" -o "$OUT/bin/$safe$EXEEXT" >"$OUT/log/$safe.cc.out" 2>"$OUT/log/$safe.cc.err"
  "$OUT/bin/$safe$EXEEXT" >"$OUT/run/$safe.out" 2>"$OUT/run/$safe.err" < /dev/null
  if [ $((count % 50)) -eq 0 ]; then
    echo "examples checked: $count"
  fi
done < "$OUT/examples.list"
echo "CMotive examples: PASS ($count examples)"

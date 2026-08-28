#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
MANIFEST="$ROOT/manifests/examples.jsonl"
COMPILER=${CMOTIVE:-cmotive}
PREPROCESSOR=${CMOTIVEPP:-cmotivepp}
TIMEOUT=${TIMEOUT:-5}
MODE=${1:-help}
shift || true
while [ $# -gt 0 ]; do
  case "$1" in
    --compiler) COMPILER="$2"; shift 2 ;;
    --preprocessor) PREPROCESSOR="$2"; shift 2 ;;
    --timeout) TIMEOUT="$2"; shift 2 ;;
    *) echo "unknown option: $1" >&2; exit 2 ;;
  esac
done
extract() {
  key="$1"
  sed -n "s/.*\"$key\"[ ]*:[ ]*\"\([^\"]*\)\".*/\1/p"
}
extract_num() {
  key="$1"
  sed -n "s/.*\"$key\"[ ]*:[ ]*\([0-9][0-9]*\).*/\1/p"
}
include_args() {
  printf '%s\n' -I "$ROOT/headers" -I "$ROOT" -I "${CMOTIVE_ROOT:-$ROOT/..}/lib"
}
compile_one() {
  src="$1"; out="$2"; objflag="$3"
  mkdir -p "$(dirname -- "$out")"
  if [ "$objflag" = "yes" ]; then
    "$COMPILER" -I "$ROOT/headers" -I "$ROOT" -I "${CMOTIVE_ROOT:-$ROOT/..}/lib" -c "$ROOT/$src" -o "$out"
  else
    "$COMPILER" -I "$ROOT/headers" -I "$ROOT" -I "${CMOTIVE_ROOT:-$ROOT/..}/lib" "$ROOT/$src" -o "$out"
  fi
}
preprocess_one() {
  src="$1"; out="$2"
  mkdir -p "$(dirname -- "$out")"
  "$PREPROCESSOR" -I "$ROOT/headers" -I "$ROOT" -I "${CMOTIVE_ROOT:-$ROOT/..}/lib" "$ROOT/$src" -o "$out"
}
rows() {
  sed '/^[[:space:]]*$/d' "$MANIFEST"
}
case "$MODE" in
  help)
    printf '%s\n' 'CMotive language examples shell runner:'
    printf '%s\n' '  manifest | list | compile | objects | preprocess | run | check | debug-symbols | clean'
    ;;
  manifest)
    total=0; missing=0
    mkdir -p "$ROOT/build"
    rows | while IFS= read -r line; do
      file=$(printf '%s\n' "$line" | extract file)
      total=$((total+1))
      if [ ! -f "$ROOT/$file" ]; then echo "missing: $file"; missing=$((missing+1)); fi
      printf '%s %s\n' "$total" "$missing" > "$ROOT/build/.manifest_count"
    done
    set -- $(cat "$ROOT/build/.manifest_count" 2>/dev/null || printf '0 0')
    rm -f "$ROOT/build/.manifest_count"
    if [ "$2" != 0 ]; then exit 1; fi
    echo "manifest ok: $1 examples"
    ;;
  list)
    rows | while IFS= read -r line; do printf '%s\n' "$line" | extract file; done
    ;;
  clean)
    rm -rf "$ROOT/build"
    echo 'cleaned examples build outputs'
    ;;
  compile)
    n=0
    rows | while IFS= read -r line; do
      file=$(printf '%s\n' "$line" | extract file)
      base=$(basename "$file")
      stem=${base%.*}
      n=$((n+1))
      compile_one "$file" "$ROOT/build/bin/${n}_${stem}" no
    done
    echo 'examples compile: PASS'
    ;;
  objects)
    n=0
    rows | while IFS= read -r line; do
      file=$(printf '%s\n' "$line" | extract file)
      base=$(basename "$file")
      stem=${base%.*}
      n=$((n+1))
      compile_one "$file" "$ROOT/build/obj/${n}_${stem}.o" yes
    done
    echo 'examples objects: PASS'
    ;;
  preprocess)
    n=0
    rows | while IFS= read -r line; do
      file=$(printf '%s\n' "$line" | extract file)
      base=$(basename "$file")
      stem=${base%.*}
      n=$((n+1))
      preprocess_one "$file" "$ROOT/build/pp/${n}_${stem}.pp.CMOT"
    done
    echo 'examples preprocess: PASS'
    ;;
  run|check)
    failures="$ROOT/build/example_failures.txt"
    mkdir -p "$ROOT/build/bin"
    : > "$failures"
    n=0
    rows | while IFS= read -r line; do
      file=$(printf '%s\n' "$line" | extract file)
      expected_exit=$(printf '%s\n' "$line" | extract_num expected_exit)
      expected_text=$(printf '%s\n' "$line" | extract expected_stdout_contains)
      base=$(basename "$file")
      stem=${base%.*}
      n=$((n+1))
      exe="$ROOT/build/bin/${n}_${stem}"
      out="$ROOT/build/bin/${n}_${stem}.out"
      err="$ROOT/build/bin/${n}_${stem}.err"
      if ! compile_one "$file" "$exe" no >"$out.compile" 2>"$err.compile"; then
        echo "$file compile failed" >> "$failures"
        continue
      fi
      set +e
      "$exe" >"$out" 2>"$err"
      rc=$?
      set -e
      if [ "$rc" != "${expected_exit:-0}" ]; then echo "$file exit $rc expected ${expected_exit:-0}" >> "$failures"; fi
      if [ -s "$err" ]; then echo "$file stderr not empty" >> "$failures"; fi
      if [ -n "$expected_text" ] && ! grep -q "$expected_text" "$out"; then echo "$file missing expected output" >> "$failures"; fi
    done
    if [ -s "$failures" ]; then
      echo 'examples run: FAILED'
      cat "$failures"
      exit 1
    fi
    echo 'examples run: PASS'
    ;;
  debug-symbols)
    src="examples/150_debug_symbols_options.CMOT"
    out="$ROOT/build/debug-symbols/example_150_debug"
    mkdir -p "$(dirname -- "$out")"
    "$COMPILER" -I "$ROOT/headers" -I "$ROOT" -I "${CMOTIVE_ROOT:-$ROOT/..}/lib" -g3 -O2 "$ROOT/$src" -o "$out"
    test -f "$out"_cmot_debugsymbols.syms
    echo 'examples debug-symbols: PASS'
    ;;
  *)
    echo "unknown mode: $MODE" >&2
    exit 2
    ;;
esac

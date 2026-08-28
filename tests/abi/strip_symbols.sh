#!/usr/bin/env sh
set -eu
strip "$1" || llvm-strip "$1"

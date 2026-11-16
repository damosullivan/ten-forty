#!/bin/sh
# Host-side geometry check. Compiles the same header the watch uses, so the
# scaling and Quick View rules are exercised for every platform without an
# emulator.
set -eu
cd "$(dirname "$0")/.."

CC="${CC:-cc}"
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
OUT="$TMP/test_geometry"

"$CC" -std=c11 -Wall -Wextra -Werror -Isrc/c test/test_geometry.c -o "$OUT"
"$OUT"

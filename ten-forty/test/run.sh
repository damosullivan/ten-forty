#!/bin/sh
# Build and run the word generator test on the host.
set -eu
cd "$(dirname "$0")/.."
out=$(mktemp)
trap 'rm -f "$out"' EXIT
gcc -Wall -Wextra -I src/c -o "$out" src/c/words.c test/words_test.c
"$out"

#!/bin/sh
# Build every face that has a wscript. Run from anywhere.
set -eu
cd "$(dirname "$0")/.."

export PATH="$HOME/.local/share/uv/tools/pebble-tool/bin:$PATH"

for face in ten-forty bauhaus; do
  if [ ! -f "$face/wscript" ]; then
    echo "skip $face: no wscript yet"
    continue
  fi
  echo "== $face"
  (cd "$face" && pebble build)
done

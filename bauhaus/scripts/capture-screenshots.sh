#!/bin/bash
# Capture the store screenshots: two times per platform, exact display size.
#
# The store wants the native resolution, unframed: aplite/basalt/diorite/flint
# 144x168, chalk 180x180, emery 200x228, gabbro 260x260. A 2x capture is
# rejected, so there is no --scale here; images/ holds the 2x files the README
# uses.
#
# The date is pinned to 2026-11-16 so every platform shows the same two-digit
# day. 10:10 is the concept-sheet pose; 04:45 keeps both hands clear of the
# date window.
#
# Emulators keep their flash between sessions. The time is set AFTER the
# install, because installing launches the face with whatever clock the
# emulator had; setting it afterwards pushes a tick and the face redraws.
#
#   scripts/capture-screenshots.sh
set -uo pipefail
cd "$(dirname "$0")/.." || exit 1

export PATH="$HOME/.local/share/uv/tools/pebble-tool/bin:$PATH"

OUT=screenshots
VERSION=$(python3 -c 'import json;print(json.load(open("package.json"))["version"])')
PBW=build/bauhaus.pbw
DATE=2026-11-16

mkdir -p "$OUT"
[ -f "$PBW" ] || { echo "build first: $PBW is missing" >&2; exit 1; }

say() { echo "[$(date +%H:%M:%S)] $*"; }

shot() { # platform time name
  local p=$1 t=$2 name=$3
  local file="$OUT/${p}_${VERSION}_${name}.png"
  local ts
  ts=$(date -u -d "$DATE $t" +%s)

  pebble install --emulator "$p" "$PBW" >/dev/null 2>&1 \
    || { say "install failed: $p"; return 1; }
  sleep 2
  pebble emu-set-time --emulator "$p" "$ts" >/dev/null 2>&1 \
    || { say "set-time failed: $p"; return 1; }
  sleep 2
  pebble screenshot --no-open --emulator "$p" "$file" >/dev/null 2>&1 \
    || { say "screenshot failed: $file"; return 1; }
  say "ok $file"
}

for p in aplite basalt flint chalk diorite emery gabbro; do
  shot "$p" "10:10:00" "1-ten-ten" || continue
  shot "$p" "04:45:00" "2-four-forty-five"
done
say done

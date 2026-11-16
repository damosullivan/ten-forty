#!/bin/bash
# Capture the store screenshots: three per platform, one per theme/state.
#
# RULE: every shot is a three-row time. A two-row time is the face at its
# plainest — "ten forty" is two words and half the layout — and a listing built
# out of those undersells it. The third row is the thing a buyer has not seen,
# so the shots are chosen from the minutes that fill all three rows and the
# choice is checked against src/c/words.c before anything is captured.
#
# The store wants exact display size, so there is no --scale here. A 2x capture
# is rejected with "Image dimensions must be 144x168px (got 288x336px)".
#
# Settings are pushed explicitly each time: the emulator keeps its flash between
# sessions, so a shot taken cold can show the date hidden or the wrong theme.
#
#   scripts/capture-screenshots.sh
set -uo pipefail
cd "$(dirname "$0")/.." || exit 1

OUT=screenshots
VERSION=$(python3 -c 'import json;print(json.load(open("package.json"))["version"])')
KEYS=build/js/message_keys.json
mkdir -p "$OUT"

[ -f "$KEYS" ] || { echo "build first: $KEYS is missing" >&2; exit 1; }

say() { echo "[$(date +%H:%M:%S)] $*"; }

# A time is three-row when the minutes split into a tens word AND a ones word.
# Compile the real word generator rather than restating its rules here.
CHECK=$(mktemp -d)
trap 'rm -rf "$CHECK"' EXIT
cat > "$CHECK/rows.c" <<'C'
#include <stdio.h>
#include <stdlib.h>
#include "words.h"
int main(int argc, char **argv) {
  char hour[24], tens[24], ones[24];
  (void)argc;
  hour_to_12h_word(atoi(argv[1]), hour);
  minute_to_words(atoi(argv[2]), true, tens, ones);
  printf("%s / %s / %s\n", hour, tens, ones);
  return ones[0] ? 0 : 1;
}
C
gcc -Wall -Wextra -I src/c -o "$CHECK/rows" src/c/words.c "$CHECK/rows.c" || exit 1

assert_three_rows() { # HH:MM:SS
  local h rest m
  h=${1%%:*}
  rest=${1#*:}
  m=${rest%%:*}
  if ! "$CHECK/rows" "$h" "$m"; then
    say "REFUSING $1: only two rows. Pick a minute with a ones word."
    return 1
  fi
}

# Setting ids are assigned alphabetically by the SDK, so read them from the
# build instead of hardcoding numbers that move whenever a key is added or
# removed. build/js/message_keys.json is the source of truth.
settings() { # platform theme align
  local p=$1 th=$2 al=$3 ids
  ids=$(python3 - "$th" "$al" "$KEYS" <<'PY'
import json, sys
m = json.load(open(sys.argv[3]))
v = {"Theme": int(sys.argv[1]), "Align": int(sys.argv[2]), "Date": 1,
     "OhWords": 1, "Weight": 0, "Case": 0, "Transition": 2}
print(" ".join("%d=%d" % (m[k], v[k]) for k in v))
PY
)
  [ -n "$ids" ] || { say "could not read $KEYS"; return 1; }
  pebble send-app-message --emulator "$p" --int $ids >/dev/null 2>&1
}

shot() { # platform time theme align name
  local p=$1 t=$2 th=$3 al=$4 name=$5
  local file="$OUT/${p}_${VERSION}_${name}.png"
  assert_three_rows "$t" || return 1
  pebble emu-set-time "$t" --emulator "$p" >/dev/null 2>&1
  pebble install --emulator "$p" >/dev/null 2>&1 || { say "install failed: $p"; return 1; }
  sleep 2
  settings "$p" "$th" "$al"
  sleep 2
  pebble screenshot --no-open --emulator "$p" "$file" >/dev/null 2>&1 \
    || { say "screenshot failed: $file"; return 1; }
  say "ok $file"
}

# Words default to left on the rectangles and centred on the circles, which is
# what the face does out of the box.
declare -A ALIGN=([basalt]=0 [flint]=0 [chalk]=1 [emery]=0 [gabbro]=1)

for p in basalt flint chalk emery gabbro; do
  al=${ALIGN[$p]}
  shot "$p" 11:31:00 0 "$al" 1-eleven-thirty-one  || continue
  shot "$p" 10:42:00 0 "$al" 2-ten-forty-two
  shot "$p" 09:57:00 1 "$al" 3-nine-fifty-seven-light
done
say done

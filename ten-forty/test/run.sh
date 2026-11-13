#!/bin/sh
# Build and run the word generator test on the host. Font sizes and box widths
# come from package.json, so the test follows the project rather than a copy of
# its numbers. Also fails if the hosted settings page has drifted from the
# script that generates it — a stale page is invisible until a user opens it.
set -eu
cd "$(dirname "$0")/.."

hdr=$(mktemp)
out=$(mktemp)
trap 'rm -f "$hdr" "$out"' EXIT

node scripts/config-page.js --check

python3 - "$hdr" <<'PY'
import json, sys

# platform group -> (macro suffix, box width in px)
groups = {
    ("basalt", "flint"): ("BASALT", 132),   # 144 - 6 each side
    ("chalk",):          ("CHALK",  144),   # 180 - 10% each side
    ("emery",):          ("EMERY",  188),   # 200 - 6 each side
    ("gabbro",):         ("GABBRO", 208),   # 260 - 10% each side
}

media = json.load(open("package.json"))["pebble"]["resources"]["media"]
lines = []
for entry in media:
    key = tuple(entry.get("targetPlatforms") or ())
    if key not in groups:
        continue
    suffix = groups[key][0]
    name = entry["name"]
    if name.startswith("FONT_HOUR_"):
        lines.append("#define HOUR_%s %d" % (suffix, entry["pixelHeight"]))
    elif name.startswith("FONT_MIN_LIGHT_"):
        lines.append("#define MIN_%s %d" % (suffix, entry["pixelHeight"]))

for suffix, box in groups.values():
    lines.append("#define BOX_%s %d" % (suffix, box))

open(sys.argv[1], "w").write("\n".join(lines) + "\n")
PY

gcc -Wall -Wextra -I src/c -include "$hdr" \
    -o "$out" src/c/words.c test/words_test.c
"$out"

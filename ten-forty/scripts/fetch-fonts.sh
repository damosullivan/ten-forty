#!/bin/sh
# Fetch the Iosevka weights this watchface uses into resources/fonts/.
#
# The TTFs are not committed: three weights of the full Iosevka build are 24 MB,
# and a clean clone only needs enough to build. The SIL OFL licence text IS
# committed, as resources/fonts/OFL.txt.
set -eu

VERSION=34.9.1
BASE="https://github.com/be5invis/Iosevka/releases/download/v${VERSION}"
ZIP="PkgTTF-Unhinted-Iosevka-${VERSION}.zip"
DEST="$(cd "$(dirname "$0")/.." && pwd)/resources/fonts"

if [ -f "$DEST/font-bold.ttf" ] && [ -f "$DEST/font-light.ttf" ] \
   && [ -f "$DEST/font-regular.ttf" ]; then
  echo "fonts already present in $DEST"
  exit 0
fi

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

echo "downloading Iosevka ${VERSION} (about 120 MB; only three files are kept)"
curl -fL --progress-bar -o "$tmp/iosevka.zip" "${BASE}/${ZIP}"

unzip -o -j -q "$tmp/iosevka.zip" \
  Iosevka-Bold.ttf Iosevka-Light.ttf Iosevka-Regular.ttf -d "$tmp"

mkdir -p "$DEST"
cp "$tmp/Iosevka-Bold.ttf"    "$DEST/font-bold.ttf"
cp "$tmp/Iosevka-Light.ttf"   "$DEST/font-light.ttf"
cp "$tmp/Iosevka-Regular.ttf" "$DEST/font-regular.ttf"

echo "installed:"
ls -l "$DEST"/font-*.ttf

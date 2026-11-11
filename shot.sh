#!/bin/bash
# usage: ./shot.sh HH:MM:SS [outfile]
set -e
export PATH="$HOME/.local/bin:$PATH"
cd /home/damien/src/pebble/ten-forty
t="$1"; out="${2:-/home/damien/src/pebble/shots/latest.png}"
pebble emu-set-time "$t" --emulator basalt >/dev/null
pebble install --emulator basalt >/dev/null 2>&1
sleep 3
pebble screenshot --no-open --emulator basalt "$out" >/dev/null
python3 - "$out" <<'PY'
import sys
from PIL import Image
p=sys.argv[1]
im=Image.open(p).convert('RGB')
im.resize((im.width*3,im.height*3), Image.NEAREST).save(p.replace('.png','-3x.png'))
w,h=im.size; px=im.load(); bg=px[2,2]
bands=[]; cur=None
for y in range(h):
    if any(px[x,y]!=bg for x in range(w)):
        if cur is None: cur=y
    elif cur is not None: bands.append((cur,y-1)); cur=None
if cur is not None: bands.append((cur,h-1))
gaps=[bands[i+1][0]-bands[i][1]-1 for i in range(len(bands)-1)]
print(f"{p}: bands={bands} gaps={gaps} top={bands[0][0]} bottom={h-1-bands[-1][1]}")
PY

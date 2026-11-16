# Bauhaus — agent notes

Context for anyone picking this up cold. Built 2026-11-16 from `SPEC.md`.

## What this is

A vector instrument dial for Pebble. A white dial, a 1px black ring, ten ticks
and a 7x7 triangle at 12, two baton hands, and a black date window at 3 with
the day number reversed out in the system font. No numerals on the dial, no
logo, no second hand, no animation, no settings.

Targets **aplite, basalt, flint, chalk, diorite, emery and gabbro**. The face
is one C file plus a pure-integer geometry header; the only resource is the
25x25 launcher icon. There is no phone-side JS and no settings page.

## Decisions, and why

- **No settings.** The spec says one correct face is the starting point. Black
  on white, date at 3, no config page, no AppMessage keys. This is also why
  there is no `src/pkjs`.
- **Monochrome only.** The concept sheet's black date window is the default.
  The optional red window was not used; colour is not load-bearing.
- **Date at 3, not below the dial.** The spec's default. On the round screens
  this is the widest point of the circle, so the 22x14 window fits with room.
- **Quick View drops the date, then shrinks the dial.** The time comes first.
  The dial is centred in the unobstructed area and reduced only as far as it
  must be. On a Time the dial goes 120 -> 108; on a Time 2, 166 -> 160; on
  Round 2 it goes 216 -> 114 because the notification leaves much less room.
  Chalk reports no obstruction and never changes.
- **Aplite and diorite are included.** Everything is vectors and the system
  font, so the 1-bit screens work. Aplite uses 2.7KB of its 24KB RAM.
- **Antialiasing is off.** The brief treats the 1-bit screen as the medium, and
  AA-off makes every platform render the same crisp black/white dial. It also
  removes the grey fringes that would otherwise soften the batons.

## Geometry

The reference is the concept sheet: a 120px dial on a 144x168 screen. Every
length scales with the dial: `bh_scale(v) = round(v * dial / 120)`. The dial
itself is `round(120 * screen_width / 144)`, forced even so the centre lands on
a pixel.

| Element | 120px reference |
|---|---|
| Dial | 120 diameter, centred, 1px ring |
| Ticks | 1px wide, 4 long, ending 1px inside the ring; 6 and 9 are 6 long |
| Triangle | 7 wide, 7 tall, tip 5 inside the ring, base 12 inside |
| Hour hand | 30 long, 3 thick, round tip |
| Minute hand | 48 long, 2 thick, round tip |
| Centre cap | 4 diameter |
| Date window | 22x14, right edge 45 from the centre (0.75R), vertically centred |
| Date type | system Gothic bold, about 10px; digits' ink ~9px tall |

| Platform | Screen | Dial | r | Date | Hour | Minute | QV dial |
|---|---|---|---|---|---|---|---|
| aplite | 144x168 | 120 | 60 | 22x14 | 30/3 | 48/2 | n/a |
| basalt | 144x168 | 120 | 60 | 22x14 | 30/3 | 48/2 | 108 |
| flint | 144x168 | 120 | 60 | 22x14 | 30/3 | 48/2 | 108 |
| chalk | 180x180 | 150 | 75 | 28x18 | 38/4 | 60/3 | 150 |
| diorite | 144x168 | 120 | 60 | 22x14 | 30/3 | 48/2 | 108 |
| emery | 200x228 | 166 | 83 | 30x19 | 42/4 | 66/3 | 160 |
| gabbro | 260x260 | 216 | 108 | 40x25 | 54/5 | 86/4 | 114 |

`test/test_geometry.c` compiles the same header and asserts these relationships
for every platform and for obstructed heights, so a rounding change cannot
silently put the window through the ring.

## How the drawing works

The whole face is drawn into one full-screen canvas layer, once at load and
once a minute (`MINUTE_UNIT` only — no timer between minutes).

Four rasterisation choices are load-bearing:

- **The ring is 120 short chords, not `graphics_draw_circle`.** With AA off,
  `graphics_draw_circle` leaves gaps at 1-bit. A 3-degree chord is about 3px on
  the reference dial, so the ring stays smooth.
- **The hands are scanline-filled quads, not GPath.** `gpath_draw_filled`
  under-covers a 2px-wide quad, so the minute hand came out thinner than the
  hour hand and its round tip detached. Drawing parallel 1px lines instead
  striped the batons at diagonal angles. The quad is filled row by row with a
  half-open edge rule: counting both boundary rows made a horizontal 2px hand
  three rows thick.
- **The triangle is filled row by row.** GPath's boundary handling turned a
  7x7 triangle into a 5x5 one.
- **The date digits are drawn after the hands.** The black window swallows the
  black hand, but at 3:15 the hand would otherwise cut through the white day
  number. The window goes down before the hands; the digits go on top. At 3:15
  the hand reads as passing behind the aperture and the number stays whole.

The date font is chosen by dial size: Gothic 14 bold at 120, Gothic 18 bold at
150/166, Gothic 24 bold at 216. The text box is centred on the window and then
lifted 3px: the line box is taller than the window and the digits sit low in
it. That lift was measured on all three sizes. If the font or window changes,
re-measure the white ink's top and bottom margins before trusting it.

## Build, run, test

```sh
cd bauhaus
pebble build                         # -> build/bauhaus.pbw, all seven
pebble install --emulator basalt
pebble emu-set-time --emulator basalt 10:10:00
pebble screenshot --no-open --emulator basalt /tmp/out.png
./test/run.sh                        # host geometry check
./scripts/capture-screenshots.sh     # store set, exact display size
../scripts/build-all.sh              # every face in the monorepo
```

No settings means no `config.html`, no `docs/` page and no `message_keys.json`.
The SDK still prints an `enableMultiJS` notice on every build; that is expected
for a face with no JS, not an error.

## Screenshots

`screenshots/` holds two per platform, named `<platform>_<version>_<pose>.png`
and captured at the exact display resolution (no `--scale`; the store rejects
2x). The date is pinned to 2026-11-16 so every platform shows the same two-digit
day. `images/` holds 2x versions of the ten-ten pose for the READMEs.

Round-platform screenshots are RGBA with transparent corners; that is the
emulator's screen mask, not part of the face.

## Gotchas

- **Aplite lacks the unobstructed-area APIs.** `layer_get_unobstructed_bounds`
  and `unobstructed_area_service_subscribe` are absent, so both uses are behind
  `_PBL_API_EXISTS_*` guards. Aplite simply draws the full dial.
- **Chalk reports no obstruction.** Ten Forty found the same; chalk keeps its
  full 150px dial and its date under Quick View. Gabbro does report an
  obstruction: its visible height leaves only a 114px dial, so the date goes.
- **Set the time after installing on the emulator.** Installing launches the
  face with whatever clock the emulator had; setting the time afterwards
  pushes a tick and the face redraws. The capture script does this.
- **`pebble screenshot` does not create directories.** `mkdir -p` first.
- **Emulator hygiene.** A stale emulator shows the launcher, another face, or a
  crash dialog. `pebble kill && pebble wipe`, then reinstall.
- **The UUID is permanent after the first publish.** It is
  `93c8b851-8d18-4fbf-8050-a6efb9c1023f`; do not regenerate it.
- **Name check done.** The store already has a watchface called "Bauhaus" (an
  abstract red-disc/blue-field composition). The name is kept deliberately;
  "Bauhaus Dial" is the checked fallback. See `STORE.md`.

## Publishing state

Version **1.0.0**, not yet submitted. The store copy, screenshot list and
release checks are in `STORE.md`. `watchapp.watchface` and the UUID freeze at
submission.

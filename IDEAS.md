# Ideas / backlog

Deferred by decision, not by accident. Nothing here is started.

## Decided against

- **24-hour mode.** Spelled-out 24h doesn't fit 144px: "seventeen" needs a
  ~30px font and "twenty-three" ~22px, against 37px for normal rows. The only
  options were a jarring size drop or a digit hour. Skipped deliberately.
  Revisit only if the layout changes to allow a 4th row or shorter words.

## Typography

- **Auto-fit per string.** Pick the largest size that fits the row, so "oh" and
  "five" render optically larger than "seventy". Makes the face feel alive at
  no informational cost. Needs a small size ladder (3-4 steps) and one font
  resource per step per weight; watch the RAM budget.
- **Condensed alternative.** Archivo at width 75% fits 53/49 instead of 44/37
  — noticeably denser, least white space. Kept as a fallback if Iosevka's
  narrowness ever feels too technical.
- **Optical margins.** Left margin currently a fixed 6px; large words look
  closer to the edge than small ones. Could bias by glyph sidebearing.
- **Alternate date formats.** Currently "Tue 10". Candidates: "10 Nov",
  "Tuesday", "Tue 10 Nov".

## Motion

- **Per-row slide direction.** Hour slides down as minutes slide up. Nobody in
  the Sliding Text family has done it; roughly 10 lines.
- **Differential sliding.** Only slide the letters that actually changed
  ("seventy" -> "seventy one" moves just the tail). Calmer than re-sliding the
  whole word. This is what Mongo Time / Reel Time get right.
- **Tunable duration / easing.** Current speed is `abs(x)/3 + 1` at 40ms per
  frame, per-row delays 6/3/0.
- **Entrance animation on wake.** Currently only on launch.

## Information

- **Battery dot.** 3px in a corner, only below 20%. The only extra info that
  wouldn't violate the "one or two items, small" rule.
- Deliberately rejected as clutter: steps, weather, heart rate, Bluetooth icon.

## Theming

- **Automatic day/night.** White-on-black after sunset, black-on-white by day.
  Zero clutter, one setting. Needs local sunrise/sunset or a fixed hour.
- **Accent option for the hour row** (one of the 64 Pebble colours).

## Platform

- **Aplite and diorite.** Excluded for now: 1-bit screens plus a 24KB heap make
  three custom fonts impractical. They would need to fall back to the system
  Bitham fonts, which means a second typographic path in `main.c` of ALWAYS
  selecting between custom and system fonts, and skipping `fonts_unload` for
  system fonts.
- **Quick View.** `layer_get_unobstructed_bounds()` is wired up but never
  tested against an actual notification.
- **Config page hosting.** Currently a `data:` URI, which iOS blocks. Needs
  `config.html` hosted and `CONFIG_URL` in `src/pkjs/index.js` pointed at it.
- **Publishing.** Needs a permanent UUID, 80x80 and 144x144 icons, per-platform
  screenshots and GIFs, then `pebble login` + `pebble publish`.
- **i18n.** `words.c` is English-only; the original face was translated many
  times over (see Sliding Text International and friends).

## Known upstream quirk

The original `coredevices/pebble-apps/sliding-text` calls `make_animation()` on
every minute tick, creating a new `Animation` and overwriting the pointer
without destroying the old one. One leaked allocation per minute. Worth filing
upstream; our version uses a self-cancelling `AppTimer` instead.

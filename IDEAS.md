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
- **Alternate date formats.** Done: 0 "Tue 10", 1 "10 Nov", 2 "Tue 10 Nov",
  3 "Tuesday". 2 and 3 are rectangular-only — a round display leaves the date an
  8-character chord at the top, so they clamp to 0 there and the config page
  hides them. Measured, not guessed.

## Motion

- **Per-row slide direction.** Hour slides down as minutes slide up. Nobody in
  no face in this family has done it; roughly 10 lines.
- **Differential sliding.** Done, in the form the data allows: a row travels
  only as far as its own text, so "four" -> "five" is 76px each way instead of
  clearing the screen twice, and a blank row enters from just past where it
  lands. Letter-level pinning of the shared prefix was the original idea and was
  dropped: `words.c` puts the tens and the ones on separate rows, so "seventy" ->
  "seventy one" already leaves the tens row untouched, and across the whole
  English set only about six of the 132 hourly transitions share a leading
  character at all. The measured effect was nothing; the travel is what was felt.
- **Tunable duration / easing.** Current speed is `abs(x)/3 + 1` at 40ms per
  frame. Rows move together; the old 6/3/0 stagger was removed in favour of
  the hour-only slide.
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
- **Quick View.** Done: the words re-fit into the space above a notification and
  the date is skipped while it is up.
- **Config page hosting.** Currently a `data:` URI, which iOS blocks. Needs
  `config.html` hosted and `CONFIG_URL` in `src/pkjs/index.js` pointed at it.
- **Publishing.** Needs a permanent UUID, 80x80 and 144x144 icons, per-platform
  screenshots and GIFs, then `pebble login` + `pebble publish`.
- **i18n.** `words.c` is English-only; the original face was translated many
  times over by other faces in this genre.

## Known upstream quirk

The original `coredevices/pebble-apps/sliding-text` calls `make_animation()` on
every minute tick, creating a new `Animation` and overwriting the pointer
without destroying the old one. One leaked allocation per minute. Filed as
coredevices/pebble-apps#8 (2026-11-11) — the standalone `coredevices/sliding-text`
repo has issues disabled and redirects there. Our version uses a self-cancelling
`AppTimer` instead, so it never allocates per tick.

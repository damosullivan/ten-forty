# Ideas / backlog

Deferred by decision, not by accident. Nothing here is started.

## Decided against

- **24-hour mode.** Spelled-out 24h doesn't fit 144px: "seventeen" needs a
  ~30px font and "twenty-three" ~22px, against 37px for normal rows. The only
  options were a jarring size drop or a digit hour. Skipped deliberately.
  Revisit only if the layout changes to allow a 4th row or shorter words.
- **Multiple date formats.** Removed in 1.1.0. One format, `Tue 10`, at the
  bottom. The alternatives either lost the weekday, lost the day number, or did
  not fit the chord a round display leaves at the bottom.
- **Top date.** Removed in 1.1.0. Reading order puts the date after the time,
  and the bottom is the wider chord on a circle. The top is also where the
  hour's ascenders already fight the mask.
- **Capital letter case.** Removed in 1.1.0. It capitalised the first letter of
  each row, turning one sentence into three.
- **Slide every row.** Removed in 1.1.1. Giving the minutes the same motion as
  the hour flattened that hierarchy and made the face kinetic every minute.
- **Per-row slide direction.** Moot once only the hour slides.
- **Auto day/night.** White-on-black after sunset, black-on-white by day. The
  idea is clean, but it adds a rule and a failure mode to a face whose theme is
  already one deliberate choice.
- **Accent colour on the hour row.** The two-tone face is the reason it reads as
  Bauhaus; a colour accent would be the first ornament.
- **Battery dot.** A second information channel, even at 3px below 20%. Steps,
  weather, heart rate and a Bluetooth icon stay rejected as clutter too.

## Status line / plugins

Not started. The bottom line is designed as a register, not a canvas: one line,
text only, about nine characters to clear the chord a round display leaves at
the bottom, fixed size/weight/colour, alignment inherited from the words,
chord-clamped on round, and the first thing to yield under Quick View. A plugin
supplies a string, not a layout. When it lands, the `Date` setting becomes
`Status` / `Bottom line` and the AppMessage keys renumber.

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

## Motion

- **Differential sliding.** Done, in the form the data allows: the hour travels
  only as far as its own text, so "four" -> "five" is 76px rather than clearing
  the screen. Letter-level pinning of the shared prefix was the original idea and
  was dropped: `words.c` puts the tens and the ones on separate rows, so
  "seventy" -> "seventy one" already leaves the tens row untouched, and across
  the whole English set only about six of the 132 hourly transitions share a
  leading character at all. The measured effect was nothing; the travel is what
  was felt.
- **Tunable duration / easing.** Current speed is `abs(x)/3 + 1` at 40ms per
  frame. The hour moves on its own; the old 6/3/0 stagger was removed in favour
  of the hour-only slide.
- **Entrance animation on wake.** Currently only on launch.

## Platform

- **Aplite and diorite.** Excluded for now: 1-bit screens plus a 24KB heap make
  three custom fonts impractical. They would need to fall back to the system
  Bitham fonts, which means a second typographic path in `main.c` of ALWAYS
  selecting between custom and system fonts, and skipping `fonts_unload` for
  system fonts.
- **Quick View.** Done: the words re-fit into the space above a notification and
  the date is skipped while it is up.
- **Config page hosting.** Done: GitHub Pages serves `docs/config.html`, and
  `CONFIG_URL` in `src/pkjs/index.js` points at it. iOS blocks `data:` URIs,
  which is what made this necessary rather than tidy.
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

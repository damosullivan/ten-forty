# AGENTS.md

Context for anyone picking this up cold — human or agent. Written 2026-11-11,
at the end of the session that built most of it.

## What this is

**Ten Forty**, a text-clock watchface for Pebble. The time is spelled in three
stacked words — hour, minute-tens, minute-ones — set in Iosevka, hour bold,
minutes light. Optional date at the bottom. Seven settings, all presentational.

Targets basalt, flint, chalk, emery, gabbro. Developed and tested on a Pebble
Time Steel (basalt) using the real hardware via CloudPebble.

## How we got here

Worth reading before changing anything, because several obvious-looking
improvements were considered and deliberately rejected.

- **Started from Sliding Text.** The user asked for "Sliding Time", which turned
  out to be Pebble's MIT-licensed `sliding-text` example. The project is a
  rewrite of it, not a fork with tweaks. MIT attribution lives in `LICENSE.md`
  and is a legal requirement — do not remove it.
- **Renamed to Ten Forty.** The word "Sliding" described the genre it was forked
  from, not what it is. The face is a text clock.
- **Sliding is now a transition, not the identity.** It slides the *hour row
  only*, because that is the one change anyone notices; options are
  none / hour only. The hour travels only as far as its own text
  (`w + SLIDE_GAP`), not the screen width, so it moves by the width of a word.
  Letter-level differential was measured and dropped as dead code — minute rows
  in English share no common prefix, and only about six of the 132 hourly
  transitions do, pinning one or two letters. "Slide every row" was removed in
  1.1.1: giving the minutes the same motion flattened the hour's hierarchy and
  made the face kinetic every minute.
- **Iosevka, not Polestar.** The user wanted Polestar's typeface, which is
  Unica77 — a bespoke Lineto commission, not licensable. Iosevka was chosen
  over Inter, Archivo and Hanken Grotesk. Iosevka **Charon** was tried first and
  rejected as too rounded; the plain monospace Iosevka is correct.
- **Iosevka being monospace is load-bearing.** 0.5em advance makes row widths
  predictable, and means changing letter case never changes layout.
- **Teens are split on the morpheme, and only one needs a hyphen.** A row holds
  seven characters at the minute size, so the 8-letter teens are cut into head +
  "teen" across the two minute rows. "four", "seven", "eight" and "nine" are
  words, so those read as two fragments the eye joins. "thir" is not a word, so
  thirteen alone is `thir-`, the mark for a word broken across a line.
- **Layout derives from live font metrics**, not a fixed pixel grid, so it
  scales across displays. The date is drawn in the bottom margin rather than
  reserving a strip, and the words are centred in whatever space the date
  leaves: with no date they centre in the full screen, with the date they sit
  higher by half its strip. Turning the date off therefore recentres them once,
  by design.
- **24-hour mode was removed.** Spelled-out 24h does not fit: "seventeen" needs
  a ~30px font and "twenty-three" ~22px against 37px normally. Do not
  reintroduce it without solving that first.
- **Tap-to-toggle-theme was removed.** It was firing on accidental knocks.
- **The date fade / tap-reveal / backlight reveal was added, then removed.** It
  worked (verified on flint), but it solved a problem that did not survive a
  second look, and it added two settings to a face whose whole point is
  restraint. Recoverable from `fc049a6` and `efda0d2` if a Time 2 ever makes it
  worth having.

Future work is in `IDEAS.md`. Publishing state is in `TODO.md`. Store copy is
in `ten-forty/STORE.md`.

## Repo layout

```
.
├── AGENTS.md            this file
├── IDEAS.md             backlog / considered-and-deferred
├── TODO.md              publishing checklist
├── LICENSE.md           upstream MIT notice + font pointer
├── shot.sh              emulator screenshot + ink-band measurement helper
├── images/              screenshots for the README (2x, clean names)
├── README.md            the front page
├── baseline/            upstream clone (gitignored, has its own .git)
├── docs/                the settings page GitHub Pages serves; generated
│   └── config.html      do not hand-edit — see Gotchas
└── ten-forty/           the watchface
    ├── src/c/main.c     layout, drawing, animation, settings
    ├── src/c/words.c    number -> words
    ├── src/pkjs/index.js  phone side; opens the hosted settings page
    ├── test/            host-side word tests
    ├── scripts/fetch-fonts.sh
    ├── scripts/config-page.js  generates docs/config.html
    ├── scripts/capture-screenshots.sh  the store screenshots
    ├── scripts/make-menu-icon.py  regenerates the launcher icon
    ├── resources/fonts/ TTFs NOT committed — see Gotchas
    ├── resources/images/menu-icon.png  25x25 launcher icon, committed
    ├── screenshots/     per-platform captures
    ├── STORE.md
    └── README.md
```

## Build, run, test

```sh
cd ten-forty

./scripts/fetch-fonts.sh      # once per clone; downloads ~120MB, keeps 3 files
pebble build                  # -> build/ten-forty.pbw
pebble install --emulator basalt
pebble screenshot --no-open --emulator basalt /tmp/out.png
pebble logs --emulator basalt

./test/run.sh                 # host-side word test + config page drift check
```

On the real watch: enable **Dev Connection** in the Pebble app (Devices →
three dots → Dev Connection), then

```sh
pebble login                  # once
pebble install --cloudpebble
pebble screenshot --cloudpebble /tmp/watch.png
pebble send-app-message --cloudpebble --int <id>=<value>
```

`pebble login` needs a browser and binds `localhost:60000`. If a previous
attempt was interrupted it leaves the port held and every retry fails with
`Address already in use` — find the pid with `ss -ltnp | grep 60000` and kill it.
If the browser is on another machine, run the auth URL there, then
`curl "http://127.0.0.1:60000/?broker_status=ok&flow_id=<id>"` locally.

## Settings and ids

| Setting | Id | Values | Default |
|---|---|---|---|
| Theme | 10004 | 0 dark, 1 light | 0 |
| Alignment | 10000 | 0 left, 1 centre, 2 right | 0, centre on round |
| Date | 10002 | 0 hidden, 1 bottom | 1 |
| `oh` minutes | 10003 | 1 "seven oh five", 0 "seven five" | 1 |
| Font weight | 10006 | 0 Light, 1 Regular | 0 |
| Letter case | 10001 | 0 lowercase, 1 UPPERCASE | 0 |
| Transition | 10005 | 0 none, 1 hour only | 1 |

There is one date format, `Tue 10`, and one position, the bottom. It is the
widest form that fits the chord a circle leaves at the bottom (9 characters),
and it carries both the weekday and the day number. **Alignment is one axis for
the whole face**; the date has no alignment of its own.

Persist keys in `main.c` are hand-allocated and do not match the ids above
(1 theme, 2 align, 4 date, 5 oh, 7 regular, 8 caps, 9 trans). Key 4 stores
0/1 now. Key 9 stores 0/1 too; an old every-row or hour-only value both become
hour-only. Key 3 is a retired TimeFormat, 6 and 12 are retired DateAlign and
DateFormat, and 10/11 are retired DateHold/TapDate — do not reuse them.

## Gotchas

**Row overflow is silent.** The text layer truncates with an ellipsis rather
than failing. Iosevka is monospace but the rasteriser rounds each glyph advance
**up** to a whole pixel, so at N px the advance is `ceil(N/2)` and a row holds
`box_width / ceil(N/2)` characters. Getting this wrong shipped a build where
"fifteen", "o'clock" and "seventy" all rendered as "fifte..." — and it was
invisible because every test time happened to use short words. `test/run.sh`
now derives sizes from `package.json` and checks every platform. If you change
a font size, run it.

**Box widths differ per platform.** Rect screens inset 6px each side; round
screens inset 10%, so a round display gives up proportionally more width:
gabbro's box is 208 of 260, emery's is 188 of 200. A bigger screen does not
automatically mean more room per character — check the table in `test/run.sh`.

**On a circle, the mask eats the top row's ascenders, not the box's corners.**
The visible area is a radius-`w/2` circle, so at the height of the hour row's
ascenders only the middle of the screen exists: on chalk at 48px the ink starts
at x25 while the circle does not reach x47 until 26px lower. Anything tall at
the left edge of the top row — `f`, `l`, `t`, `h` — loses its tip when the words
are left- or right-aligned. Two things answer it: chalk runs 40/34 rather than
48/40, and round platforms default to centred words. Gabbro is clipped the same
way in proportion, and gets by on the centred default alone. Emery is a
rectangle and has none of this.

**Adding a setting renumbers every AppMessage id.** They are assigned
alphabetically, so inserting one shifts everything after it and any saved
`--int` command will silently hit the wrong field. The config page is immune —
it addresses settings by name. Always read `build/js/message_keys.json` after
adding one.

**Fonts are not committed.** Three Iosevka weights are 24MB. `.gitignore` skips
them and `scripts/fetch-fonts.sh` fetches v34.9.1. `OFL.txt` is committed.

**The launcher icon is 25x25, and its digits sit on an opaque tile.** A bitmap
resource with `menuIcon: true` becomes the icon the watch shows beside the name
in the watchfaces list — and above it on chalk and gabbro, where the launcher
lays that list out differently. The SDK caps the file at 25x25. The row being
highlighted is white on basalt and emery, a dark bar on chalk and gabbro, and
black on flint, and the launcher does not invert the icon consistently: ink on a
transparent background simply disappeared against at least one of them, and
`memoryFormat: "1Bit"` on an RGBA file rendered a plain white box. White behind
the digits reads on all five and is how the stock icons are drawn. Regenerate
with `python3 scripts/make-menu-icon.py`.

**Changing a resource's config does not always rebuild its pbpack.** Editing
`memoryFormat` or `targetPlatforms` left the old `.pbpack` in `build/`; touching
the PNG did not shake it loose, because staleness is keyed to the file, not to
the entry. If the icon looks wrong after a config change, `pebble clean` first.

**The config page is hosted, and it is platform-agnostic.**
iOS refuses to load a `data:` URI in the config webview, so the page is a real
file served by GitHub Pages: `https://damosullivan.github.io/ten-forty/config.html`,
which is `CONFIG_URL` in `src/pkjs/index.js`. The phone only opens that URL and
appends the current settings as a fragment.

There is one date format and it fits every display, so the page no longer needs
to know which watch it is talking to and the watch no longer sends its platform.
If a future setting does need per-platform options, the choice has to travel in
the fragment again.

`docs/config.html` is generated by `scripts/config-page.js` and committed, since
Pages serves plain files. **Do not hand-edit it** — `test/run.sh` fails if it
differs from what the generator produces. After changing any field, label,
option or the page markup: `node scripts/config-page.js --write`.

Pages caches HTML for about ten minutes, so a change to the page can lag the
commit that made it. `CONFIG_URL` therefore carries a `?v=` query, bumped
whenever the page changes: a cached old page must not be able to send the
previous AppMessage ids after a release renumbers them. The page reads only the
fragment, so the query is free to be a version string.

**Watchfaces cannot see buttons.** Up/Down belong to the timeline, Select to the
launcher, Back to the system. The only gesture a face receives is an
accelerometer tap. `backlight_service_subscribe` exists but is a compiled-out
stub on aplite, basalt, chalk and diorite — only emery, flint and gabbro can
observe the backlight (`#ifdef _PBL_API_EXISTS_backlight_service_subscribe`).

**The watch owns the settings.** It persists them and pushes them to the phone
on launch; the phone mirrors them and pushes only when the user saves. Do not
push from the JS `ready` handler — that used to silently overwrite the watch.

**Vertical centring is calibrated, not derived.** The anchor is
`screen_height * 94 / 168` and the ink centre lands about 10px below it. That
94 was measured, not calculated — if you change the hour font size or the
pitch, re-measure the top and bottom margins with `shot.sh` or the ink-band
snippet rather than trusting the number.

Re-measured on basalt for 1.1.x, 11:31: with the date off, top 22px and bottom
21px; with the date on, top 12px and the date's ink 5px from the bottom, with a
13px gap between the ones row and the date. The 5px is the caption margin and
is deliberate: growing it crowds the date against the words.

On round displays the gap is 8px (`DATE_EDGE_BOTTOM` is
`PBL_IF_ROUND_ELSE(8, 5)`), because the chord narrows fastest at the bottom. At
5px, a 26px date on gabbro was exactly as wide as the chord and the text layer
silently ellipsised it to "Mo…"; the chord is also measured at the bottom of the
date's ink rather than the middle of its box, so a left-aligned date starts
inside the circle. Both were found by testing left alignment on chalk and
gabbro, not by reading the code.

**Emulator hygiene.** A stale emulator shows another app, a crash dialog, or
your app highlighted but not launched. `pebble kill && pebble wipe`, then
reinstall. Screenshots taken immediately after `pebble install` can catch the
launcher instead of the face.

**`pebble screenshot` does not create directories.**

## What is missing

- **Config hosting for iOS** — done, see gotchas. GitHub Pages serves
  `docs/config.html`; the iPhone path is assumed to work but has never been
  confirmed on an actual iPhone.
- **Version is `1.1.1`** — first store release, then a screenshot rebuild, then
  the date/letter-case simplification, then the every-row removal. Numbers and
  dots only, and no version may be reused: every upload needs a fresh one, even
  when only the screenshots changed.
- **Screenshots are current for 1.1.1** — three per platform in
  `ten-forty/screenshots/`, captured from the 1.1.1 build by
  `scripts/capture-screenshots.sh`. The store takes exact display size, so there
  is no `--scale`: a 2x file is rejected. Settings are pushed explicitly before
  each shot, because the emulator keeps its flash between sessions and a cold
  shot can come out date-hidden or in the wrong theme.
- **Every store screenshot is a three-row time.** Two rows — "ten forty" — is
  the face at its plainest and a listing built from those hides the layout that
  makes it worth installing. `scripts/capture-screenshots.sh` refuses a time
  whose minutes have no ones word, checking it by compiling `src/c/words.c`
  rather than restating the rule.
- **Quick View reflows, and drops the date** — `layer_get_unobstructed_bounds()`
  shrinks to the area above a notification (117 of 168px on a Time, 169 of 228
  on a Time 2), which three rows at their usual leading do not fit into. The
  rows close up and the date is skipped. Chalk reports no obstruction, so it
  keeps its normal layout. The ink ratios in `ink_above()`/`ink_below()` are
  measured, not derived.
- **No localisation** — the content is English words, so non-English users
  cannot use the face at all. See `IDEAS.md`; this is the biggest unlock.
- **No 24-hour, no aplite/diorite** (1-bit screens, 24KB heap, would need the
  system fonts).

## Release steps

1. Settings page is hosted — check it still loads, and that `CONFIG_URL` points
   at the live URL.
2. Bump `version` to the next fresh number; rebuild.
3. Recapture screenshots (exact display size, unframed).
4. `pebble publish`, or the Developer Dashboard at
   developer.repebble.com/dashboard.
5. Publish **Unlisted**, install from the store on the watch, verify.
6. Flip to **Listed**.

There is **no review queue** — apps go live on submit. The UUID
(`6f1c2b40-8a3d-4e57-9c21-7b5e0d4a9f13`) becomes permanent at that point, and
`watchapp.watchface` cannot be changed after submission.

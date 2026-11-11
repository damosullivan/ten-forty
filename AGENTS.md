# AGENTS.md

Context for anyone picking this up cold — human or agent. Written 2026-11-11,
at the end of the session that built most of it.

## What this is

**Ten Forty**, a text-clock watchface for Pebble. The time is spelled in three
stacked words — hour, minute-tens, minute-ones — set in Iosevka, hour bold,
minutes light. Optional date. Eight settings, all of them presentational.

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
- **Sliding is now a transition, not the identity.** It defaults to sliding the
  *hour row only*, because that is the one change anyone notices. Options are
  none / every row / hour only.
- **Iosevka, not Polestar.** The user wanted Polestar's typeface, which is
  Unica77 — a bespoke Lineto commission, not licensable. Iosevka was chosen
  over Inter, Archivo and Hanken Grotesk. Iosevka **Charon** was tried first and
  rejected as too rounded; the plain monospace Iosevka is correct.
- **Iosevka being monospace is load-bearing.** 0.5em advance makes row widths
  predictable, and means changing letter case never changes layout.
- **Layout derives from live font metrics**, not a fixed pixel grid, so it
  scales across displays. The date is drawn in the margin rather than reserving
  a strip, and the words are centred in whatever space the date leaves: with no
  date they centre in the full screen, with a date at the bottom they sit
  higher, with it at the top they sit lower. Changing the date setting
  therefore shifts the words once, by design.
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
├── baseline/            upstream clone (gitignored, has its own .git)
└── ten-forty/           the watchface
    ├── src/c/main.c     layout, drawing, animation, settings
    ├── src/c/words.c    number -> words
    ├── src/pkjs/index.js  settings page (embedded) + phone side
    ├── test/            host-side word tests
    ├── scripts/fetch-fonts.sh
    ├── resources/fonts/ TTFs NOT committed — see Gotchas
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

./test/run.sh                 # host-side word test; no SDK needed
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
| Theme | 10005 | 0 dark, 1 light | 0 |
| Time alignment | 10000 | 0 left, 1 centre, 2 right | 0 |
| Date alignment | 10002 | 0 left, 1 centre, 2 right | 1 |
| Date position | 10003 | 0 hidden, 1 top, 2 bottom | 2 |
| `oh` minutes | 10004 | 1 "seven oh five", 0 "seven five" | 1 |
| Font weight | 10007 | 0 Light, 1 Regular | 0 |
| Letter case | 10001 | 0 lowercase, 1 UPPERCASE, 2 Capital | 0 |
| Transition | 10006 | 0 none, 1 every row, 2 hour only | 2 |

Persist keys in `main.c` are hand-allocated and do not match the ids above
(1 theme, 2 align, 4 datepos, 5 oh, 6 datealign, 7 regular, 8 caps, 9 trans).
Key 3 is a retired TimeFormat and 10/11 are retired DateHold/TapDate — do not
reuse them.

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

**Adding a setting renumbers every AppMessage id.** They are assigned
alphabetically, so inserting one shifts everything after it and any saved
`--int` command will silently hit the wrong field. The config page is immune —
it addresses settings by name. Always read `build/js/message_keys.json` after
adding one.

**Fonts are not committed.** Three Iosevka weights are 24MB. `.gitignore` skips
them and `scripts/fetch-fonts.sh` fetches v34.9.1. `OFL.txt` is committed.

**The config page is a `data:` URI, which iOS refuses to load.** Android and
the emulator are fine. To support iPhone users, host `config.html` and set
`CONFIG_URL` in `src/pkjs/index.js`. This is the one real blocker to publishing.

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

**Emulator hygiene.** A stale emulator shows another app, a crash dialog, or
your app highlighted but not launched. `pebble kill && pebble wipe`, then
reinstall. Screenshots taken immediately after `pebble install` can catch the
launcher instead of the face.

**`pebble screenshot` does not create directories.**

## What is missing

- **Config hosting for iOS** — see gotchas. Only genuine publishing blocker.
- **Version is `0.1.0`** — bump to `1.0.0` for the first store release.
- **Screenshots are stale** — the five in `ten-forty/screenshots/` predate the
  clipping fixes. Recapture, ideally 3 per platform (dark, light, another time).
- **Quick View is untested** — `layer_get_unobstructed_bounds()` is wired up but
  has never been checked against a real notification.
- **No localisation** — the content is English words, so non-English users
  cannot use the face at all. See `IDEAS.md`; this is the biggest unlock.
- **No 24-hour, no aplite/diorite** (1-bit screens, 24KB heap, would need the
  system fonts).

## Release steps

1. Host the config page; set `CONFIG_URL`.
2. Bump `version` to `1.0.0`; rebuild.
3. Recapture screenshots (exact display size, unframed).
4. `pebble publish`, or the Developer Dashboard at
   developer.repebble.com/dashboard.
5. Publish **Unlisted**, install from the store on the watch, verify.
6. Flip to **Listed**.

There is **no review queue** — apps go live on submit. The UUID
(`6f1c2b40-8a3d-4e57-9c21-7b5e0d4a9f13`) becomes permanent at that point, and
`watchapp.watchface` cannot be changed after submission.

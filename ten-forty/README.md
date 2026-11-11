# Ten Forty

A text clock for Pebble: three stacked words spell the time, set in Iosevka
with a baseline grid that fills the screen.

## Build

```sh
pebble build
pebble install --emulator basalt
pebble screenshot --no-open --emulator basalt shot.png
```

Output: `build/v2.pbw`

On a real watch: enable *Dev Connection* in the Pebble app (Devices -> your
watch -> three dots), then `pebble login` and `pebble install --cloudpebble`.

## Settings

Six options, persisted on the watch. Configurable from the phone config page,
or by `pebble send-app-message --int <key>=<value>`.

| Setting        | Id    | Values                                  | Default |
|----------------|-------|-----------------------------------------|---------|
| Theme          | 10005 | 0 white-on-black, 1 black-on-white      | 0       |
| Time alignment | 10000 | 0 left, 1 centre, 2 right               | 0       |
| Date alignment | 10002 | 0 left, 1 centre, 2 right               | 1       |
| Date position  | 10003 | 0 hidden, 1 top, 2 bottom               | 2       |
| `oh` minutes   | 10004 | 1 "seven oh five", 0 "seven five"       | 1       |
| Font weight    | 10007 | 0 Light, 1 Regular                      | 0       |
| Letter case    | 10001 | 0 lowercase, 1 UPPERCASE, 2 Capital     | 0       |
| Transition     | 10006 | 0 none, 1 slide every row, 2 hour only  | 2       |

Ids are assigned alphabetically by the SDK, so adding a setting renumbers
them. `build/js/message_keys.json` is the source of truth. For example:

```sh
pebble send-app-message --emulator basalt --int 10007=1   # Regular weight
```

Transition defaults to **slide the hour only**: the minute rows cut instantly,
so the face is still almost all of the time, and the one change you would
notice -- the hour turning over -- gets the animation.

Case is applied to all three rows together, and to the date. It is one
setting by design: the rows are a single sentence split across lines, and the
hour is already distinguished by weight and size.

The watch owns the settings. It persists them and pushes them to the phone on
launch, so the config page opens showing the real values. The phone never
pushes on startup -- only when you press Save.

## Tests

The word generator has a host-side test, no watch or SDK needed:

```sh
ten-forty/test/run.sh
```

It checks that no row can exceed seven characters (the width the minute font
allows) across all 60 minutes and all 12 hours. Eight characters is silently
clipped by the text layer, which is how "thirteen" once rendered as "thirt...".

## Platforms

Targets basalt, flint, chalk, emery and gabbro. Aplite and diorite are excluded:
1-bit screens plus a 24KB heap make three custom fonts impractical there — they
would need the system fonts instead.

Font sizes are picked per display so every platform fills its width the same
way:

| Platform        | Screen   | Hour | Minutes | Date |
|-----------------|----------|------|---------|------|
| basalt, flint   | 144x168  | 44   | 36      | 16   |
| chalk           | 180x180  | 48   | 40      | 17   |
| emery           | 200x228  | 61   | 51      | 22   |
| gabbro          | 260x260  | 70   | 59      | 26   |

## Layout

Rows are placed on a baseline grid: `anchor = 85 * screen_height / 168`, pitch
`= 1.07 * hour_size`, so the stack scales with the display. The date is drawn in
the margin rather than reserving a strip, which is how the upstream face wasted
~38px at the bottom. The words centre in whatever space the date leaves, so
turning the date off recentres them -- a one-off shift when the setting
changes, not something that happens while you are looking at it.

Animation is a self-cancelling 40ms `AppTimer` that only runs while rows are
moving, then stops. Nothing ticks between minutes.

## Fonts

Iosevka (Bold for the hour, Light for the minutes and date; Regular optional).
SIL Open Font License 1.1 — see `resources/fonts/OFL.txt`.
Upstream Iosevka: https://github.com/be5invis/Iosevka

The TTFs are **not committed** (three weights of the full build are 24 MB).
After a fresh clone, fetch them before building:

```sh
ten-forty/scripts/fetch-fonts.sh
```

## Caveats

- The config page is passed to the phone as a `data:` URI. Android and the
  emulator handle it; iOS blocks `data:` URIs. For iOS, host `config.html` and
  set `CONFIG_URL` in `src/pkjs/index.js`.
- Quick View (`layer_get_unobstructed_bounds`) is wired up but only lightly
  tested.
- UUID is a placeholder. Change it before publishing.

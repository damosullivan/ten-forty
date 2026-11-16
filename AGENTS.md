# AGENTS.md

Pebble-wide context for this repo. The faces have their own notes:
`ten-forty/AGENTS.md` for Ten Forty, `bauhaus/SPEC.md` for the Bauhaus concept.

## What this is

A small monorepo of Pebble watch faces. Each face is a self-contained Pebble
project, built and published from its own directory:

- `ten-forty/` — the text clock. Custom Iosevka fonts, seven settings, a hosted
  settings page and a phone-side bundle.
- `bauhaus/` — the instrument dial. Pure vectors, the system font, no settings,
  seven platforms including the 1-bit screens.

The repo does not give Pebble a workspace: `pebble build` still runs inside one
project directory. The repo shares the context, not the build system.

## Layout

```
.
├── AGENTS.md              this file
├── README.md              the faces index
├── LICENSE.md             repo licence; Ten Forty has its own upstream notice
├── .gitignore
├── scripts/build-all.sh
├── docs/                  GitHub Pages root
│   ├── .nojekyll
│   └── ten-forty/config.html
├── ten-forty/             the text clock (full Pebble project)
├── bauhaus/               the instrument dial (full Pebble project)
├── baseline/              upstream clone (gitignored)
├── fonts-src/             candidate fonts (gitignored)
└── shots/                 design screenshots (gitignored)
```

Each face's `build/` is gitignored. The fetched Iosevka TTFs under
`ten-forty/resources/fonts/` are gitignored too.

## Toolchain

```sh
export PATH="$HOME/.local/share/uv/tools/pebble-tool/bin:$PATH"
# pebble 5.1.2, SDK 4.44
```

Build one face:

```sh
cd ten-forty
./scripts/fetch-fonts.sh      # once per clone
pebble build                  # -> build/ten-forty.pbw
pebble install --emulator basalt
pebble screenshot --no-open --emulator basalt /tmp/out.png
./test/run.sh
```

Build every face that has a `wscript`:

```sh
./scripts/build-all.sh
```

## Publishing

Publishing is per face and per directory. The store sees separate apps, each
with its own UUID, version and listing.

First release of a new face:

```sh
cd <face>
pebble publish \
  --pbw build/<face>.pbw \
  --release-notes "First release." \
  --no-gif-all-platforms \
  --non-interactive \
  --name "<Name>" \
  --version 1.0.0 \
  --description "<one paragraph>" \
  --source "https://github.com/damosullivan/<repo>" \
  --screenshots screenshots/*.png
```

Later releases:

```sh
pebble publish \
  --pbw build/<face>.pbw \
  --release-notes "..." \
  --replace-screenshots \
  --no-gif-all-platforms \
  --is-published \
  --non-interactive \
  --screenshots screenshots/*_<version>_*.png
```

Facts that do not change:

- **No review queue.** A submitted app goes live immediately.
- **UUID is permanent.** It matches future releases to the listing. Freeze it
  before the first submission.
- **`watchapp.watchface` is permanent.** A watchface cannot become an app.
- **Versions must be fresh.** Numbers and dots only, no reuse, even for a
  screenshot-only upload.
- **`pebble publish` does not edit an existing app's description.** Use the
  dashboard for listing copy.
- Dashboard: https://developer.repebble.com/dashboard

## Store screenshots

- Exact display size, unframed, no `--scale`:
  basalt/flint 144x168, chalk 180x180, emery 200x228, gabbro 260x260.
- Filenames must start with the platform name for `--non-interactive`.
- `pebble screenshot` does not create directories.
- The emulator keeps its flash between sessions; push settings explicitly before
  each shot. Ten Forty's `scripts/capture-screenshots.sh` is the worked example.

## Shared Pebble gotchas

**AppMessage ids are assigned alphabetically.** Adding or removing a setting
renumbers everything after it. Any saved `--int` command silently hits the wrong
field. Read the built `build/js/message_keys.json` after changing the list.

**On a circle, the mask eats the top row's ascenders, not the box's corners.**
At the height of tall glyphs only the middle of the screen exists. Round
platforms need smaller type and, usually, centred words. Ten Forty's
`AGENTS.md` has the measured chalk/gabbro numbers.

**Quick View shrinks the unobstructed area.** `layer_get_unobstructed_bounds()`
can be much shorter than the screen; a face needs to reflow or drop its least
important element. Chalk reports no obstruction.

**Watchfaces cannot see buttons.** Up/Down belong to the timeline, Select to the
launcher, Back to the system. The only gesture a face receives is an
accelerometer tap.

**Emulator hygiene.** A stale emulator shows another app, a crash dialog, or the
launcher. `pebble kill && pebble wipe`, then reinstall. A screenshot taken
immediately after `pebble install` can catch the launcher.

## GitHub Pages

Pages serves `/docs`, so the Ten Forty settings page is at
`docs/ten-forty/config.html`. The repo was renamed from `ten-forty` to
`pebble`; the live URL is:

```
https://damosullivan.github.io/pebble/ten-forty/config.html
```

`ten-forty/src/pkjs/index.js` carries that URL in `CONFIG_URL`. It has a `?v=`
query because Pages caches HTML: bump it whenever the page or its AppMessage
keys change. The page is generated by `ten-forty/scripts/config-page.js`; do not
hand-edit `docs/ten-forty/config.html`.

Ten Forty 1.1.1 and earlier still point at the old `/ten-forty/` URL. A 1.1.2
release with the new `CONFIG_URL` is needed for the settings page to work again
on the watch; until then, only the code in this repo knows the new path.

## Face-specific notes

The face-specific design history, settings ids, persist keys and measured
layout live in `ten-forty/AGENTS.md`; Bauhaus's measured geometry and drawing
notes live in `bauhaus/AGENTS.md`. Do not duplicate them here.

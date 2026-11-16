# Store listing — Bauhaus

## Facts

- **Name:** Bauhaus
- **UUID:** `93c8b851-8d18-4fbf-8050-a6efb9c1023f` (permanent at submission)
- **Version:** 1.0.0
- **Category:** Watchface
- **Platforms:** aplite, basalt, flint, chalk, diorite, emery, gabbro
- **No settings, no phone JS.**
- Dashboard: https://developer.repebble.com/dashboard

## Description

> A quiet instrument dial for Pebble. A white dial, a thin black ring, ten
> ticks and a triangle at 12, plain baton hands and a black date window at 3.
> Drawn entirely from vector primitives and the system font: no numerals, no
> second hand, no settings. Works on every Pebble, including the 1-bit screens.

## Screenshots

Two per platform, exact display size, unframed:

```
screenshots/aplite_1.0.0_1-ten-ten.png
screenshots/aplite_1.0.0_2-four-forty-five.png
... and the same pair for basalt, flint, chalk, diorite, emery, gabbro
```

Filenames start with the platform, as `pebble publish --non-interactive`
requires. `scripts/capture-screenshots.sh` regenerates the set with the date
pinned to 2026-11-16.

## Name check

There is already a store watchface called **Bauhaus** (an abstract red-disc and
blue-field composition), plus several "Bauhaus Time" variants. This project
keeps the working name deliberately: the launcher and store will show a
duplicate name, and the two faces are visually unrelated. If the store rejects
it or the duplicate becomes a problem, **Bauhaus Dial** was the checked
fallback and no exact listing used it.

## Release checklist

1. Check the name on the store.
2. Bump `version` to a fresh number (numbers and dots only; never reuse one).
3. `pebble build`.
4. `./scripts/capture-screenshots.sh`, and check the images are exact size.
5. Publish **Unlisted**.
6. Install from the store on the watch, verify on real hardware.
7. Flip to **Listed**.

First publish:

```sh
pebble publish \
  --pbw build/bauhaus.pbw \
  --release-notes "First release." \
  --no-gif-all-platforms \
  --non-interactive \
  --name "Bauhaus" \
  --version 1.0.0 \
  --description "<the paragraph above>" \
  --source "https://github.com/damosullivan/pebble" \
  --screenshots screenshots/*_1.0.0_*.png
```

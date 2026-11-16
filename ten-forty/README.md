<h1 align="center">Ten Forty</h1>

<p align="center"><em>The time, spelled. Three stacked words in Iosevka —<br>
the hour bold, the minutes light.</em></p>

<p align="center">
  <img src="images/basalt.png" height="150" alt="Ten Forty on a Pebble Time: eleven, thirty, one">
  <img src="images/basalt-light.png" height="150" alt="Light theme: ten, forty">
  <img src="images/chalk.png" height="150" alt="Ten Forty on a Pebble Time Round: one o'clock">
</p>

---

Ten Forty is a watchface for Pebble. No digits, no hands, no ticks, no
complications: `ten` / `forty`, and nothing else unless you ask it for the
date. Two rows at ten forty. Three at eleven thirty-one. That is the whole
face.

It is set in [Iosevka](https://github.com/be5invis/Iosevka), monospaced, so a
row is exactly as wide as its letters and changing case never moves anything.
The hour carries the weight; the minutes are light, and the eye reads them as
one sentence broken over three lines.

## Build

From this directory (or `cd ten-forty` from the repo root):

```sh
./scripts/fetch-fonts.sh
pebble build
pebble install --emulator basalt
```

`fetch-fonts.sh` pulls Iosevka, which is 24MB and not committed. `pebble
build` writes `build/ten-forty.pbw`.

On a watch, turn on **Dev Connection** in the Pebble app and use
`pebble install --cloudpebble`. The store listing is
[Ten Forty](https://apps.repebble.com/719f27f4b48048d080225d6b).

## Platforms

Basalt, flint, chalk, emery and gabbro. The 1-bit screens — aplite and
diorite — are not targeted: three custom fonts do not fit in 24KB of heap.

<p align="center">
  <img src="images/emery.png" height="130" alt="Ten Forty on a Pebble 2">
  <img src="images/gabbro.png" height="130" alt="Ten Forty on a Pebble Time 2">
</p>

## Settings

Seven settings. All presentational, all persisted on the watch. The phone page
is at
[damosullivan.github.io/pebble/ten-forty/config.html](https://damosullivan.github.io/pebble/ten-forty/config.html).

| Setting | Values |
|---|---|
| Theme | white on black, black on white |
| Alignment | left, centre or right — one axis for the whole face |
| Date | hidden, or shown at the bottom as `Tue 10` |
| Single-digit minutes | `seven oh five` or `seven five` |
| Font weight | light or regular |
| Letter case | lowercase or UPPERCASE |
| Transition | none, or the hour only |

The hour slides: it is the one change anyone notices. The minute rows cut
instantly; `No animation` cuts the hour too.

## Notes

**Thirteen takes a hyphen.** A row holds seven characters at the minute size,
so the eight-letter teens are cut across the two minute rows. *four*, *seven*,
*eight* and *nine* read as two fragments the eye joins by itself. *thir* does
not, so thirteen alone is `thir-` — the mark for a word broken across a line.

**The circle is the reason chalk runs smaller.** The top row's ascenders sit
where a round screen is narrowest, so anything tall at the left edge loses its
tip. Round platforms therefore set smaller type, and centre the words by
default.

**There is no 24-hour mode.** Spelled out it does not fit: *seventeen* wants a
30px font and *twenty-three* a 22px one, against 37px normally. It would need
a different idea, not a setting.

More in [AGENTS.md](AGENTS.md): the design decisions, the gotchas, and the
things that were tried and removed. Store copy is in [STORE.md](STORE.md).

## Licence

MIT. The word-splitting and slide approach comes from Pebble's `sliding-text`
example; the notice is in [LICENSE.md](LICENSE.md) and travels with the code.
Iosevka is under the SIL Open Font License 1.1.

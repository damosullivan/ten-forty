# Bauhaus — watch face spec

Working name: **Bauhaus**. A quiet instrument dial for Pebble, built from the
watch's own primitives.

## The idea

A white dial, a thin black ring, twelve ticks, two plain baton hands, and a
black square date window at 3. The triangle at 12, the square date and the
circle of the dial are the three primitives. No numerals, no logo, no second
hand, no animation.

Ten Forty imports a typeface and spells the time. This uses the device's own
vocabulary and draws it.

## Principles

- **Form follows function.** The line gives the time; the triangle marks 12;
  the square holds the date. Remove any of them and the face reads worse.
- **Less is more.** No numerals, no branding, no chapter ring, no seconds, no
  transitions. The dial's job is to be read in half a second.
- **Honest materials.** Vector primitives and the Pebble system font only. No
  custom typeface, no bitmaps. The 1-bit screens are part of the brief, not an
  afterthought.
- **Legibility first.** High contrast, thin batons, clear hour marks, and the
  date at 3 o'clock where the circle is widest.
- **No ornament.** Colour is optional and functional; the monochrome face is
  the default.

## The face

- **Dial:** 120px diameter on the 144x168 reference, centred; scale with screen
  width. 1px ring.
- **Marks:** twelve 1x4px ticks; 6 and 9 slightly longer; a 7x7px filled
  triangle at 12; no tick at 3, where the date window sits.
- **Hands:** hour 30px x 3px, minute 48px x 2px, plain batons with rounded ends;
  a 4px centre cap. The mockup is at 10:10. No second hand.
- **Date:** a 22x14px black square at 3 o'clock, the day number reversed out in
  the system font at about 10px. No weekday.
- **Colour:** black on white, or inverted. An optional single red date window on
  colour screens; never required.
- **Motion:** none. The minute hand jumps once a minute; the hour hand moves in
  half-degree steps. No timer between minutes.
- **Type:** system font for the date only. The dial has no text.

## Platform

- Targets basalt, flint, chalk, emery, gabbro. The vector drawing and system
  font also make aplite and diorite possible, unlike Ten Forty.
- Draw with lines, circles/ellipses, polygons and rectangles. No custom font or
  bitmap resources.
- Round screens: the dial fills the circle; the date window sits at the wide 3
  o'clock point. Rectangles: the dial is centred; the corners stay empty.
- Quick View: decide whether the dial shrinks into the unobstructed area or the
  date is dropped first.

## Non-goals

Numerals on the dial; a logo or name; a seconds hand; a slide or fade; a custom
typeface; a bezel; a chapter ring; red/blue/yellow colour blocks. No settings
page unless the face needs one; one correct face is the starting point.

## References

- `reference/sheet.png` — four variants: black date, red date, inverted, date
  below the dial.
- `reference/hero.png` — 4x hero of the monochrome face.
- Junghans max bill and NOMOS Tangente for proportion and restraint. Do not copy
  their branding.

## Open questions

- Date window at 3 vs. date below the dial. Default: 3.
- Light/dark: a setting, or two builds? Default: light dial.
- A second hand: no, unless it can be done without a per-second timer.
- The name: "Bauhaus" is descriptive and generic. Check the store listing and
  trademark use before publishing.

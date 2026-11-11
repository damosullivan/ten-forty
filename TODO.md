# TODO — publishing the watchface

Parked 2026-11-11. Nothing here is started. Research done, decisions open.

Current build: `ten-forty/`, 198 KB pbw, five platforms, SDK 4.44.
Store: https://apps.repebble.com · Dashboard: https://developer.repebble.com/dashboard

## Facts worth keeping

- **No review queue.** Apps go live the moment you submit.
- Three routes, same listing: Developer Dashboard (only place to *edit* a
  listing later), `pebble publish`, CloudPebble.
- **UUID is permanent.** It is how a new release is matched to an existing
  listing. Everything else is reversible; this is not.
- `watchapp.watchface` sets watchface vs watchapp and **cannot be changed**
  after submission.
- Version must be numbers and dots only, each segment <= 255, and no version
  may be reused. First release should be `1.0.0`.
- PBW limit 4.4 MB (we are 198 KB). Each image also 4.4 MB.
- Screenshots: exact platform display size, **unframed** (no watch outline),
  up to 5 per platform. `pebble screenshot --all-platforms` already writes
  correctly sized, correctly named files to `screenshots/`.
- Banner 720x320 optional. **Icons are watchapp-only** — a watchface needs
  none; they are generated from the name.
- Visibility: Listed / Unlisted / Hidden, changeable any time.
- CI path exists: `PEBBLE_FIREBASE_ID_TOKEN` + `pebble publish --non-interactive`.

## Blocking

- [ ] **Freeze the UUID.** Currently `6f1c2b40-8a3d-4e57-9c21-7b5e0d4a9f13`.
      Decide once, then never change it.
- [x] **Name: Ten Forty.** Applied to `displayName`, README, STORE.md and the
      project directory. Store index checked: free.
- [ ] **Host the config page.** `Pebble.openURL` uses a `data:` URI, which
      iOS refuses to load — every iPhone user gets a dead settings button.
      Host `config.html`, set `CONFIG_URL` in `src/pkjs/index.js`.
- [x] **MIT attribution.** Done: `LICENSE.md` carries the upstream MIT notice
      and the font licence pointer. It is the only file that names the upstream
      project; the README and store copy do not.

## Ready

- [x] Released SDK build (4.44)
- [x] PBW under 4.4 MB
- [x] Five platforms: basalt, flint, chalk, emery, gabbro
- [x] Config page works (Android at least)

## To do

- [ ] Write the description — one plain-text paragraph.
- [ ] Bump `version` to `1.0.0` and rebuild.
- [ ] Capture 3-5 screenshots per platform. Currently one each. Worth showing
      a different time and the light theme.
- [ ] Decide on a banner (optional, 720x320).
- [ ] Decide whether to publish the source (source URL is optional but adds
      credibility, and we are standing on MIT code).
- [ ] Publish as **Unlisted**, install from the store on the real watch,
      confirm settings still work.
- [ ] Flip to **Listed**.

## Notes

- Adding a setting renumbers the AppMessage ids (assigned alphabetically).
  `build/js/message_keys.json` is the source of truth; `README.md` has the
  current table.
- Aplite and diorite are not targeted. 1-bit screens plus a 24KB heap make
  three custom fonts impractical; would need a fallback to the system Bitham fonts.
- If we ever add aplite, note the store maps binaries to wider hardware:
  an aplite binary also serves basalt/diorite/flint/emery, a basalt binary
  also serves emery, chalk also serves gabbro. Building for newer platforms
  directly is what gets their full display.

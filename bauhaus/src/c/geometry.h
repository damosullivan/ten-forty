/* Bauhaus geometry.
 *
 * Pure integer maths, no Pebble headers, so test/test_geometry.c can compile
 * exactly what the watch runs. The reference is the concept sheet: a 120px
 * dial on a 144x168 screen. Every length below is a reference pixel; bh_scale
 * turns it into device pixels.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  int16_t x, y, w, h;
} BhRect;

typedef struct {
  int16_t screen_w, screen_h;

  int16_t cx, cy; /* dial centre */
  int16_t r;      /* dial radius */
  int16_t dial;   /* dial diameter, always even so the centre lands on a pixel */

  int16_t tick_inner;      /* a regular tick starts this far inside the ring */
  int16_t tick_long_inner; /* ... and 6 and 9 start this far inside */
  int16_t tick_outer;      /* every tick ends this far inside the ring */

  int16_t tri_w;    /* base width */
  int16_t tri_tip;  /* tip, this far inside the ring */
  int16_t tri_base; /* base, this far inside the ring */

  int16_t hour_len, hour_w;
  int16_t min_len, min_w;
  int16_t cap_d;

  int16_t date_w, date_h;
  int16_t date_right; /* dial centre to the window's right edge */
  bool date_shown;
} BhMetrics;

static inline int16_t bh_scale(int16_t v, int16_t dial) {
  return (int16_t)(((int32_t)v * dial + 60) / 120);
}

static inline BhMetrics bh_metrics(int16_t screen_w, int16_t screen_h, BhRect vis) {
  BhMetrics m = {0};
  m.screen_w = screen_w;
  m.screen_h = screen_h;

  /* The sheet's 120/144 ratio, rounded, then evened off. Quick View gives back
   * a shorter area: the date goes first, then the dial shrinks only as far as
   * it must to clear the notification. */
  int16_t dial = (int16_t)(((int32_t)120 * screen_w + 72) / 144);
  m.date_shown = (vis.h >= screen_h);
  if (!m.date_shown) {
    int16_t fit = (int16_t)(vis.h - 8); /* 4px of air top and bottom */
    if (dial > fit) dial = fit;
  }
  if (dial < 40) dial = 40;
  dial &= ~1;

  m.dial = dial;
  m.r = dial / 2;
  m.cx = (int16_t)(vis.x + vis.w / 2);
  m.cy = (int16_t)(vis.y + vis.h / 2);

  m.tick_inner = bh_scale(5, dial);
  m.tick_long_inner = bh_scale(7, dial);
  m.tick_outer = bh_scale(1, dial);

  m.tri_w = bh_scale(7, dial);
  m.tri_tip = bh_scale(5, dial);
  m.tri_base = bh_scale(12, dial);

  m.hour_len = bh_scale(30, dial);
  m.hour_w = bh_scale(3, dial);
  m.min_len = bh_scale(48, dial);
  m.min_w = bh_scale(2, dial);
  m.cap_d = bh_scale(4, dial);

  m.date_w = bh_scale(22, dial);
  m.date_h = bh_scale(14, dial);
  m.date_right = bh_scale(45, dial);

  return m;
}

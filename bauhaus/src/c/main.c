/* Bauhaus: a quiet instrument dial.
 *
 * A white dial, a 1px ring, ten ticks and a triangle at 12, two batons and a
 * black date window at 3. Everything is vector; the only type is the day
 * number in the system font. No settings, no second hand, no animation: the
 * face is redrawn once a minute.
 */
#include <pebble.h>

#include "geometry.h"

static Window *s_window;
static Layer *s_canvas;

static BhMetrics s_m;
static int s_hour;
static int s_minute;
static char s_date_text[4];

/* ------------------------------------------------------------------ maths */

static int32_t div_round(int32_t n, int32_t d) {
  return n >= 0 ? (n + d / 2) / d : -((-n + d / 2) / d);
}

/* A point on the dial. Angle 0 is 12 o'clock and turns clockwise. */
static GPoint polar(int16_t radius, int32_t angle) {
  return (GPoint){
      .x = (int16_t)(s_m.cx +
                     div_round((int32_t)sin_lookup(angle) * radius, TRIG_MAX_RATIO + 1)),
      .y = (int16_t)(s_m.cy -
                     div_round((int32_t)cos_lookup(angle) * radius, TRIG_MAX_RATIO + 1)),
  };
}

/* graphics_draw_circle leaves gaps at 1-bit, so the ring is a closed run of
 * short chords. At 3 degrees a chord is about 3px on the reference dial. */
static void draw_ring(GContext *ctx) {
  const int16_t segments = 120;
  GPoint prev = polar(s_m.r, 0);
  for (int16_t i = 1; i <= segments; i++) {
    const int32_t angle = (int32_t)(i % segments) * TRIG_MAX_ANGLE / segments;
    const GPoint p = polar(s_m.r, angle);
    graphics_draw_line(ctx, prev, p);
    prev = p;
  }
}

/* A baton as a quad. gpath_draw_filled leaves a 2px-wide shape partly empty,
 * and parallel lines stripe, so the quad is filled scanline by scanline. */
static void hand_points(GPoint *p, int16_t len, int16_t width, int32_t angle) {
  const int32_t sin = sin_lookup(angle);
  const int32_t cos = cos_lookup(angle);
  const int16_t cap = width / 2;
  const int16_t reach = len - cap;

  const int32_t perp_x = div_round(cos * width * 8, TRIG_MAX_RATIO);
  const int32_t perp_y = div_round(sin * width * 8, TRIG_MAX_RATIO);
  const int32_t tip_x =
      (int32_t)s_m.cx * 16 + div_round(sin * reach * 16, TRIG_MAX_RATIO + 1);
  const int32_t tip_y =
      (int32_t)s_m.cy * 16 - div_round(cos * reach * 16, TRIG_MAX_RATIO + 1);
  const int32_t base_x = (int32_t)s_m.cx * 16;
  const int32_t base_y = (int32_t)s_m.cy * 16;

  p[0] = GPoint((int16_t)((tip_x - perp_x + 8) >> 4),
                (int16_t)((tip_y - perp_y + 8) >> 4));
  p[1] = GPoint((int16_t)((tip_x + perp_x + 8) >> 4),
                (int16_t)((tip_y + perp_y + 8) >> 4));
  p[2] = GPoint((int16_t)((base_x + perp_x + 8) >> 4),
                (int16_t)((base_y + perp_y + 8) >> 4));
  p[3] = GPoint((int16_t)((base_x - perp_x + 8) >> 4),
                (int16_t)((base_y - perp_y + 8) >> 4));
}

static void fill_quad(GContext *ctx, const GPoint *p) {
  int16_t top = p[0].y;
  int16_t bottom = p[0].y;
  for (int i = 1; i < 4; i++) {
    if (p[i].y < top) top = p[i].y;
    if (p[i].y > bottom) bottom = p[i].y;
  }

  for (int16_t y = top; y <= bottom; y++) {
    int16_t x_min = 32767;
    int16_t x_max = -32768;
    for (int i = 0; i < 4; i++) {
      const GPoint a = p[i];
      const GPoint b = p[(i + 1) & 3];
      if (a.y == b.y) continue;
      const int16_t lo = a.y < b.y ? a.y : b.y;
      const int16_t hi = a.y < b.y ? b.y : a.y;
      /* Half-open: include the top edge, exclude the bottom. Counting both
       * gives a 2px-wide horizontal hand three rows and makes it as heavy as
       * the hour hand. */
      if (y < lo || y >= hi) continue;
      const int16_t x = (int16_t)(a.x + (int32_t)(b.x - a.x) * (y - a.y) /
                                            (b.y - a.y));
      if (x < x_min) x_min = x;
      if (x > x_max) x_max = x;
    }
    if (x_min <= x_max) {
      graphics_draw_line(ctx, GPoint(x_min, y), GPoint(x_max, y));
    }
  }
}

static void draw_hand(GContext *ctx, int16_t len, int16_t width,
                      int32_t angle) {
  GPoint pts[4];
  hand_points(pts, len, width, angle);
  fill_quad(ctx, pts);

  const int16_t cap = width / 2;
  if (cap > 0) {
    graphics_fill_circle(ctx, polar(len - cap, angle), cap);
  }
}

static void draw_triangle(GContext *ctx) {
  /* Filled row by row. gpath_draw_filled leaves the boundary pixels out, which
   * turns a 7x7 triangle into a 5x5 one; horizontal lines cover exactly the
   * rows the shape owns. */
  const int16_t tip_y = (int16_t)(s_m.cy - (s_m.r - s_m.tri_tip));
  const int16_t base_y = (int16_t)(s_m.cy - (s_m.r - s_m.tri_base));
  const int16_t rows = (int16_t)(base_y - tip_y);
  if (rows <= 1) return;

  for (int16_t row = 0; row < rows; row++) {
    const int16_t half =
        (int16_t)(((int32_t)s_m.tri_w * row) / (2 * (rows - 1)));
    const int16_t y = (int16_t)(tip_y + row);
    graphics_draw_line(ctx, GPoint((int16_t)(s_m.cx - half), y),
                       GPoint((int16_t)(s_m.cx + half), y));
  }
}

/* ----------------------------------------------------------------- drawing */

static void draw_dial(GContext *ctx) {
  draw_ring(ctx);

  for (int i = 1; i < 12; i++) {
    if (i == 3) continue; /* the date window lives at 3 */
    const int16_t inner =
        (i == 6 || i == 9) ? s_m.tick_long_inner : s_m.tick_inner;
    const int32_t angle = i * (TRIG_MAX_ANGLE / 12);
    graphics_draw_line(ctx, polar(s_m.r - inner, angle),
                       polar(s_m.r - s_m.tick_outer, angle));
  }

  draw_triangle(ctx);
}

static GFont date_font(int16_t dial) {
  if (dial >= 190) return fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  if (dial >= 135) return fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  return fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
}

static void draw_date_window(GContext *ctx) {
  const GRect win = GRect(s_m.cx + s_m.date_right - s_m.date_w,
                          s_m.cy - s_m.date_h / 2, s_m.date_w, s_m.date_h);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, win, 0, GCornerNone);
}

/* Drawn after the hands. The black window already swallows the hand; putting
 * the digits on top of that means the day number is never obscured, which is
 * the one thing the window exists for. */
static void draw_date_digits(GContext *ctx) {
  const GRect win = GRect(s_m.cx + s_m.date_right - s_m.date_w,
                          s_m.cy - s_m.date_h / 2, s_m.date_w, s_m.date_h);
  GFont font = date_font(s_m.dial);
  const GRect probe = GRect(0, 0, (int16_t)(s_m.date_w * 2), 64);
  const GSize ink = graphics_text_layout_get_content_size(
      s_date_text, font, probe, GTextOverflowModeFill, GTextAlignmentCenter);

  /* The line box can be taller than the window (Gothic 14 reports 18px) and
   * the digits sit low in it, so centre the box and then lift by the measured
   * amount. Calibrated from the emulator, not derived from the line height. */
  const GRect box = GRect(win.origin.x + (win.size.w - ink.w) / 2,
                          win.origin.y + (win.size.h - ink.h) / 2 - 3,
                          ink.w, ink.h);
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(ctx, s_date_text, font, box, GTextOverflowModeFill,
                     GTextAlignmentCenter, NULL);
}

static BhRect visible_bounds(void) {
#if defined(_PBL_API_EXISTS_layer_get_unobstructed_bounds)
  const GRect g = layer_get_unobstructed_bounds(s_canvas);
  return (BhRect){g.origin.x, g.origin.y, g.size.w, g.size.h};
#else
  const GRect g = layer_get_bounds(s_canvas);
  return (BhRect){g.origin.x, g.origin.y, g.size.w, g.size.h};
#endif
}

static void canvas_update(Layer *layer, GContext *ctx) {
  const GRect bounds = layer_get_bounds(layer);
  graphics_context_set_antialiased(ctx, false);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  s_m = bh_metrics(bounds.size.w, bounds.size.h, visible_bounds());
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorBlack);

  draw_dial(ctx);
  if (s_m.date_shown && s_date_text[0]) {
    draw_date_window(ctx);
  }

  const int32_t min_angle = (int32_t)s_minute * TRIG_MAX_ANGLE / 60;
  const int32_t hour_angle =
      (int32_t)(s_hour % 12) * (TRIG_MAX_ANGLE / 12) +
      (int32_t)s_minute * (TRIG_MAX_ANGLE / 720);

  draw_hand(ctx, s_m.hour_len, s_m.hour_w, hour_angle);
  draw_hand(ctx, s_m.min_len, s_m.min_w, min_angle);

  if (s_m.date_shown && s_date_text[0]) {
    draw_date_digits(ctx);
  }

  graphics_fill_circle(ctx, GPoint(s_m.cx, s_m.cy), s_m.cap_d / 2);
}

/* -------------------------------------------------------------- lifecycle */

static void update_time(struct tm *t) {
  s_hour = t->tm_hour;
  s_minute = t->tm_min;
  snprintf(s_date_text, sizeof(s_date_text), "%d", t->tm_mday);
}

static void tick_handler(struct tm *t, TimeUnits units_changed) {
  (void)units_changed;
  update_time(t);
  layer_mark_dirty(s_canvas);
}

#if defined(_PBL_API_EXISTS_unobstructed_area_service_subscribe)
static void unobstructed_change(AnimationProgress progress, void *context) {
  (void)progress;
  (void)context;
  layer_mark_dirty(s_canvas);
}
#endif

static void window_load(Window *window) {
  const GRect bounds = layer_get_bounds(window_get_root_layer(window));
  s_canvas = layer_create(bounds);
  layer_set_update_proc(s_canvas, canvas_update);
  layer_add_child(window_get_root_layer(window), s_canvas);

  time_t now = time(NULL);
  update_time(localtime(&now));

#if defined(_PBL_API_EXISTS_unobstructed_area_service_subscribe)
  unobstructed_area_service_subscribe(
      (UnobstructedAreaHandlers){.change = unobstructed_change}, NULL);
#endif

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  layer_mark_dirty(s_canvas);
}

static void window_unload(Window *window) {
  (void)window;
  tick_timer_service_unsubscribe();
#if defined(_PBL_API_EXISTS_unobstructed_area_service_subscribe)
  unobstructed_area_service_unsubscribe();
#endif

  layer_destroy(s_canvas);
}

int main(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorWhite);
  window_set_window_handlers(
      s_window, (WindowHandlers){.load = window_load, .unload = window_unload});
  window_stack_push(s_window, true);
  app_event_loop();
  window_destroy(s_window);
}

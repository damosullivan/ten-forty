#include <pebble.h>
#include "words.h"

/* ------------------------------------------------------------------ layout */

#define ROWS 3
#define MARGIN_X 6
#define ANIM_FRAME_MS 40

/* ------------------------------------------------------------------- fonts
 * Sizes are picked per screen so each platform fills its width the same way.
 * Grouped by display width; the 144px group covers basalt and flint.
 */
#if defined(PBL_PLATFORM_CHALK)
  #define RID_HOUR    RESOURCE_ID_FONT_HOUR_48
  #define RID_MIN_L   RESOURCE_ID_FONT_MIN_LIGHT_40
  #define RID_MIN_R   RESOURCE_ID_FONT_MIN_REG_40
  #define RID_DATE_L  RESOURCE_ID_FONT_DATE_LIGHT_17
  #define RID_DATE_R  RESOURCE_ID_FONT_DATE_REG_17
#elif defined(PBL_PLATFORM_EMERY)
  #define RID_HOUR    RESOURCE_ID_FONT_HOUR_61
  #define RID_MIN_L   RESOURCE_ID_FONT_MIN_LIGHT_51
  #define RID_MIN_R   RESOURCE_ID_FONT_MIN_REG_51
  #define RID_DATE_L  RESOURCE_ID_FONT_DATE_LIGHT_22
  #define RID_DATE_R  RESOURCE_ID_FONT_DATE_REG_22
#elif defined(PBL_PLATFORM_GABBRO)
  #define RID_HOUR    RESOURCE_ID_FONT_HOUR_70
  #define RID_MIN_L   RESOURCE_ID_FONT_MIN_LIGHT_59
  #define RID_MIN_R   RESOURCE_ID_FONT_MIN_REG_59
  #define RID_DATE_L  RESOURCE_ID_FONT_DATE_LIGHT_26
  #define RID_DATE_R  RESOURCE_ID_FONT_DATE_REG_26
#else /* basalt, flint */
  #define RID_HOUR    RESOURCE_ID_FONT_HOUR_44
  #define RID_MIN_L   RESOURCE_ID_FONT_MIN_LIGHT_37
  #define RID_MIN_R   RESOURCE_ID_FONT_MIN_REG_37
  #define RID_DATE_L  RESOURCE_ID_FONT_DATE_LIGHT_16
  #define RID_DATE_R  RESOURCE_ID_FONT_DATE_REG_16
#endif

typedef enum {
  MOVING_IN,
  IN_FRAME,
  PREPARE_TO_MOVE,
  MOVING_OUT
} SlideState;

typedef struct {
  SlideState state;
  char text[2][24];
  uint8_t buf;        /* which of the two strings is on screen */
  int16_t x;          /* horizontal offset, 0 == at rest */
  int16_t delay;      /* frames to hold before sliding out */
  int16_t delay_count;
  bool bold;
} Row;

typedef struct {
  Window *window;
  Layer *canvas;

  Row rows[ROWS];
  GFont f_bold, f_light, f_date;
  int16_t h_bold, h_light, h_date;

  AppTimer *timer;   /* non-NULL only while rows are moving */
  int16_t pitch;     /* baseline-to-baseline distance */

  int last_hour;
  int last_minute;
  int last_day;

  uint8_t theme;     /* 0 dark, 1 light */
  uint8_t align;     /* 0 left, 1 centre, 2 right  -- the three time rows */
  uint8_t datealign; /* 0 left, 1 centre, 2 right  -- the date row */
  uint8_t datepos;   /* 0 off, 1 top, 2 bottom */
  uint8_t oh;        /* 1 = "oh five", 0 = "five" */
  uint8_t regular;   /* 0 = Iosevka Light, 1 = Iosevka Regular */
  uint8_t caps;      /* 0 lower, 1 UPPER, 2 Capital */
  uint8_t trans;     /* 0 none, 1 slide every row, 2 slide the hour only */

  char date_text[16];
} App;

static App *app;

static bool any_moving(void);

/* ------------------------------------------------------------------ colours */

static GColor bg_color(void)   { return app->theme ? GColorWhite : GColorBlack; }
static GColor fg_color(void)   { return app->theme ? GColorBlack : GColorWhite; }
static GColor dim_color(void)  { return app->theme ? GColorDarkGray : GColorLightGray; }

static GTextAlignment to_alignment(uint8_t v) {
  switch (v) {
    case 1:  return GTextAlignmentCenter;
    case 2:  return GTextAlignmentRight;
    default: return GTextAlignmentLeft;
  }
}

static GFont font_for(const Row *r) {
  return r->bold ? app->f_bold : app->f_light;
}

static int16_t height_for(const Row *r) {
  return r->bold ? app->h_bold : app->h_light;
}

/* ---------------------------------------------------------------- geometry */

/* Integer square root, used to work out how much of the screen a round
 * display actually has at a given row. */
static int16_t isqrt32(int32_t v) {
  if (v <= 0) return 0;
  int32_t x = v, y = (x + 1) / 2;
  while (y < x) { x = y; y = (x + v / x) / 2; }
  return (int16_t)x;
}

static int16_t margin_x(void) {
  GRect b = layer_get_bounds(app->canvas);
  return PBL_IF_ROUND_ELSE((int16_t)(b.size.w * 10 / 100), MARGIN_X);
}

static GRect visible_bounds(void) {
  return layer_get_unobstructed_bounds(window_get_root_layer(app->window));
}

/* The layout is tuned on the reference 144x168 screen and scaled from there,
 * so a 200x228 Time 2 gets the same proportions as a Time. */
static int16_t row_baseline(int i) {
  GRect b = visible_bounds();
  int16_t anchor = (int16_t)(((int32_t)b.size.h * 85) / 168);
  return anchor + (i - 1) * app->pitch;
}

static int16_t date_y(void) {
  GRect b = visible_bounds();
  return (app->datepos == 1) ? 1 : (int16_t)(b.size.h - app->h_date - 2);
}

/* ------------------------------------------------------------------ drawing */

static void canvas_update(Layer *layer, GContext *ctx) {
  GRect b = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, bg_color());
  graphics_fill_rect(ctx, b, 0, GCornerNone);

  const int16_t mx = margin_x();
  const int16_t w = b.size.w - mx * 2;

  graphics_context_set_text_color(ctx, fg_color());
  for (int i = 0; i < ROWS; i++) {
    Row *r = &app->rows[i];
    const char *txt = r->text[r->buf];
    if (!txt[0]) continue;
    int16_t h = height_for(r);
    int16_t asc = (int16_t)((int32_t)h * 86 / 100);
    GRect box = GRect(mx + r->x, row_baseline(i) - asc, w, h);
    graphics_draw_text(ctx, txt, font_for(r), box,
                       GTextOverflowModeFill, to_alignment(app->align), NULL);
  }

  if (app->datepos && app->date_text[0]) {
    GRect box = GRect(mx, date_y(), w, app->h_date);
#if defined(PBL_ROUND)
    /* Keep the date inside the circle instead of clipping it in the corner. */
    const int16_t r = b.size.w / 2;
    int16_t dy = (int16_t)(box.origin.y + app->h_date / 2 - r);
    if (dy < 0) dy = (int16_t)-dy;
    int16_t chord = isqrt32((int32_t)r * r - (int32_t)dy * dy);
    if (chord > 12) {
      box.origin.x = (int16_t)(r - chord + 4);
      box.size.w = (int16_t)(chord * 2 - 8);
    }
#endif
    if (box.size.w > 12) {
      graphics_context_set_text_color(ctx, dim_color());
      graphics_draw_text(ctx, app->date_text, app->f_date, box,
                         GTextOverflowModeFill, to_alignment(app->datealign), NULL);
    }
  }
}

/* ---------------------------------------------------------------- animation */

static int16_t abs16(int16_t v) { return v < 0 ? (int16_t)-v : v; }

static void row_set_text(Row *r, const char *s, bool animate) {
  if (strcmp(r->text[r->buf], s) == 0) {
    return;
  }

  if (!animate) {
    /* Cut straight to the new word. Writing into the visible buffer keeps the
     * change atomic within this draw, and clears any slide that was pending. */
    strncpy(r->text[r->buf], s, sizeof(r->text[0]) - 1);
    r->text[r->buf][sizeof(r->text[0]) - 1] = '\0';
    r->x = 0;
    r->state = IN_FRAME;
    r->delay_count = 0;
    return;
  }

  char *next = r->text[r->buf ^ 1];
  strncpy(next, s, sizeof(r->text[0]) - 1);
  next[sizeof(r->text[0]) - 1] = '\0';

  if (r->text[r->buf][0] == '\0') {
    /* nothing on screen yet: fly in from the right */
    r->buf ^= 1;
    r->x = layer_get_bounds(app->canvas).size.w;
    r->state = MOVING_IN;
  } else {
    r->state = PREPARE_TO_MOVE;
    r->delay_count = 0;
  }
}

static bool row_step(Row *r) {
  const int16_t w = layer_get_bounds(app->canvas).size.w;

  switch (r->state) {
    case PREPARE_TO_MOVE:
      if (++r->delay_count > r->delay) {
        r->state = MOVING_OUT;
        r->delay_count = 0;
      }
      return true;

    case MOVING_IN: {
      r->x -= abs16(r->x) / 3 + 1;
      if (r->x <= 0) {
        r->x = 0;
        r->state = IN_FRAME;
      }
      return true;
    }

    case MOVING_OUT: {
      r->x -= abs16(r->x) / 3 + 1;
      if (r->x <= -w) {
        r->x = w;
        r->buf ^= 1;
        r->state = MOVING_IN;
        r->delay_count = 0;
      }
      return true;
    }

    case IN_FRAME:
    default:
      return false;
  }
}

/* A frame of the slide. The timer is only alive while something is moving, so
 * the watch idles at zero CPU cost between minutes. */
static void anim_frame(void *data) {
  app->timer = NULL;

  bool changed = false;
  for (int i = 0; i < ROWS; i++) {
    changed = row_step(&app->rows[i]) || changed;
  }

  if (changed) {
    layer_mark_dirty(app->canvas);
    app->timer = app_timer_register(ANIM_FRAME_MS, anim_frame, NULL);
  }
}

static void start_animation(void) {
  if (!app->timer) {
    app->timer = app_timer_register(ANIM_FRAME_MS, anim_frame, NULL);
  }
}

/* -------------------------------------------------------------- time & date */

/* Iosevka is monospace, so changing case never changes a row's width --
 * the layout does not need to know about this. */
static char *to_case(char *s) {
  if (app->caps == 1) {
    for (char *p = s; *p; p++) {
      if (*p >= 'a' && *p <= 'z') *p = (char)(*p - 'a' + 'A');
    }
  } else if (app->caps == 2 && s[0] >= 'a' && s[0] <= 'z') {
    s[0] = (char)(s[0] - 'a' + 'A');
  }
  return s;
}

static void refresh_date(const struct tm *t) {
  static const char *const DAYS[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  snprintf(app->date_text, sizeof(app->date_text), "%s %d",
           DAYS[t->tm_wday], t->tm_mday);
  to_case(app->date_text);
  app->last_day = t->tm_mday;
}

static void refresh_time(const struct tm *t, bool animate) {
  char hour[24], tens[24], ones[24];

  hour_to_12h_word(t->tm_hour, hour);
  minute_to_words(t->tm_min, app->oh, tens, ones);

  const bool slide_all = animate && (app->trans == 1);
  const bool slide_hour = animate && (app->trans != 0);

  row_set_text(&app->rows[0], to_case(hour), slide_hour);
  row_set_text(&app->rows[1], to_case(tens), slide_all);
  row_set_text(&app->rows[2], to_case(ones), slide_all);

  app->last_hour = t->tm_hour;
  app->last_minute = t->tm_min;
}

static void tick_handler(struct tm *t, TimeUnits units_changed) {
  if (t->tm_mday != app->last_day) {
    refresh_date(t);
  }
  refresh_time(t, true);
  layer_mark_dirty(app->canvas);
  if (any_moving()) {
    start_animation();
  }
}

/* ---------------------------------------------------------------- type size */

static int16_t line_height(GFont f) {
  GRect probe = GRect(0, 0, 260, 260);
  return graphics_text_layout_get_content_size("Ag", f, probe,
             GTextOverflowModeFill, GTextAlignmentLeft).h;
}

static void measure(void) {
  app->h_bold = line_height(app->f_bold);
  app->h_light = line_height(app->f_light);
  app->h_date = line_height(app->f_date);
  /* Iosevka's reported line height is tight (== pixel height), so derive the
   * leading from the glyph size instead of the box height. */
  app->pitch = (int16_t)(((int32_t)app->h_bold * 107) / 100);
}

/* --------------------------------------------------------------- settings */

static void apply_theme(void) {
  window_set_background_color(app->window, bg_color());
  layer_mark_dirty(app->canvas);
}

/* Swap the minute/date faces between Iosevka Light and Regular. */
static void apply_weight(void) {
  fonts_unload_custom_font(app->f_light);
  fonts_unload_custom_font(app->f_date);

  if (app->regular) {
    app->f_light = fonts_load_custom_font(resource_get_handle(RID_MIN_R));
    app->f_date = fonts_load_custom_font(resource_get_handle(RID_DATE_R));
  } else {
    app->f_light = fonts_load_custom_font(resource_get_handle(RID_MIN_L));
    app->f_date = fonts_load_custom_font(resource_get_handle(RID_DATE_L));
  }

  measure();
  layer_mark_dirty(app->canvas);
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  app->theme ^= 1;
  persist_write_int(1, app->theme);
  apply_theme();
}

static void inbox_received(DictionaryIterator *it, void *context) {
  Tuple *t;
  bool dirty = false;
  bool weight_changed = false;

  if ((t = dict_find(it, MESSAGE_KEY_Theme))) {
    app->theme = t->value->int32 ? 1 : 0;
    persist_write_int(1, app->theme);
    dirty = true;
  }
  if ((t = dict_find(it, MESSAGE_KEY_Align))) {
    app->align = (uint8_t)t->value->int32;
    persist_write_int(2, app->align);
    dirty = true;
  }
  if ((t = dict_find(it, MESSAGE_KEY_DateAlign))) {
    app->datealign = (uint8_t)t->value->int32;
    persist_write_int(6, app->datealign);
    dirty = true;
  }
  if ((t = dict_find(it, MESSAGE_KEY_DatePos))) {
    app->datepos = (uint8_t)t->value->int32;
    persist_write_int(4, app->datepos);
    dirty = true;
  }
  if ((t = dict_find(it, MESSAGE_KEY_Case))) {
    app->caps = (uint8_t)t->value->int32;
    persist_write_int(8, app->caps);
    dirty = true;
  }
  if ((t = dict_find(it, MESSAGE_KEY_Transition))) {
    app->trans = (uint8_t)t->value->int32;
    persist_write_int(9, app->trans);
    dirty = true;
  }
  if ((t = dict_find(it, MESSAGE_KEY_OhWords))) {
    app->oh = t->value->int32 ? 1 : 0;
    persist_write_int(5, app->oh);
    dirty = true;
  }
  if ((t = dict_find(it, MESSAGE_KEY_Weight))) {
    uint8_t want = t->value->int32 ? 1 : 0;
    persist_write_int(7, want);
    if (want != app->regular) {
      app->regular = want;
      weight_changed = true;
    }
    dirty = true;
  }

  if (dirty) {
    /* The phone sends every key on save, so this must run whatever changed --
     * it is what makes a setting take effect without waiting for the next
     * minute tick. */
    if (weight_changed) {
      apply_weight();
    }
    apply_theme();
    time_t now = time(NULL);
    struct tm tm_now = *localtime(&now);
    refresh_time(&tm_now, false);
    layer_mark_dirty(window_get_root_layer(app->window));
    layer_mark_dirty(app->canvas);
    start_animation();
  }
}

static void inbox_dropped(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_WARNING, "message dropped: %d", (int)reason);
}

/* ------------------------------------------------------------- lifecycle */

static void unobstructed_change(AnimationProgress progress, void *context) {
  layer_mark_dirty(app->canvas);
}

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);

  app->canvas = layer_create(bounds);
  layer_set_update_proc(app->canvas, canvas_update);
  layer_add_child(root, app->canvas);
}

static void window_unload(Window *window) {
  layer_destroy(app->canvas);
}

static bool any_moving(void) {
  for (int i = 0; i < ROWS; i++) {
    if (app->rows[i].state != IN_FRAME) return true;
  }
  return false;
}

static void init(void) {
  app = (App *)malloc(sizeof(App));
  memset(app, 0, sizeof(App));

  app->theme     = persist_exists(1) ? persist_read_int(1) : 0;
  app->align     = persist_exists(2) ? persist_read_int(2) : 0;
  app->datepos   = persist_exists(4) ? persist_read_int(4) : 2;
  app->oh        = persist_exists(5) ? persist_read_int(5) : 1;
  app->datealign = persist_exists(6) ? persist_read_int(6) : 1;
  app->regular   = persist_exists(7) ? persist_read_int(7) : 0;
  app->caps      = persist_exists(8) ? persist_read_int(8) : 0;
  app->trans     = persist_exists(9) ? persist_read_int(9) : 2;

  app->last_hour = -1;
  app->last_minute = -1;
  app->last_day = -1;

  app->f_bold = fonts_load_custom_font(resource_get_handle(RID_HOUR));
  if (app->regular) {
    app->f_light = fonts_load_custom_font(resource_get_handle(RID_MIN_R));
    app->f_date = fonts_load_custom_font(resource_get_handle(RID_DATE_R));
  } else {
    app->f_light = fonts_load_custom_font(resource_get_handle(RID_MIN_L));
    app->f_date = fonts_load_custom_font(resource_get_handle(RID_DATE_L));
  }
  measure();

  for (int i = 0; i < ROWS; i++) {
    app->rows[i].bold = (i == 0);
    app->rows[i].delay = (int16_t)(6 - i * 3);
    app->rows[i].state = IN_FRAME;
  }

  app->window = window_create();
  window_set_background_color(app->window, bg_color());
  window_set_window_handlers(app->window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload,
  });

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  unobstructed_area_service_subscribe((UnobstructedAreaHandlers){
    .change = unobstructed_change,
  }, NULL);
  accel_tap_service_subscribe(tap_handler);

  app_message_register_inbox_received(inbox_received);
  app_message_register_inbox_dropped(inbox_dropped);
  app_message_open(128, 128);

  window_stack_push(app->window, true);

  time_t now = time(NULL);
  struct tm tm_now = *localtime(&now);
  refresh_date(&tm_now);
  refresh_time(&tm_now, false);
  layer_mark_dirty(app->canvas);
}

static void deinit(void) {
  if (app->timer) {
    app_timer_cancel(app->timer);
    app->timer = NULL;
  }

  tick_timer_service_unsubscribe();
  unobstructed_area_service_unsubscribe();
  accel_tap_service_unsubscribe();

  window_destroy(app->window);

  fonts_unload_custom_font(app->f_bold);
  fonts_unload_custom_font(app->f_light);
  fonts_unload_custom_font(app->f_date);

  free(app);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
  return 0;
}

#include <assert.h>
#include <stdio.h>

#include "geometry.h"

typedef struct {
  const char *name;
  int16_t w, h;
  int16_t quick_view_h;
} Screen;

static const Screen SCREENS[] = {
    {"aplite", 144, 168, 117},
    {"basalt", 144, 168, 117},
    {"chalk", 180, 180, 180},
    {"diorite", 144, 168, 117},
    {"emery", 200, 228, 169},
    {"flint", 144, 168, 117},
    {"gabbro", 260, 260, 260},
};

static void check_full(const Screen *s, BhMetrics m) {
  const int16_t date_left = (int16_t)(m.cx + m.date_right - m.date_w);
  const int16_t date_top = (int16_t)(m.cy - m.date_h / 2);
  const int16_t date_right = (int16_t)(date_left + m.date_w - 1);
  const int16_t date_bottom = (int16_t)(date_top + m.date_h - 1);

  assert(m.date_shown);
  assert(m.dial % 2 == 0);
  assert(m.cx - m.r >= 0 && m.cx + m.r < s->w);
  assert(m.cy - m.r >= 0 && m.cy + m.r < s->h);

  /* The window sits inside the dial, clear of the ring and the centre. */
  assert(m.date_right < m.r);
  assert(m.date_right > m.date_w);
  assert(date_left > m.cx);
  assert(date_right < m.cx + m.r);
  assert(date_top >= 0 && date_bottom < s->h);
  assert(date_top > m.cy - m.r && date_bottom < m.cy + m.r);

  /* Hands stop inside the ring; ticks start inside it. */
  assert(m.hour_len < m.r);
  assert(m.min_len < m.r);
  assert(m.hour_w >= 1 && m.min_w >= 1);
  assert(m.tick_outer < m.r);
  assert(m.tick_long_inner > m.tick_inner);
  assert(m.tri_tip > 0 && m.tri_tip < m.tri_base && m.tri_base < m.r);
  assert(m.tri_w > 0);
}

static void check_quick_view(const Screen *s, BhMetrics full, BhMetrics m) {
  if (s->quick_view_h >= s->h) {
    /* Chalk and gabbro report no obstruction; the face must not change. */
    assert(m.date_shown);
    assert(m.dial == full.dial);
    return;
  }

  assert(!m.date_shown);
  assert(2 * m.r <= s->quick_view_h - 8);
  assert(m.cy - m.r >= 0);
  assert(m.cy + m.r <= s->quick_view_h);
  assert(m.dial < full.dial);
  assert(m.dial >= 40);
}

int main(void) {
  puts("screen     dial   r   date       hour  minute  quick-view dial");
  for (size_t i = 0; i < sizeof(SCREENS) / sizeof(SCREENS[0]); i++) {
    const Screen *s = &SCREENS[i];
    const BhMetrics full = bh_metrics(s->w, s->h, (BhRect){0, 0, s->w, s->h});
    check_full(s, full);

    const BhRect qv = {0, 0, s->w, s->quick_view_h};
    const BhMetrics quick = bh_metrics(s->w, s->h, qv);
    check_quick_view(s, full, quick);

    printf("%-9s %4d %3d  %2dx%-2d     %2d/%-2d %2d/%-2d   %d\n", s->name,
           full.dial, full.r, full.date_w, full.date_h, full.hour_len,
           full.hour_w, full.min_len, full.min_w,
           quick.date_shown ? full.dial : quick.dial);
  }
  puts("ok");
  return 0;
}

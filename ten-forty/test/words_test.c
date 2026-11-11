/* Host-side test for the word generator: no watch, no SDK, no emulator.
 *
 *   test/run.sh
 *
 * The constraint being protected is that no row overflows its box. Iosevka is
 * monospace at 0.5em, but the rasteriser rounds each glyph advance UP to a
 * whole pixel, so at N px a row holds
 *
 *     box_width / ceil(N/2)
 *
 * characters. Getting this wrong truncates silently: "fifteen" rendered as
 * "fifte..." rather than failing anywhere visible. Sizes and box widths are
 * supplied by run.sh from package.json, so changing a font size cannot quietly
 * reintroduce the bug.
 */
#include <stdio.h>
#include <string.h>
#include "words.h"

typedef struct {
    const char *name;
    int hour_px, min_px, box;
} Platform;

static const Platform platforms[] = {
    { "basalt/flint", HOUR_BASALT, MIN_BASALT, BOX_BASALT },
    { "chalk",        HOUR_CHALK,  MIN_CHALK,  BOX_CHALK  },
    { "emery",        HOUR_EMERY,  MIN_EMERY,  BOX_EMERY  },
    { "gabbro",       HOUR_GABBRO, MIN_GABBRO, BOX_GABBRO },
};

static int capacity(int px, int box) { return box / ((px + 1) / 2); }

int main(void) {
    int failures = 0;
    int three_rows = 0;

    for (size_t p = 0; p < sizeof(platforms) / sizeof(platforms[0]); p++) {
        const Platform *pl = &platforms[p];
        const int hour_cap = capacity(pl->hour_px, pl->box);
        const int min_cap = capacity(pl->min_px, pl->box);

        printf("%-13s box %3d  hour %dpx (%d ch)  minute %dpx (%d ch)\n",
               pl->name, pl->box, pl->hour_px, hour_cap, pl->min_px, min_cap);

        for (int h = 0; h < 24; h++) {
            char hour[24];
            hour_to_12h_word(h, hour);
            if ((int)strlen(hour) > hour_cap) {
                printf("  FAIL hour %02d: '%s' is %zu chars\n", h, hour, strlen(hour));
                failures++;
            }
        }

        for (int m = 0; m < 60; m++) {
            char tens[24], ones[24];
            minute_to_words(m, 1, tens, ones);
            if ((int)strlen(tens) > min_cap || (int)strlen(ones) > min_cap) {
                printf("  FAIL minute %02d: '%s' / '%s'\n", m, tens, ones);
                failures++;
            }
            if (p == 0 && ones[0]) three_rows++;
        }
    }

    printf("\n%d of 60 minutes use all three rows\n", three_rows);

    if (failures) {
        printf("%d failure(s)\n", failures);
        return 1;
    }
    printf("all rows fit on every platform\n");
    return 0;
}

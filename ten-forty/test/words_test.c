/* Host-side test for the word generator: no compiler, no watch required.
 *
 *   test/run.sh
 *
 * The constraint being protected is that every row must fit seven characters
 * at the minute font size (Iosevka is monospace at 0.5em, so seven characters
 * is 3.5em wide, and the row box is 3.57em). Eight characters is silently
 * clipped by the text layer, which is how "thirteen" came to render as
 * "thirt...".
 */
#include <stdio.h>
#include <string.h>
#include "words.h"

#define MIN_ROW_MAX 7
#define HOUR_ROW_MAX 6

int main(void) {
    int failures = 0;

    for (int h = 0; h < 24; h++) {
        char hour[24];
        hour_to_12h_word(h, hour);
        if (strlen(hour) > HOUR_ROW_MAX) {
            printf("FAIL hour %02d: '%s' is %zu chars\n", h, hour, strlen(hour));
            failures++;
        }
    }

    int three_rows = 0;
    printf("two-row minutes:");
    for (int m = 0; m < 60; m++) {
        char tens[24], ones[24];
        minute_to_words(m, 1, tens, ones);
        if (strlen(tens) > MIN_ROW_MAX || strlen(ones) > MIN_ROW_MAX) {
            printf("FAIL minute %02d: '%s' / '%s'\n", m, tens, ones);
            failures++;
        }
        if (ones[0]) three_rows++;
        else printf(" %02d", m);
    }

    printf("\n%d of 60 minutes use all three rows\n", three_rows);

    if (failures) {
        printf("%d failure(s)\n", failures);
        return 1;
    }
    printf("all rows fit\n");
    return 0;
}

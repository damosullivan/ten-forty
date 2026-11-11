#include "words.h"

#include <stdio.h>
#include <string.h>

/* Adapted from Pebble's num2words (MIT, (c) 2015 Pebble Examples), with the
 * single-digit minutes changed from "seven five" to "seven oh five". */

static const char *const ONES[] = {
  "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"
};

static const char *const TENS[] = {
  "", "ten", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"
};

/* 10..19 that fit on a single row */
static const char *const TEENS_WHOLE[] = {
  "ten", "eleven", "twelve", "thirteen", NULL, "fifteen", "sixteen", NULL, NULL, NULL
};

/* 10..19 that split into two rows: "<head>" and "teen" */
static const char *const TEENS_SPLIT_HEAD[] = {
  NULL, NULL, NULL, NULL, "four", NULL, NULL, "seven", "eight", "nine"
};

static void copy(char *dst, const char *src) {
  strncpy(dst, src, 23);
  dst[23] = '\0';
}

void minute_to_words(int minutes, bool oh_words, char *tens, char *ones) {
  tens[0] = '\0';
  ones[0] = '\0';

  if (minutes <= 0) {
    copy(tens, "o'clock");
    return;
  }

  if (minutes < 10) {
    if (oh_words) {
      copy(tens, "oh");
      copy(ones, ONES[minutes]);
    } else {
      copy(tens, ONES[minutes]);
    }
    return;
  }

  if (minutes < 20) {
    int idx = minutes - 10;
    if (TEENS_WHOLE[idx]) {
      copy(tens, TEENS_WHOLE[idx]);
    } else {
      copy(tens, TEENS_SPLIT_HEAD[idx]);
      copy(ones, "teen");
    }
    return;
  }

  copy(tens, TENS[minutes / 10]);
  if (minutes % 10) {
    copy(ones, ONES[minutes % 10]);
  }
}

static const char *const HOURS_12[] = {
  "twelve", "one", "two", "three", "four", "five",
  "six", "seven", "eight", "nine", "ten", "eleven"
};

void hour_to_12h_word(int hours, char *word) {
  int idx = ((hours % 12) + 12) % 12;
  copy(word, HOURS_12[idx]);
}

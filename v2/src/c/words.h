#pragma once

#include <stdbool.h>

// Splits the minutes into the two lower rows.
// e.g. 5 -> tens="oh", ones="five"   17 -> tens="seven", ones="teen"
//      0 -> tens="o'clock", ones=""  30 -> tens="thirty", ones=""
// With `oh_words` false, 5 -> tens="five", ones=""
void minute_to_words(int minutes, bool oh_words, char *tens, char *ones);

// Spells 12-hour clock hours: 0 -> "twelve", 13 -> "one"
void hour_to_12h_word(int hours, char *word);

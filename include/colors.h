// LED colours as true R, G, B (the pixels are GRB; leds.c handles the byte order).
// These reproduce the colours the v1 firmware showed.
#pragma once

#define COLOR_NOTE        64,   0, 191      // key held, note
#define COLOR_TOGGLE     208,   0,  47      // toggle on
#define COLOR_MOMENTARY  222,  33,   0      // momentary held

#define COLOR_PROGMODE   240,   0,  15      // programming mode (red)
#define COLOR_SAVED        0, 240,  15      // settings saved (green)
#define COLOR_PROGBOOT   222,  33,   0      // jumping to bootloader

#define COLOR_PROG_NOTE      150,   0, 105  // key set to note while programming
#define COLOR_PROG_TOGGLE    195,   0,  60  // key set to CC toggle
#define COLOR_PROG_MOMENTARY   0,  96, 159  // key set to CC momentary

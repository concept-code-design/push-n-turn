// ======== NeoPixels (WS2812, GRB) ========
#include "firmware.h"
#include <WS2812.h>

// neopixel_show_P<port>_<bit>() is generated per pin by the WS2812 library.
#define NEO_SHOW_(port, bit) neopixel_show_P##port##_##bit
#define NEO_SHOW(port, bit) NEO_SHOW_(port, bit)
#define NEO_PIN (NEO_PORT * 10 + NEO_BIT)

#ifndef NEO_SET_PIXEL
#define NEO_SET_PIXEL set_pixel_for_GRB_LED
#endif

static __xdata uint8_t ledData[NEO_COUNT * 3];
static __xdata uint32_t flashUntil = 0;

void ledsInit(void) {
  pinMode(NEO_PIN, OUTPUT);
  digitalWrite(NEO_PIN, LOW);
  ledsClear();
}

void ledsShow(void) {
  NEO_SHOW(NEO_PORT, NEO_BIT)(ledData, NEO_COUNT * 3);
}

void ledSet(uint8_t pixel, uint8_t r, uint8_t g, uint8_t b) {
  if (pixel >= NEO_COUNT) return;
  NEO_SET_PIXEL(ledData, pixel, r, g, b);
}

void ledsFill(uint8_t r, uint8_t g, uint8_t b) {
  uint8_t i;
  for (i = 0; i < NEO_COUNT; i++) ledSet(i, r, g, b);
  ledsShow();
}

void ledsClear(void) {
  ledsFill(0, 0, 0);
}

// Colour wheel: 0..255 -> red -> green -> blue -> red.
void ledWheel(uint8_t pixel, uint8_t pos) {
  if (pos < 85) {
    ledSet(pixel, 255 - pos * 3, pos * 3, 0);
  } else if (pos < 170) {
    pos -= 85;
    ledSet(pixel, 0, 255 - pos * 3, pos * 3);
  } else {
    pos -= 170;
    ledSet(pixel, pos * 3, 0, 255 - pos * 3);
  }
}

// Show a colour on all pixels for a while; ledsUpdate() clears it again.
void ledsFlash(uint8_t r, uint8_t g, uint8_t b, uint16_t ms) {
  ledsFill(r, g, b);
  flashUntil = millis() + ms;
  if (flashUntil == 0) flashUntil = 1;
}

void ledsUpdate(void) {
  if (flashUntil && (int32_t)(millis() - flashUntil) >= 0) {
    flashUntil = 0;
    ledsClear();
  }
}

// ======== NeoPixel pin finder (build envs: push31_neotest, push4_neotest) ========
// Drives WS2812 data on each free CH552 pin in turn, 2 s per pin, all pixels dim
// white. When the LEDs light up, the active pin is the NeoPixel pin.
//
// The active pin is reported two ways:
//   - USB serial:  "NeoPixel test: P1.4"   (pio device monitor)
//   - USB MIDI:    Note On, channel 1, note number = pin (14 = P1.4)
// Hold any key to stay on the current pin. The boot key works as usual.
#include "firmware.h"

#ifdef NEOPIXEL_PIN_TEST
#include <WS2812.h>

#define TEST_PIXELS   8                 // more than fitted; extra data is harmless
#define DWELL_MS      2000

// Free pins of this device (NEO_TEST_PINS in the device header): no keys, encoder or
// USB D+/D- (P3.6/P3.7). P1.0, P1.2, P1.3 and P3.5 only exist on the CH552T package.
static __code uint8_t candidates[] = NEO_TEST_PINS;
#define NUM_CANDIDATES (sizeof(candidates))

static __code uint8_t keyPins[NUM_KEYS] = KEY_PINS;
static __xdata uint8_t pixels[TEST_PIXELS * 3];
static __xdata uint8_t current = 0;
static __xdata uint32_t switchedAt = 0;

static void show(uint8_t pin) {
  switch (pin) {
    case 10: neopixel_show_P1_0(pixels, sizeof(pixels)); break;
    case 12: neopixel_show_P1_2(pixels, sizeof(pixels)); break;
    case 13: neopixel_show_P1_3(pixels, sizeof(pixels)); break;
    case 14: neopixel_show_P1_4(pixels, sizeof(pixels)); break;
    case 15: neopixel_show_P1_5(pixels, sizeof(pixels)); break;
    case 30: neopixel_show_P3_0(pixels, sizeof(pixels)); break;
    case 31: neopixel_show_P3_1(pixels, sizeof(pixels)); break;
    case 32: neopixel_show_P3_2(pixels, sizeof(pixels)); break;
    case 33: neopixel_show_P3_3(pixels, sizeof(pixels)); break;
    case 34: neopixel_show_P3_4(pixels, sizeof(pixels)); break;
    case 35: neopixel_show_P3_5(pixels, sizeof(pixels)); break;
  }
}

static void fillPixels(uint8_t level) {
  uint8_t i;
  for (i = 0; i < sizeof(pixels); i++) pixels[i] = level;
}

static uint8_t anyKeyHeld(void) {
  uint8_t i;
  for (i = 0; i < NUM_KEYS; i++)
    if (!digitalRead(keyPins[i])) return 1;
  return 0;
}

static void selectPin(uint8_t idx) {
  uint8_t pin = candidates[current];

  // switch the old pin off and back to high-Z
  fillPixels(0);
  show(pin);
  pinMode(pin, INPUT);

  current = idx;
  pin = candidates[current];
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delayMicroseconds(100);              // WS2812 reset
  fillPixels(40);
  show(pin);
  switchedAt = millis();

  USBSerial_print_s("NeoPixel test: P");
  USBSerial_print_u(pin / 10);
  USBSerial_print_s(".");
  USBSerial_println_u(pin % 10);
  sendNoteOn(0, pin, 100);
  sendNoteOff(0, pin);
}

void setup(void) {
  uint8_t i;
  for (i = 0; i < NUM_KEYS; i++) pinMode(keyPins[i], INPUT_PULLUP);
  for (i = 0; i < NUM_CANDIDATES; i++) pinMode(candidates[i], INPUT);
  delay(5);
  if (!digitalRead(PROGBOOT_BTN)) enterBootloader();

  USBInit();
  selectPin(0);
}

void loop(void) {
  if (!anyKeyHeld() && (millis() - switchedAt) >= DWELL_MS) {
    selectPin((current + 1) % NUM_CANDIDATES);
  }
  processMidiMessage();
  USBSerial_flush();
}

#endif

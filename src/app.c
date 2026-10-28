// Push-n-Turn family firmware v2.0 (push31, push4) - Ronald Leenes, AGPL v3.0
// https://github.com/concept-code-design
//
// USB-MIDI controllers on CH552 using the mimuz-ch55x MIDI library.
// The device (push31: 3 keys + encoder, push4: 4 keys) is chosen at build time.
//
// v2: Windows support (IAD for the MIDI interfaces, see usb_descriptors.c),
// PlatformIO build, web config tool, EEPROM signature + factory defaults.

#include "firmware.h"

#ifndef USER_USB_RAM
#error "This firmware needs a USER USB RAM setting (see boards/ch552.json)"
#endif

__xdata uint8_t currentMode = PLAY_MODE;

#ifndef NEOPIXEL_PIN_TEST   // the pin finder (neopixel_pin_test.c) has its own setup/loop

void setup(void) {
  keysInit();
#if HAS_ENCODER
  encoderInit();
#endif
  ledsInit();
  settingsLoad();

  delay(5);                                   // let the pull-ups settle
  if (!digitalRead(PROGBOOT_BTN)) enterBootloader();

  USBInit();
  setHdlCtlChange(onCTLChange);
}

void loop(void) {
  keysUpdate();
#if HAS_ENCODER
  encoderUpdate();
#endif
  processMidiMessage();
  ledsUpdate();
  USBSerial_flush();
}

#endif

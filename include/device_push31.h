// ===================================================================================
// Push-n-Turn 31: 3 keys + rotary encoder with push button
// ===================================================================================
// Pin numbers are ch55xduino style: port * 10 + bit, e.g. 33 = P3.3.
#pragma once

#define INSTRUMENT_TYPE   31                // reported via config query CC102=44

// USB product string (UTF-16LE, one 0x00 after each character) and its length
#define USB_PRODUCT_CHARS 'P',0,'u',0,'s',0,'h',0,'&',0,'T',0,'u',0,'r',0,'n',0,'-',0,'3',0,'1',0
#define USB_PRODUCT_LEN   12

// Keys: sw1 P1.1, sw2 P1.7, sw3 P1.6, encoder push button P3.3 (key 4)
#define NUM_KEYS          4
#define KEY_PINS          {11, 17, 16, 33}
#define PROGBOOT_BTN      33                // hold while plugging in -> USB bootloader

// Rotary encoder on P3.1 / P3.0
#define HAS_ENCODER       1
#define ENCODER_A         31
#define ENCODER_B         30

// NeoPixels: one per key 1-3; the encoder and its push button have no LED
// (ENCODER_NEO >= NEO_COUNT, so those ledSet() calls are ignored)
#define NEO_COUNT         3
#define NEO_PORT          3                 // data on P3.4 (confirmed with push31_neotest)
#define NEO_BIT           4
#define ENCODER_NEO       NEO_COUNT

// Pins the NeoPixel pin finder may drive: everything except keys, encoder and USB
#define NEO_TEST_PINS     {10, 12, 13, 14, 15, 32, 34, 35}

// Factory defaults (used on a blank chip or after a factory reset)
#define DEFAULT_TYPES     {NOTE, NOTE, NOTE, CC_TOGGLE}
#define DEFAULT_DATA      {60, 62, 64, 20}
#define DEFAULT_ENC_CC    21
#define DEFAULT_ENC_STEPS 2

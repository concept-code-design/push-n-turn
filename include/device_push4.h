// ===================================================================================
// Push 4: 4 keys, 4 NeoPixels
// ===================================================================================
// Pin numbers are ch55xduino style: port * 10 + bit, e.g. 33 = P3.3.
#pragma once

#define INSTRUMENT_TYPE   4                 // reported via config query CC102=44

// USB product string (UTF-16LE, one 0x00 after each character) and its length
#define USB_PRODUCT_CHARS 'P',0,'u',0,'s',0,'h',0,'-',0,'4',0
#define USB_PRODUCT_LEN   6

// Keys: sw1 P1.1, sw2 P1.7, sw3 P1.6, sw4 P1.5
#define NUM_KEYS          4
#define KEY_PINS          {11, 17, 16, 15}
#define PROGBOOT_BTN      11                // hold key 1 while plugging in -> USB bootloader

#define HAS_ENCODER       0

// NeoPixels: one per key
#define NEO_COUNT         4
#define NEO_PORT          3                 // data on P3.4 (confirmed with push4_neotest)
#define NEO_BIT           4

// Pins the NeoPixel pin finder may drive: everything except keys and USB
#define NEO_TEST_PINS     {10, 12, 13, 14, 30, 31, 32, 33, 34, 35}

// Factory defaults (used on a blank chip or after a factory reset)
#define DEFAULT_TYPES     {NOTE, NOTE, NOTE, NOTE}
#define DEFAULT_DATA      {60, 62, 64, 65}

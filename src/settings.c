// ======== Persistent settings (CH552 data flash) ========
// Layout:
//   [0]       signature (EE_MAGIC); absent on a blank chip or v1 data
//   [1]       MIDI channel (0..15)
//   [2]       note velocity (1..127)
//   [3]       encoder step size (1..15)
//   [4]       encoder CC
//   [8..15]   key types
//   [16..23]  key data (note or CC number)
//
// Without the signature the factory defaults from the device header are loaded
// and stored once. v1 used another layout without a signature, so a v1 device
// starts with factory defaults after the update.

#include "firmware.h"

#define EE_MAGIC        0xB2   // change to force a one-time re-init after a layout change
#define EE_MAGIC_ADDR   0
#define EE_CHANNEL      1
#define EE_VELOCITY     2
#define EE_STEPS        3
#define EE_ENC_CC       4
#define EE_TYPES        8
#define EE_DATA         16

__xdata uint8_t keyType[NUM_KEYS];
__xdata uint8_t keyData[NUM_KEYS];
__xdata uint8_t midiChannel = 0;
__xdata uint8_t noteVelocity = 127;
#if HAS_ENCODER
__xdata uint8_t encCC = DEFAULT_ENC_CC;
__xdata uint8_t encStepSize = DEFAULT_ENC_STEPS;
#endif

static __code uint8_t defaultTypes[NUM_KEYS] = DEFAULT_TYPES;
static __code uint8_t defaultData[NUM_KEYS] = DEFAULT_DATA;

void settingsDefaults(void) {
  uint8_t i;
  for (i = 0; i < NUM_KEYS; i++) {
    keyType[i] = defaultTypes[i];
    keyData[i] = defaultData[i];
  }
  midiChannel = 0;                       // channel 1
  noteVelocity = 127;
#if HAS_ENCODER
  encCC = DEFAULT_ENC_CC;
  encStepSize = DEFAULT_ENC_STEPS;
#endif
}

void settingsSave(void) {
  uint8_t i;
  eeprom_write_byte(EE_MAGIC_ADDR, EE_MAGIC);
  eeprom_write_byte(EE_CHANNEL, midiChannel);
  eeprom_write_byte(EE_VELOCITY, noteVelocity);
#if HAS_ENCODER
  eeprom_write_byte(EE_STEPS, encStepSize);
  eeprom_write_byte(EE_ENC_CC, encCC);
#endif
  for (i = 0; i < NUM_KEYS; i++) {
    eeprom_write_byte(EE_TYPES + i, keyType[i]);
    eeprom_write_byte(EE_DATA + i, keyData[i]);
  }
}

void settingsLoad(void) {
  uint8_t i, v;
  settingsDefaults();
  if (eeprom_read_byte(EE_MAGIC_ADDR) != EE_MAGIC) {
    settingsSave();
    return;
  }
  // out-of-range fields keep their default
  v = eeprom_read_byte(EE_CHANNEL);   if (v <= 15) midiChannel = v;
  v = eeprom_read_byte(EE_VELOCITY);  if (v >= 1 && v <= 127) noteVelocity = v;
#if HAS_ENCODER
  v = eeprom_read_byte(EE_STEPS);     if (v >= 1 && v <= 15) encStepSize = v;
  v = eeprom_read_byte(EE_ENC_CC);    if (v <= 127) encCC = v;
#endif
  for (i = 0; i < NUM_KEYS; i++) {
    v = eeprom_read_byte(EE_TYPES + i); if (v >= NOTE && v <= CC_MOMENTARY) keyType[i] = v;
    v = eeprom_read_byte(EE_DATA + i);  if (v <= 127) keyData[i] = v;
  }
}

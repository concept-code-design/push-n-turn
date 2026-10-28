// ===================================================================================
// Project:   Push-n-Turn family (push31, push4)
// Version:   v2.0
// Year:      2025
// Author:    Ronald Leenes
// Github:    https://github.com/concept-code-design
// License:   GNU AGPL v3.0 license
// ===================================================================================
#pragma once

#include <Arduino.h>
#include "mimuz-ch55x.h"

// Key types. Also the tens digit of the config CC number (see README / prog protocol).
#define NOTE          1
#define CC_TOGGLE     2
#define CC_MOMENTARY  3
#define ENCODER_R     4
#define MIDICHANNEL   5
#define ENCODERSTEPS  6
#define NOTEVELOCITY  7

#if defined(DEVICE_PUSH31)
#include "device_push31.h"
#elif defined(DEVICE_PUSH4)
#include "device_push4.h"
#else
#error "No device selected: build with -DDEVICE_PUSH31 or -DDEVICE_PUSH4 (see platformio.ini)"
#endif

#define FW_VERSION    20            // v2.0

// ---- Config protocol: CCs on MIDI channel 11 (nibble 10, status 0xBA) ----
#define CFG_CHANNEL        10
#define CC_CMD             102
#define CMD_ENTER          42       // enter programming mode
#define CMD_SAVE           43       // save to EEPROM, leave programming mode
#define CMD_TYPE           44       // -> CC103 = INSTRUMENT_TYPE
#define CMD_FW             45       // -> CC104 = FW_VERSION
#define CMD_CONFIG         46       // -> full configuration dump, ends with CC102 = 46
#define CMD_STEPS          47       // -> CC105 = encoder step size
#define CMD_CHANNEL        48       // -> CC106 = MIDI channel (0..15)
#define CMD_BOOT           49       // reboot into USB bootloader
#define CMD_FACTORY        50       // restore factory defaults and save
#define CC_REPLY_TYPE      103
#define CC_REPLY_FW        104
#define CC_REPLY_STEPS     105
#define CC_REPLY_CHANNEL   106
#define CC_REPLY_VELOCITY  107

#define PLAY_MODE 0
#define PROG_MODE 1

// ---- settings.c ----
extern __xdata uint8_t keyType[NUM_KEYS];
extern __xdata uint8_t keyData[NUM_KEYS];
extern __xdata uint8_t midiChannel;     // 0..15
extern __xdata uint8_t noteVelocity;    // 1..127
#if HAS_ENCODER
extern __xdata uint8_t encCC;
extern __xdata uint8_t encStepSize;     // 1..15
#endif
void settingsLoad(void);
void settingsSave(void);
void settingsDefaults(void);

// ---- app.c ----
extern __xdata uint8_t currentMode;
void enterBootloader(void);

// ---- keys.c ----
void keysInit(void);
void keysUpdate(void);
void keysReset(void);

// ---- encoder.c ----
#if HAS_ENCODER
void encoderInit(void);
void encoderUpdate(void);
#endif

// ---- leds.c ----
void ledsInit(void);
void ledsShow(void);
void ledsClear(void);
void ledsFill(uint8_t r, uint8_t g, uint8_t b);
void ledSet(uint8_t pixel, uint8_t r, uint8_t g, uint8_t b);
void ledWheel(uint8_t pixel, uint8_t pos);
void ledsFlash(uint8_t r, uint8_t g, uint8_t b, uint16_t ms);
void ledsUpdate(void);

// ---- midi.c ----
void onCTLChange(uint8_t ch, uint8_t num, uint8_t val);

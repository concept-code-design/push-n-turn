// ======== Incoming MIDI: configuration protocol ========
// All config traffic is CC on MIDI channel 11 (nibble 10, status byte 0xBA).
//
//   CC102 = 42..50            command (see firmware.h)
//   CC (type*10 + key) = v    while programming: key 1..NUM_KEYS becomes
//                             type 1 note / 2 CC toggle / 3 CC momentary, number v
//   CC 40..49 = v             encoder CC number                 (push31)
//   CC 50..59 = v             MIDI channel, 0..15 (= channel 1..16)
//   CC 60..69 = v             encoder step size, 1..15          (push31)
//   CC 70..79 = v             note velocity, 1..127
//
// The config dump (CC102=46) uses the same numbers, then CC105 (steps, push31),
// CC106 (channel), CC107 (velocity) and finally CC102=46 as end marker.

#include "firmware.h"
#include "colors.h"

static void sendCfg(uint8_t num, uint8_t val) {
  sendCtlChange(CFG_CHANNEL, num, val);
}

static void showKeyType(uint8_t i) {
  switch (keyType[i]) {
    case NOTE:         ledSet(i, COLOR_PROG_NOTE); break;
    case CC_TOGGLE:    ledSet(i, COLOR_PROG_TOGGLE); break;
    case CC_MOMENTARY: ledSet(i, COLOR_PROG_MOMENTARY); break;
  }
}

static void dumpConfig(void) {
  uint8_t i;
  for (i = 0; i < NUM_KEYS; i++) sendCfg(keyType[i] * 10 + i + 1, keyData[i]);
#if HAS_ENCODER
  sendCfg(ENCODER_R * 10 + NUM_KEYS + 1, encCC);
  sendCfg(CC_REPLY_STEPS, encStepSize);
#endif
  sendCfg(CC_REPLY_CHANNEL, midiChannel);
  sendCfg(CC_REPLY_VELOCITY, noteVelocity);
  sendCfg(CC_CMD, CMD_CONFIG);                 // end of dump
}

static void handleCommand(uint8_t cmd) {
  uint8_t i;
  switch (cmd) {
    case CMD_ENTER:
      currentMode = PROG_MODE;
      keysReset();
      ledsFill(COLOR_PROGMODE);
      for (i = 0; i < NUM_KEYS; i++) showKeyType(i);
      ledsShow();
      sendCfg(CC_CMD, CMD_ENTER);              // ack
      break;
    case CMD_SAVE:
      settingsSave();
      currentMode = PLAY_MODE;
      keysReset();
      ledsFlash(COLOR_SAVED, 1000);
      sendCfg(CC_CMD, CMD_SAVE);               // ack
      break;
    case CMD_TYPE:    sendCfg(CC_REPLY_TYPE, INSTRUMENT_TYPE); break;
    case CMD_FW:      sendCfg(CC_REPLY_FW, FW_VERSION); break;
    case CMD_CONFIG:  dumpConfig(); break;
#if HAS_ENCODER
    case CMD_STEPS:   sendCfg(CC_REPLY_STEPS, encStepSize); break;
#endif
    case CMD_CHANNEL: sendCfg(CC_REPLY_CHANNEL, midiChannel); break;
    case CMD_BOOT:    enterBootloader(); break;
    case CMD_FACTORY:
      settingsDefaults();
      settingsSave();
      currentMode = PLAY_MODE;
      keysReset();
      ledsFlash(COLOR_SAVED, 1000);
      sendCfg(CC_CMD, CMD_FACTORY);            // ack
      break;
  }
}

// Registered with the mimuz library. The library calls handlers through a
// __reentrant pointer, so the arguments are not passed reliably to this
// (non-reentrant) function: read them back through the getters instead.
void onCTLChange(uint8_t ch, uint8_t num, uint8_t val) {
  __xdata uint8_t type, key;
  ch = getChannel();
  num = getNote();
  val = getVel();

  if (ch != CFG_CHANNEL) return;
  if (num == CC_CMD) {
    handleCommand(val);
    return;
  }
  if (currentMode != PROG_MODE) return;

  type = num / 10;
  key = num % 10;
  switch (type) {
    case NOTE:
    case CC_TOGGLE:
    case CC_MOMENTARY:
      if (key < 1 || key > NUM_KEYS) return;
      keyType[key - 1] = type;
      keyData[key - 1] = val;
      showKeyType(key - 1);
      ledsShow();
      break;
#if HAS_ENCODER
    case ENCODER_R:
      encCC = val;
      break;
    case ENCODERSTEPS:
      if (val >= 1 && val <= 15) encStepSize = val;
      break;
#endif
    case MIDICHANNEL:
      if (val <= 15) midiChannel = val;
      break;
    case NOTEVELOCITY:
      if (val >= 1) noteVelocity = val;
      break;
  }
}

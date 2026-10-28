// ======== Keys: debounce + MIDI ========
#include "firmware.h"
#include "colors.h"

#define DEBOUNCE_MS 5

static __code uint8_t keyPins[NUM_KEYS] = KEY_PINS;

static __xdata uint8_t lastReading[NUM_KEYS];
static __xdata uint8_t keyState[NUM_KEYS];
static __xdata uint8_t toggleState[NUM_KEYS];
static __xdata uint32_t lastChange[NUM_KEYS];

void keysInit(void) {
  uint8_t i;
  for (i = 0; i < NUM_KEYS; i++) pinMode(keyPins[i], INPUT_PULLUP);
  keysReset();
}

// Forget held/toggled state (after programming the key types may have changed).
void keysReset(void) {
  uint8_t i;
  for (i = 0; i < NUM_KEYS; i++) {
    lastReading[i] = HIGH;
    keyState[i] = HIGH;
    toggleState[i] = 0;
    lastChange[i] = 0;
  }
}

static void keyPressed(uint8_t i) {
  switch (keyType[i]) {
    case NOTE:
      sendNoteOn(midiChannel, keyData[i], noteVelocity);
      ledSet(i, COLOR_NOTE);
      break;
    case CC_TOGGLE:
      toggleState[i] = !toggleState[i];
      sendCtlChange(midiChannel, keyData[i], toggleState[i] ? 127 : 0);
      if (toggleState[i]) ledSet(i, COLOR_TOGGLE); else ledSet(i, 0, 0, 0);
      break;
    case CC_MOMENTARY:
      sendCtlChange(midiChannel, keyData[i], 127);
      ledSet(i, COLOR_MOMENTARY);
      break;
  }
  ledsShow();
}

static void keyReleased(uint8_t i) {
  switch (keyType[i]) {
    case NOTE:
      sendNoteOff(midiChannel, keyData[i]);
      break;
    case CC_MOMENTARY:
      sendCtlChange(midiChannel, keyData[i], 0);
      break;
    default:
      return;                     // toggle: nothing on release
  }
  ledSet(i, 0, 0, 0);
  ledsShow();
}

void keysUpdate(void) {
  uint8_t i, reading;
  uint32_t now = millis();
  for (i = 0; i < NUM_KEYS; i++) {
    reading = digitalRead(keyPins[i]);
    if (reading != lastReading[i]) {
      lastReading[i] = reading;
      lastChange[i] = now;
    } else if (reading != keyState[i] && (now - lastChange[i]) > DEBOUNCE_MS) {
      keyState[i] = reading;
      if (currentMode != PLAY_MODE) continue;   // keys are silent while programming
      if (reading == LOW) keyPressed(i); else keyReleased(i);
    }
  }
}

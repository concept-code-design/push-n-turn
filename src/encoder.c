// ======== Rotary encoder (push31) ========
// Quadrature state machine from v1: a detent counts once the encoder returns to
// the rest state (3) after passing through at least 3 of the 4 states in one
// direction (one may be missed if we did not sample fast enough).
#include "firmware.h"

#if HAS_ENCODER

static __code int8_t transition[4][4] = {{0, -1, 1, 2},
                                         {1, 0, 2, -1},
                                         {-1, 2, 0, 1},
                                         {2, 1, -1, 0}};

static __xdata uint8_t encPrev;
static __xdata uint8_t cwSeen, ccwSeen;
static __xdata uint8_t encValue = 0;

static uint8_t encRead(void) {
  return (digitalRead(ENCODER_A) << 1) | digitalRead(ENCODER_B);
}

static uint8_t isDetent(uint8_t seen) {
  return seen == 0x0B || seen == 0x0D || seen == 0x0E || seen == 0x0F;
}

static void encStep(int8_t dir) {
  int16_t v = (int16_t)encValue + dir * (int16_t)encStepSize;
  if (v < 0) v = 0;
  if (v > 127) v = 127;
  if ((uint8_t)v == encValue) return;
  encValue = (uint8_t)v;
  sendCtlChange(midiChannel, encCC, encValue);
  ledWheel(ENCODER_NEO, encValue * 2);
  ledsShow();
}

void encoderInit(void) {
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);
  encPrev = encRead();
}

void encoderUpdate(void) {
  uint8_t now = encRead();
  int8_t info;
  if (now == encPrev) return;

  info = transition[encPrev][now];
  if (info == 1) cwSeen |= (1 << now);
  else if (info == -1) ccwSeen |= (1 << now);

  if (now == 3) {
    if (currentMode == PLAY_MODE) {
      if (isDetent(cwSeen)) encStep(1);
      else if (isDetent(ccwSeen)) encStep(-1);
    }
    cwSeen = 0;
    ccwSeen = 0;
  }
  encPrev = now;
}

#endif

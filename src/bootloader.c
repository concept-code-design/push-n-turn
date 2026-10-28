// Firmware update: hand control to the CH552's built-in USB bootloader
// (same as v1 PROGBOOT_now). Flash with `pio run -e <env> -t upload`.
#include "firmware.h"
#include "colors.h"

void enterBootloader(void) {
  ledsFill(COLOR_PROGBOOT);
  USB_CTRL = 0;
  EA = 0;
  TMOD = 0;
  __asm lcall #BOOT_LOAD_ADDR __endasm;
}

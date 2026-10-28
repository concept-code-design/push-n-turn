# Push-n-Turn firmware v2 (push31, push4)

Firmware for the Concept & Code CH552 USB-MIDI controllers. Both devices use one
code base; the device is chosen at build time:

| env             | device                                         | INSTRUMENT_TYPE |
|-----------------|------------------------------------------------|-----------------|
| `push31`        | 3 keys + rotary encoder with push button, 3 LEDs | 31            |
| `push4`         | 4 keys, 4 LEDs                                 | 4               |
| `push31_neotest`, `push4_neotest` | NeoPixel pin finder (test only) | – |

Pins and factory defaults live in [include/device_push31.h](include/device_push31.h)
and [include/device_push4.h](include/device_push4.h).

The devices enumerate as a composite **USB-MIDI + CDC serial** device (VID/PID 1209:CAC1)
and work on **macOS and Windows** (and Linux) over USB-C without drivers. v1 only
showed up as MIDI on macOS; see the comment at the top of
[src/usb_descriptors.c](src/usb_descriptors.c) for the fix.

## How the devices work

Plug the device into a computer with a USB-C cable. It needs no driver and shows up as a
MIDI port (*Push&Turn-31* or *Push-4*) plus a serial port that is only used for firmware
updates. Pick that MIDI port as input in your DAW or MIDI software.

### Controls

| | push31 | push4 |
|---|---|---|
| keys | key 1–3 (P1.1, P1.7, P1.6) | key 1–4 (P1.1, P1.7, P1.6, P1.5) |
| key 4 | the encoder's push button (P3.3) | – |
| encoder | turn: CC (P3.1 / P3.0) | – |
| LEDs | one per key 1–3 (the encoder has none) | one per key |

Every key behaves in one of three ways, chosen per key in the configuration:

| key type | press | release | LED |
|---|---|---|---|
| **Note** | Note On, number *n*, the set velocity | Note Off | violet while held |
| **CC toggle** | CC *n* = 127, next press CC *n* = 0 | nothing | crimson while on |
| **CC momentary** | CC *n* = 127 | CC *n* = 0 | orange while held |

Toggle keys always start in the *off* state after power-up or a configuration change.

The **encoder** (push31) holds a value from 0 to 127. Each click (detent) raises it
clockwise or lowers it counter-clockwise by the step size and sends the new value as
CC *encoder CC*. It stops at 0 and 127 and starts at 0 after power-up.

All messages go out on one MIDI channel (factory setting: channel 1).

### Factory settings

| | push31 | push4 |
|---|---|---|
| key 1, 2, 3 | Note 60, 62, 64 (C4, D4, E4) | Note 60, 62, 64 |
| key 4 | encoder button: CC toggle 20 | Note 65 (F4) |
| encoder | CC 21, step size 2 | – |
| channel / velocity | 1 / 127 | 1 / 127 |

Settings are kept in the chip's data flash, so they survive unplugging. A factory reset
(from the config tool) restores this table.

### Boot key

Holding the **boot key** while plugging the device in starts the USB bootloader instead of
the MIDI firmware (LEDs orange; the MIDI port does not appear). This is only needed when a
firmware upload can't reset the device by itself; see *Build & flash* below. The boot key is
the encoder button on a push31 and key 1 on a push4.

## Build & flash (PlatformIO)

```bash
pio run                              # build push31 + push4
pio run -e push31 -t upload          # build + flash a Push&Turn 31
pio run -e push4  -t upload          # build + flash a Push 4
```

- The first build downloads the ch55xduino 0.0.26 core, its SDCC build and the
  `vnproch55x` uploader for your OS (~30 MB, cached in `~/.platformio/packages/ch55xduino-0.0.26`).
  This is done by the local platform in [platform-ch55x/](platform-ch55x/); the stock
  `intel_mcs51` platform can't be used because ch55xduino needs its own SDCC runtime library.
- **Upload** resets a running device into the bootloader by itself (1200-baud touch on its
  serial port). A blank or unresponsive device: hold the boot key while plugging it in,
  then upload — the **encoder button (P3.3)** on a push31, **key 1 (P1.1)** on a push4.
- On **Windows** the bootloader may need the WCH/libusb driver once (install it with
  [Zadig](https://zadig.akeo.ie/) for the "USB Module" device that appears in bootloader mode).
  The MIDI firmware itself needs no driver.
- macOS on Apple silicon runs the x86_64 SDCC/uploader through Rosetta.

## Finding the NeoPixel pin

```bash
pio run -e push31_neotest -t upload      # or push4_neotest
pio device monitor -e push31_neotest
```

The test drives NeoPixel data on every free pin of the device in turn (2 s each, LEDs dim
white) and reports the active pin on the serial monitor (`NeoPixel test: P1.4`) and as a
MIDI note on channel 1 (note 14 = P1.4). Hold any key to stay on the current pin. Then set
`NEO_PORT` / `NEO_BIT` in the device header.

| device | pins tried | result |
|---|---|---|
| push4  | P1.0, P1.2–P1.4, P3.0–P3.5 | **P3.4** (set) |
| push31 | P1.0, P1.2–P1.5, P3.2, P3.4, P3.5 | **P3.4** (set) |

### macOS notes

- Apple-silicon Macs may ask **"Allow accessory to connect?"** when the device (or its
  bootloader) is plugged in. Until you allow it, neither MIDI nor the serial port appears and
  uploads fail with `libusb_bulk_transfer error code -1`.
- All Concept & Code devices share USB ID 1209:CAC1, so CoreMIDI may show a push under the name
  of another device it saw before (e.g. "Behringer-02"). Delete the stale device in
  *Audio MIDI Setup › MIDI Studio* and re-plug to get the right name. The config tool
  identifies the device by its type reply, not by the port name.

## Configuration

Settings are changed over MIDI, so no firmware update is needed. Every setting can be changed:

- per key: type (*Note*, *CC toggle*, *CC momentary*) and its note or CC number (0–127)
- push31: the encoder's CC number and step size (1–15 per click)
- the MIDI channel (1–16) and the velocity of Note On messages (1–127)

### With the web tool

1. Open [config-tool/PushNTurn_Config.html](config-tool/PushNTurn_Config.html) in **Chrome or
   Edge** (Safari has no Web MIDI). The file runs locally; nothing is uploaded.
   Allow MIDI access when the browser asks.
2. Plug in the device. The tool selects its MIDI input and output by itself. Pick them by
   hand if several devices are connected, or if macOS shows the port under another name
   (see *macOS notes*).
3. Click **Read from device**. The tool asks the device what it is (push31 or push4), shows
   the matching layout and fills in the current settings.
4. Change the settings. For notes the tool shows the note name next to the number
   (60 = C4).
5. Click **Write & save to device**. The tool puts the device in programming mode (keys
   silent, each LED shows its key's type: purple = note, pink = CC toggle, blue = CC
   momentary), sends the settings, saves them (LEDs flash green for a second) and reads them
   back to check them.

Other buttons:

- **Reset fields to defaults** only resets the form; click *Write & save* to store it.
- **Factory reset device** restores and saves the factory settings in the device.
- **Reboot into bootloader** starts the USB bootloader for a firmware update; the MIDI
  device disappears until it is flashed or plugged in again.

### Without the web tool

The tool uses a small protocol, described below, which any MIDI program that can send
Control Change messages can also use.

### Protocol

All config traffic is Control Change on **MIDI channel 11** (status byte `0xBA` = 186), so it
also works from `midiutil.py -w 186 <cc> <value>`.

| host → device | meaning |
|---|---|
| `CC102 = 42` | enter programming mode (keys and encoder silent, LEDs show the key types); ack `CC102 = 42` |
| `CC (type·10 + key) = n` | key 1–4 becomes type 1 Note, 2 CC toggle, 3 CC momentary, with note/CC number `n` |
| `CC 40..49 = n` | encoder CC number (push31) |
| `CC 50..59 = c` | MIDI channel `c` = 0–15 (channel 1–16) |
| `CC 60..69 = s` | encoder step size 1–15 (push31) |
| `CC 70..79 = v` | note velocity 1–127 |
| `CC102 = 43` | save and leave programming mode (LEDs flash green); ack `CC102 = 43` |
| `CC102 = 44` | → `CC103` = device type (31 / 4) |
| `CC102 = 45` | → `CC104` = firmware version (20 = v2.0) |
| `CC102 = 46` | → one CC per key as above, `CC45` encoder CC (push31), `CC105` steps (push31), `CC106` channel, `CC107` velocity, then `CC102 = 46` |
| `CC102 = 47` | → `CC105` = encoder step size |
| `CC102 = 48` | → `CC106` = MIDI channel |
| `CC102 = 49` | reboot into the USB bootloader |
| `CC102 = 50` | restore factory defaults and save; ack `CC102 = 50` |

Settings are only accepted between `42` and `43`, and only take effect for good with `43`
(unplugging before that discards them). Example (push31: key 1 = CC toggle 20,
encoder on CC 11, channel 16, 5 steps per detent):

```
./midiutil.py -d 1 -w 186 102 42
./midiutil.py -d 1 -w 186 21 20
./midiutil.py -d 1 -w 186 45 11
./midiutil.py -d 1 -w 186 50 15
./midiutil.py -d 1 -w 186 66 5
./midiutil.py -d 1 -w 186 102 43
```

## Changes from v1

- PlatformIO project; old Arduino sketch in [legacy-arduino/](legacy-arduino/). The
  unrelated macropad/HID sources that were being compiled into the v1 image are in
  `legacy-arduino/macropad-hid-src/`.
- Windows: second IAD for the MIDI interfaces, bcdDevice 2.00; the bundled mimuz-ch55x
  library STALLs unknown string descriptors and answers GET_INTERFACE.
- Momentary keys send 127 on press and 0 on release (v1 sent 127+0 on press).
- Config fixes: queries no longer overwrite key 2 while programming, MIDI channel 1 can be
  set, the encoder clamps at 0 instead of wrapping, settings are only accepted on channel 11.
- EEPROM signature: blank chips (and v1 devices, whose layout changed) start with factory
  defaults instead of channel 255 — re-apply your settings after updating from v1.

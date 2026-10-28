# PlatformIO builder for CH55x + ch55xduino.
# Mirrors ch55xduino 0.0.26 platform.txt (compile, archive, link, upload).

import os
import shutil
import sys
import time

from SCons.Script import AlwaysBuild, Default, DefaultEnvironment

env = DefaultEnvironment()
board = env.BoardConfig()
builder_dir = os.path.join(env.PioPlatform().get_dir(), "builder")
sys.path.insert(0, builder_dir)

import ch55x_packages  # noqa: E402

pkgs = ch55x_packages.ensure_all(env.subst("$PROJECT_PACKAGES_DIR"))
env["CH55X_PKGS"] = pkgs

exe = ch55x_packages.EXE
sdcc = os.path.join(pkgs["sdcc_bin"], "sdcc" + exe)
sdar = os.path.join(pkgs["sdcc_bin"], "sdar" + exe)

env.Replace(
    CC=sdcc,
    AR=sdar,
    LINK=sdcc,
    OBJSUFFIX=".rel",
    LIBPREFIX="",
    LIBSUFFIX=".lib",
    PROGNAME="firmware",
    PROGSUFFIX=".ihx",
    CCCOM='"$PYTHONEXE" "%s" "$CC" -c $CFLAGS $CCFLAGS $_CCCOMCOM $SOURCES -o $TARGET'
    % os.path.join(builder_dir, "sdcc_wrap.py"),
    ARCOM='"$AR" rcs $TARGET $SOURCES',
    RANLIBCOM="",
    # CH55X_LIBS (core + bundled libraries + SDCC runtime) go after the objects
    LINKCOM='"$LINK" $LINKFLAGS $SOURCES $CH55X_LIBS $_LIBDIRFLAGS -o $TARGET',
    CH55X_LIBS=[],
    CH55X_LIB_NODES=[],
)

# BuildProgram() runs builder/frameworks/ch55xduino.py (declared in platform.json)
target_ihx = env.BuildProgram()
env.Depends(target_ihx, env["CH55X_LIB_NODES"])


def _mem_usage(mem_path):
    """Parse SDCC's .mem summary -> (flash_used, xram_used)."""
    flash = xram = None
    with open(mem_path) as f:
        for line in f:
            line = line.strip()
            parts = line.split()
            if line.startswith("ROM/EPROM/FLASH") and len(parts) >= 4:
                flash = int(parts[3])
            elif line.startswith("EXTERNAL RAM") and len(parts) >= 5:
                xram = int(parts[4])
    return flash, xram


def make_hex(target, source, env):
    ihx = str(source[0])
    shutil.copyfile(ihx, str(target[0]))  # SDCC's .ihx already is Intel HEX
    mem = os.path.splitext(ihx)[0] + ".mem"
    if os.path.isfile(mem):
        flash, xram = _mem_usage(mem)
        max_flash = int(board.get("upload.maximum_size"))
        max_xram = int(board.get("upload.maximum_ram_size"))
        if flash is not None:
            print("Flash: %5d / %d bytes (%.1f%%)" % (flash, max_flash, 100.0 * flash / max_flash))
        if xram is not None:
            print("XRAM:  %5d / %d bytes (%.1f%%)" % (xram, max_xram, 100.0 * xram / max_xram))
        if flash is not None and flash > max_flash:
            sys.stderr.write("Error: firmware does not fit in flash\n")
            return 1
    return None


target_hex = env.Command(
    os.path.join("$BUILD_DIR", "${PROGNAME}.hex"),
    target_ihx,
    env.VerboseAction(make_hex, "Building $TARGET"),
)
AlwaysBuild(target_hex)


def enter_bootloader(target, source, env):
    """1200-baud touch on the device's CDC port -> firmware jumps to the bootloader.

    Skipped when no running device is found (e.g. it is already in bootloader
    mode because the boot key was held while plugging it in)."""
    vid_pid = board.get("build.hwids", [["0x1209", "0xCAC1"]])[0]
    vid, pid = int(vid_pid[0], 16), int(vid_pid[1], 16)
    port = env.subst("$UPLOAD_PORT") or None
    try:
        import serial
        from serial.tools import list_ports
    except ImportError:
        return None
    if not port:
        for p in list_ports.comports():
            if p.vid == vid and p.pid == pid:
                port = p.device
                break
    if not port:
        print("No running device found on USB serial; assuming it is already in bootloader mode.")
        return None
    print("Resetting %s into the bootloader (1200 baud touch)" % port)
    try:
        s = serial.Serial(port, 1200)
        s.dtr = False
        s.close()
    except Exception as e:  # port vanishing mid-close is expected
        print("  (%s)" % e)
    time.sleep(1.5)
    return None


env.Replace(
    UPLOADER=pkgs["uploader"],
    UPLOADCMD='"$UPLOADER" -r 2 -t %s -c %s "$SOURCE"'
    % (board.get("build.mcu"), board.get("upload.bootcfg", 3)),
)

env.AddPlatformTarget(
    "upload",
    target_hex,
    [
        env.VerboseAction(enter_bootloader, "Looking for device"),
        env.VerboseAction("$UPLOADCMD", "Uploading $SOURCE"),
    ],
    "Upload",
)

Default([target_hex])

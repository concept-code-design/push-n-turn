# ch55xduino framework: compile flags, include paths, core + WS2812 libraries.
# Flags follow ch55xduino 0.0.26 platform.txt / boards.txt ("USER CODE w/ <n>B USB ram").

import os

Import("env")  # noqa: F821

board = env.BoardConfig()
pkgs = env["CH55X_PKGS"]
core_root = pkgs["core_dir"]
core_dir = os.path.join(core_root, "cores", "ch55xduino")
variant_dir = os.path.join(core_root, "variants", board.get("build.variant"))
ws2812_dir = os.path.join(core_root, "libraries", "WS2812", "src")

usb_ram = int(board.get("build.usb_ram"))

env.Append(
    CPPDEFINES=[
        ("double", "float"),
        "USE_STDINT",
        "__PROG_TYPES_COMPAT__",
        board.get("build.mcu"),
        ("F_CPU", board.get("build.f_cpu")),
        ("F_EXT_OSC", board.get("build.f_oscillator_external", "0L")),
        ("ARDUINO", 10819),
        "ARDUINO_ch55x",
        "ARDUINO_ARCH_mcs51",
        ("USER_USB_RAM", usb_ram),
    ],
    CCFLAGS=["-mmcs51", "--model-large", "--int-long-reent", "--less-pedantic"],
    CPPPATH=[core_dir, variant_dir, ws2812_dir, pkgs["sdcc_include"]],
    LINKFLAGS=[
        "--nostdlib",
        "-mmcs51",
        "--code-size", str(board.get("upload.maximum_size")),
        "--xram-size", str(board.get("upload.maximum_ram_size")),
        "--xram-loc", str(usb_ram),
        "--out-fmt-ihx",
    ],
    LIBPATH=[pkgs["sdcc_lib"]],
)

# The core and WS2812 are linked as libraries so SDCC only pulls in the modules
# that are referenced (there is no section GC on 8051). main.c is linked
# directly because nothing references it.
core_lib = env.BuildLibrary(
    os.path.join("$BUILD_DIR", "FrameworkCore"), core_dir, src_filter="+<*.c> -<main.c>"
)
ws2812_lib = env.BuildLibrary(os.path.join("$BUILD_DIR", "FrameworkWS2812"), ws2812_dir)
core_main = env.Object(
    os.path.join("$BUILD_DIR", "FrameworkCoreMain", "main.rel"), os.path.join(core_dir, "main.c")
)

env.Replace(
    CH55X_LIB_NODES=[core_main, core_lib, ws2812_lib],
    CH55X_LIBS=[core_main, core_lib, ws2812_lib]
    + ["-lmcs51", "-llibsdcc", "-lliblong", "-lliblonglong", "-llibint", "-llibfloat"],
)

# Runs one SDCC compile, then applies the same .rel fix-ups as ch55xduino's
# wrapper/sdcc.sh: CSEG and GSINIT sizes are rounded up to an even number so
# every module starts on an even code address (the CH55x fetches 16-bit words;
# odd alignment changes cycle counts, which breaks the WS2812 bit-banging), and
# GSFINAL 3 -> 4 for the same reason.
#
# usage: python sdcc_wrap.py <sdcc> <args...> -o <file.rel>

import re
import subprocess
import sys


def _round_even(match):
    size = int(match.group(2), 16)
    if size % 2:
        size += 1
    return "%s%X" % (match.group(1), size)


def patch_rel(path):
    with open(path, "r", newline="") as f:
        text = f.read()
    fixed = re.sub(r"^(A CSEG size )([0-9A-Fa-f]+)", _round_even, text, flags=re.M)
    fixed = re.sub(r"^(A GSINIT size )([0-9A-Fa-f]+)", _round_even, fixed, flags=re.M)
    fixed = re.sub(r"^A GSFINAL size 3\b", "A GSFINAL size 4", fixed, flags=re.M)
    if fixed != text:
        with open(path, "w", newline="") as f:
            f.write(fixed)


def main(argv):
    rc = subprocess.call(argv)
    if rc != 0:
        return rc
    if "-o" in argv:
        out = argv[argv.index("-o") + 1]
        if out.endswith(".rel"):
            patch_rel(out)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

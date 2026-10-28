# Downloads and locates the ch55xduino toolchain: core, patched SDCC and the
# vnproch55x uploader. These are the exact archives the Arduino Boards Manager
# installs for ch55xduino 0.0.26, cached once under the PlatformIO packages dir.
#
# The ch55xduino SDCC build is required (stock SDCC lacks the
# large_int_calc_stack_auto runtime library the core is compiled against).

import os
import shutil
import sys
import tarfile
import tempfile
import platform as host_platform

CORE_VERSION = "0.0.26"
SDCC_VERSION = "13407_4"
TOOLS_VERSION = "2026.07.10"

_REL = "https://github.com/DeqingSun/ch55xduino/releases/download/"

CORE_URL = _REL + "0.0.26/ch55xduino-core-0.0.26.tar.bz2"

SDCC_URLS = {
    "darwin": _REL + "0.0.20/sdcc-mcs51-x86_64-apple-macosx-20220422-13407_4.tar.bz2",
    "windows": _REL + "0.0.20/sdcc-mcs51-i586-mingw32msvc-20220422-13407_4.tar.bz2",
    "linux_x86_64": _REL + "0.0.20/sdcc-mcs51-amd64-ubuntu20.04-linux2.5-20220422-13407_4.tar.bz2",
    "linux_aarch64": _REL + "0.0.20/sdcc-mcs51-aarch64-linux-gnu-20230423-13407_4.tar.bz2",
}

TOOLS_URLS = {
    "darwin": _REL + "0.0.26/ch55xduino-tools_macosx-2026.07.10.tar.bz2",
    "windows": _REL + "0.0.26/ch55xduino-tools_mingw32-2026.07.10.tar.bz2",
    "linux_x86_64": _REL + "0.0.26/ch55xduino-tools_linux64-2026.07.10.tar.bz2",
    "linux_aarch64": _REL + "0.0.26/ch55xduino-tools_aarch64-2026.07.10.tar.bz2",
}

EXE = ".exe" if sys.platform.startswith("win") else ""


def host_key():
    if sys.platform.startswith("win"):
        return "windows"
    if sys.platform == "darwin":
        return "darwin"  # x86_64 binaries; run under Rosetta on Apple silicon
    if host_platform.machine().lower() in ("aarch64", "arm64"):
        return "linux_aarch64"
    return "linux_x86_64"


def _download(url, dest_file):
    print("ch55x: downloading %s" % url)
    try:
        import requests

        with requests.get(url, stream=True, timeout=60) as r:
            r.raise_for_status()
            with open(dest_file, "wb") as f:
                for chunk in r.iter_content(chunk_size=1 << 16):
                    f.write(chunk)
        return
    except ImportError:
        pass
    import ssl
    import urllib.request

    ctx = None
    try:
        import certifi

        ctx = ssl.create_default_context(cafile=certifi.where())
    except ImportError:
        pass
    with urllib.request.urlopen(url, context=ctx, timeout=60) as r, open(dest_file, "wb") as f:
        shutil.copyfileobj(r, f)


def _install(url, dest):
    """Download a .tar.bz2 and unpack it into dest (atomically)."""
    parent = os.path.dirname(dest)
    os.makedirs(parent, exist_ok=True)
    tmp = tempfile.mkdtemp(prefix=".ch55x-", dir=parent)
    try:
        archive = os.path.join(tmp, "pkg.tar.bz2")
        _download(url, archive)
        unpack = os.path.join(tmp, "unpack")
        with tarfile.open(archive, "r:bz2") as tf:
            tf.extractall(unpack)
        if os.path.isdir(dest):
            shutil.rmtree(dest)
        os.replace(unpack, dest)
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


def _find(root, predicate):
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames.sort()
        hit = predicate(dirpath, dirnames, filenames)
        if hit:
            return hit
    return None


def _find_core(root):
    return _find(root, lambda d, dn, fn: d if os.path.isdir(os.path.join(d, "cores", "ch55xduino")) else None)


def _find_sdcc(root):
    def pred(d, dn, fn):
        if os.path.basename(d) == "bin" and ("sdcc" + EXE) in fn:
            return os.path.dirname(d)
        return None

    return _find(root, pred)


def _find_uploader(root):
    def pred(d, dn, fn):
        if ("vnproch55x" + EXE) in fn:
            return os.path.join(d, "vnproch55x" + EXE)
        return None

    return _find(root, pred)


def _ensure(dest, url, finder):
    found = finder(dest) if os.path.isdir(dest) else None
    if not found:
        _install(url, dest)
        found = finder(dest)
    if not found:
        sys.stderr.write("ch55x: unexpected archive layout in %s\n" % dest)
        sys.exit(1)
    return found


def ensure_all(packages_dir):
    """Return dict with core_dir, sdcc_dir, sdcc_bin, sdcc_include, sdcc_lib, uploader."""
    key = host_key()
    base = os.path.join(packages_dir, "ch55xduino-" + CORE_VERSION)

    core_dir = _ensure(os.path.join(base, "core"), CORE_URL, _find_core)
    sdcc_dir = _ensure(os.path.join(base, "sdcc-" + SDCC_VERSION + "-" + key), SDCC_URLS[key], _find_sdcc)
    uploader = _ensure(os.path.join(base, "tools-" + TOOLS_VERSION + "-" + key), TOOLS_URLS[key], _find_uploader)

    # macOS/Linux archives use share/sdcc/{include,lib}; Windows has them at the root.
    lib_dir = _find(sdcc_dir, lambda d, dn, fn: d if os.path.basename(d) == "large_int_calc_stack_auto" else None)
    if not lib_dir:
        sys.stderr.write("ch55x: SDCC runtime library large_int_calc_stack_auto not found in %s\n" % sdcc_dir)
        sys.exit(1)
    std_root = os.path.dirname(os.path.dirname(lib_dir))

    return {
        "core_dir": core_dir,
        "sdcc_dir": sdcc_dir,
        "sdcc_bin": os.path.join(sdcc_dir, "bin"),
        "sdcc_include": os.path.join(std_root, "include"),
        "sdcc_lib": lib_dir,
        "uploader": uploader,
    }

#!/usr/bin/env python3
"""List the weak imports of an app bundle that this macOS does not provide.

    python3 -I check_weak_imports.py "Astral Italian.app"

A binary built with a newer SDK than its deployment target links APIs newer than the target weakly: on an older
macOS the symbol is simply null, and calling it without an availability check jumps to address 0 (this is how
libusb's pipe2 crashed the first 15.0 build on macOS 15.1). Run this on the OLDEST macOS you support: every symbol
it prints must be guarded by `__builtin_available`/an `if (&symbol)` test in the code that uses it, or removed
from the build (see native/cmake/libusb for an example). The list is evidence, not a verdict: the app run on that
macOS is the real test.
"""
import ctypes
import ctypes.util
import re
import subprocess
import sys
from pathlib import Path

MACHO = (b"\xcf\xfa\xed\xfe",)


def macho_files(app):
    for path in sorted(Path(app).rglob("*")):
        if path.is_file() and not path.is_symlink():
            with open(path, "rb") as f:
                if f.read(4) in MACHO:
                    yield path


def weak_imports(path):
    out = subprocess.run(["dyld_info", "-imports", str(path)], capture_output=True, text=True).stdout
    for line in out.splitlines():
        m = re.match(r"\s+0x[0-9A-F]+\s+(\S+) \[weak-import\] \(from (.+)\)", line)
        if m:
            yield m.group(1), m.group(2)


def library_handle(name, cache):
    if name not in cache:
        candidates = [name, f"/System/Library/Frameworks/{name}.framework/{name}", f"/usr/lib/{name}.dylib"]
        cache[name] = None
        for candidate in candidates:
            try:
                cache[name] = ctypes.CDLL(candidate)
                break
            except OSError:
                continue
    return cache[name]


def main():
    if len(sys.argv) != 2:
        print(__doc__)
        return 2
    objc = ctypes.CDLL("/usr/lib/libobjc.A.dylib")
    objc.objc_getClass.restype = ctypes.c_void_p
    objc.objc_getClass.argtypes = [ctypes.c_char_p]
    handles, missing = {}, []
    for path in macho_files(sys.argv[1]):
        for symbol, library in weak_imports(path):
            lib = library_handle(library, handles)
            name = symbol[1:] if symbol.startswith("_") else symbol
            if name.startswith("OBJC_CLASS_$_"):
                found = lib is not None and bool(objc.objc_getClass(name[len("OBJC_CLASS_$_"):].encode()))
            elif lib is None:
                found = False
            else:
                try:
                    getattr(lib, name)
                    found = True
                except AttributeError:
                    found = False
            if not found:
                missing.append((path.name, symbol, library))
    version = subprocess.run(["sw_vers", "-productVersion"], capture_output=True, text=True).stdout.strip()
    for owner, symbol, library in missing:
        print(f"{owner}: {symbol} (from {library}) is not available on macOS {version}")
    print(f"check_weak_imports: {len(missing)} weak import(s) missing on macOS {version}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

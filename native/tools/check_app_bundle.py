#!/usr/bin/env python3
"""Static check of a built Petari.app (or any bundle made with petari_bundle_libusb): bundled libusb, loader, licences.

Usage: check_app_bundle.py <path to X.app>
Exit 0 and "result: PASS" only when every check passes. Does not run the app.
"""
import hashlib
import subprocess
import sys
from pathlib import Path

NATIVE = Path(__file__).resolve().parents[1]


def run(*command):
    result = subprocess.run([str(c) for c in command], text=True, capture_output=True)
    if result.returncode:
        raise SystemExit(f"FAIL: {' '.join(map(str, command))}: {result.stdout}{result.stderr}")
    return result.stdout


def main():
    if len(sys.argv) != 2:
        raise SystemExit(__doc__)
    app = Path(sys.argv[1]).resolve()
    macos = app / "Contents/MacOS"
    executables = [p for p in macos.iterdir() if p.is_file()]
    if len(executables) != 1:
        raise SystemExit(f"FAIL: expected one executable in {macos}")
    exe = executables[0]
    lib = app / "Contents/Frameworks/libusb-1.0.0.dylib"
    checks = []

    def check(name, ok):
        checks.append((name, ok))
        print(("PASS " if ok else "FAIL ") + name)

    check("libusb-1.0.0.dylib is a regular file in Contents/Frameworks", lib.is_file() and not lib.is_symlink())
    if lib.is_file():
        check("install name is @rpath/libusb-1.0.0.dylib", "@rpath/libusb-1.0.0.dylib" in run("otool", "-D", lib))
        deps = run("otool", "-L", lib)
        check("libusb depends on no Homebrew/local library", "/opt/homebrew" not in deps and "/usr/local" not in deps)
        check("libusb is arm64", "arm64" in run("lipo", "-archs", lib))
        check("libusb signature verifies", subprocess.run(["codesign", "--verify", "--strict", str(lib)]).returncode == 0)
    check("executable does not link libusb (dynamically loaded by SDL)",
          "libusb" not in "\n".join(run("otool", "-L", exe).splitlines()[1:]).lower())
    check("rpath @executable_path/../Frameworks", "path @executable_path/../Frameworks" in run("otool", "-l", exe))
    data = exe.read_bytes()
    check("SDL loader patch 0001 compiled in (bundle libusb path)",
          b"@executable_path/../Frameworks/libusb-1.0.0.dylib" in data)
    check("SDL patch 0002 compiled in (Switch 2 command interface via libusb)",
          b"Couldn't open the Switch 2 controller's command interface with libusb" in data)
    lic = app / "Contents/Resources/licenses"
    for name, source in (("COPYING", "libusb-COPYING"), ("THIRD_PARTY_NOTICES.txt", "THIRD_PARTY_NOTICES.txt")):
        path = lic / name
        check(f"licence file {name} matches the repository copy",
              path.is_file() and hashlib.sha256(path.read_bytes()).hexdigest() ==
              hashlib.sha256((NATIVE / "licenses" / source).read_bytes()).hexdigest())
    check("bundle signature verifies", subprocess.run(["codesign", "--verify", "--strict", str(app)]).returncode == 0)
    ok = all(ok for _, ok in checks)
    print("result: " + ("PASS" if ok else "FAIL"))
    return 0 if ok else 1


sys.exit(main())

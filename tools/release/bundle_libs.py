#!/usr/bin/env python3
"""Make an app bundle self-contained: copy every non-system dylib it needs into Contents/Frameworks and point the
executable and the copies at them with @rpath (the usual "dylibbundler" job).

    python3 -I bundle_libs.py "Astral Italian.app" [--lib-root DIR]

--lib-root DIR resolves /opt/homebrew/... load paths below DIR instead of the real /opt/homebrew (the preserved
frozen builds keep their runtime libraries as DIR/opt/homebrew/Cellar/<formula>/<version>/lib/*.dylib).
Install-name edits invalidate code signatures, so every Mach-O is ad-hoc re-signed here; the release script signs
the finished bundle again with its final identity.
"""
import argparse
import glob
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

SYSTEM_PREFIXES = ("/usr/lib/", "/System/")
FRAMEWORKS = "Contents/Frameworks"


def run(*args, check=True):
    r = subprocess.run(args, capture_output=True, text=True)
    if check and r.returncode != 0:
        raise SystemExit(f"bundle_libs: {' '.join(args[:3])} failed: {r.stderr.strip()}")
    return r.stdout


def is_macho(path):
    with open(path, "rb") as f:
        return f.read(4) in (b"\xcf\xfa\xed\xfe", b"\xca\xfe\xba\xbe")


def loads_and_rpaths(path):
    loads = []
    for line in run("otool", "-L", str(path)).splitlines()[1:]:
        m = re.match(r"\s+(\S+) \(", line)
        if m:
            loads.append(m.group(1))
    rpaths, lines = [], run("otool", "-l", str(path)).splitlines()
    for i, line in enumerate(lines):
        if line.strip().startswith("cmd LC_RPATH"):
            for follow in lines[i + 1:i + 4]:
                m = re.match(r"\s*path (.+) \(offset", follow)
                if m:
                    rpaths.append(m.group(1))
    return loads, rpaths


def resolve(dep, owner_rpaths, owner_dir, lib_root):
    """Real file for a load path, or None if it is not found. owner_dir is where the loading library lives
    (its original location, which @loader_path rpaths are relative to)."""
    candidates = []
    if dep.startswith("@rpath/"):
        tail = dep[len("@rpath/"):]
        for rp in owner_rpaths:
            if rp.startswith("@loader_path"):
                rp = os.path.normpath(owner_dir + rp[len("@loader_path"):])
            if rp.startswith("/"):
                candidates.append(rp + "/" + tail)
        candidates.append("/opt/homebrew/lib/" + tail)
    elif dep.startswith("/"):
        candidates.append(dep)
    for c in candidates:
        patterns = [c]
        if lib_root:
            if not c.startswith(lib_root + "/"):
                patterns.append(lib_root + c)
            m = re.match(r"/opt/homebrew/opt/([^/]+)/(.+)", c)
            if m:
                patterns.append(f"{lib_root}/opt/homebrew/Cellar/{m.group(1)}/*/{m.group(2)}")
        # An archived runtime keeps libfoo.1.2.3.dylib without its libfoo.1.dylib link.
        for pat in list(patterns):
            stem = re.match(r"(.*/lib[^/]*?\.\d+)\.dylib$", pat)
            if stem:
                patterns.append(stem.group(1) + ".*.dylib")
        for pat in patterns:
            for h in sorted(glob.glob(pat)):
                return os.path.realpath(h)
    return None


def bundle(app, lib_root):
    app = Path(app)
    exe_dir = app / "Contents" / "MacOS"
    frameworks = app / FRAMEWORKS
    frameworks.mkdir(parents=True, exist_ok=True)
    work = [p for p in exe_dir.iterdir() if p.is_file() and is_macho(p)]
    if not work:
        raise SystemExit(f"bundle_libs: no executable in {exe_dir}")
    copied = {}  # real source path -> destination
    origin = {}  # copied destination -> directory of its original (for @loader_path rpaths)
    names = {}   # destination name -> real source path (two different libs must not share a name)
    done = set()
    while work:
        target = work.pop()
        if target in done:
            continue
        done.add(target)
        loads, rpaths = loads_and_rpaths(target)
        in_frameworks = target.parent == frameworks
        for dep in loads:
            if dep.startswith(SYSTEM_PREFIXES) or dep.startswith(("@executable_path/", "@loader_path/")):
                continue
            if dep == f"@rpath/{target.name}":
                continue  # a dylib's own id
            real = resolve(dep, rpaths, origin.get(target, str(target.parent)), lib_root)
            if real is None:
                raise SystemExit(f"bundle_libs: {target.name} needs {dep}, which was not found"
                                 + (f" under {lib_root}" if lib_root else ""))
            name = os.path.basename(dep)
            if name in names and names[name] != real:
                raise SystemExit(f"bundle_libs: two different libraries are both named {name}")
            names[name] = real
            if real not in copied:
                dest = frameworks / name
                shutil.copy2(real, dest)
                os.chmod(dest, 0o755)
                run("install_name_tool", "-id", f"@rpath/{name}", str(dest))
                run("install_name_tool", "-add_rpath", "@loader_path", str(dest), check=False)
                copied[real] = dest
                origin[dest] = os.path.dirname(real)
                work.append(dest)
            run("install_name_tool", "-change", dep, f"@rpath/{name}", str(target))
        for rp in rpaths:
            if rp.startswith("/"):
                run("install_name_tool", "-delete_rpath", rp, str(target))
        if not in_frameworks:
            run("install_name_tool", "-add_rpath", "@executable_path/../Frameworks", str(target), check=False)
    for lib in sorted(frameworks.iterdir()):
        if lib.is_file() and is_macho(lib):
            run("codesign", "--force", "--sign", "-", str(lib))
    for exe in sorted(exe_dir.iterdir()):
        if exe.is_file() and is_macho(exe):
            run("codesign", "--force", "--sign", "-", str(exe))
    return len(copied)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("app")
    ap.add_argument("--lib-root", help="resolve /opt/homebrew/... below this directory")
    args = ap.parse_args()
    count = bundle(args.app, os.path.realpath(args.lib_root) if args.lib_root else None)
    print(f"bundle_libs: {count} libraries copied into {args.app}/{FRAMEWORKS}")


if __name__ == "__main__":
    sys.exit(main())

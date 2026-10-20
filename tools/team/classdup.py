#!/usr/bin/env python3
"""Fail if a class or struct body is defined in two places at top level (ODR violations).

Two bodies for one class can agree on the Wii (32-bit) and disagree natively (64-bit pointers): the
MiniRoutePoint, WorldMapSequence and CollisionObj world-map/boss crashes (2026-10-20) were all a second
body (an older copy, or a u8 stand-in) in another header or .cpp. This check finds every top-level
(column 0) `class X ... {` / `struct X ... {` in include/, libs/ and src/ and reports names with bodies in
more than one file. Nested types are indented and not checked (same name in different scopes is fine).

Exceptions:
  - src/Game/NameObj/NameObjFactory.cpp: deliberate Wii-sized stand-ins for the creator table (the native
    build uses NameObjFactoryNative.cpp instead).
  - tools/team/classdup-allow.txt: known duplicates awaiting their owner, one `Name  # owner: why` per line.
    Remove a line when the duplicate is fixed; a listed name that is no longer duplicated is reported as a
    warning (not a failure, so landing a fix does not block the next landing).
Usage: classdup.py [--root DIR]   (exit 0: no new duplicates; 1: new duplicates, listed)
"""
import collections
import os
import re
import sys

PATTERN = re.compile(r"^(?:class|struct)\s+(\w+)\s*(?:final\s*)?(?::[^{;]*)?\{", re.M)
EXCLUDED_FILES = {"src/Game/NameObj/NameObjFactory.cpp"}
SUFFIXES = (".hpp", ".h", ".cpp", ".c", ".inc")


def find_definitions(root):
    defs = collections.defaultdict(set)
    for top in ("include", "libs", "src"):
        for dirpath, _, names in os.walk(os.path.join(root, top)):
            for name in names:
                if not name.endswith(SUFFIXES):
                    continue
                path = os.path.join(dirpath, name)
                rel = os.path.relpath(path, root)
                if rel in EXCLUDED_FILES:
                    continue
                with open(path, encoding="utf-8", errors="replace") as f:
                    text = f.read()
                for match in PATTERN.finditer(text):
                    defs[match.group(1)].add(rel)
    return defs


def read_allowlist(root):
    allowed = {}
    path = os.path.join(root, "tools", "team", "classdup-allow.txt")
    if os.path.exists(path):
        for line in open(path, encoding="utf-8"):
            entry = line.split("#", 1)[0].strip()
            if entry:
                allowed[entry] = line.strip()
    return allowed


def main(argv):
    root = argv[argv.index("--root") + 1] if "--root" in argv else "."
    defs = find_definitions(root)
    duplicates = {name: sorted(files) for name, files in defs.items() if len(files) > 1}
    allowed = read_allowlist(root)
    new = {name: files for name, files in duplicates.items() if name not in allowed}
    stale = sorted(name for name in allowed if name not in duplicates)
    for name, files in sorted(new.items()):
        print(f"classdup: {name} has a body in {len(files)} files: {', '.join(files)}")
    for name in stale:
        print(f"classdup: warning: {name} is listed in tools/team/classdup-allow.txt but no longer duplicated: remove the line")
    if new:
        print("classdup: one class, one definition: delete the extra body and #include the canonical header "
              "(see tools/team/objsame.py to prove the Wii objects unchanged)")
        return 1
    print(f"classdup: OK ({len(allowed)} allowlisted)")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))

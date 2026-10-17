#!/usr/bin/env python3
"""Mechanical Petari (SMG1) -> SMG2 source rewrites that are safe across many units.

  port.py FILE.cpp [FILE.cpp ...]      rewrite in place
  port.py --stdout FILE.cpp            print the result

Rewrites:
  - adds `#include "Game/Util.hpp"` after the first line (Petari includes per-file Util headers;
    Garigari keeps many MR:: decls in different headers)
  - MR::startSound / MR::startLevelSound(actor, "SE_EM_GESSO_SWIM_L", ...) ->
    MR::startActionSound(actor, "EmGessoSwimL", ...): SMG2 plays sounds through the action-sound table,
    keyed by the CamelCased SE name without the "SE_" prefix. Extra args are kept.
  - initSound(N, false) -> initSound(N, "<EnclosingClass>", nullptr, TVec3f(0.0f)) (SMG2 signature; the name is
    usually the class/archive name: check the string in the target .data)
Check the result with sg.py; these are heuristics, not guarantees.
"""
import re, sys


def camel(se):
    return "".join(w[:1].upper() + w[1:].lower() for w in se.split("_") if w)


def rewrite(s):
    lines = s.split("\n")
    if '#include "Game/Util.hpp"' not in s and lines and lines[0].startswith("#include"):
        lines.insert(1, '#include "Game/Util.hpp"')
    s = "\n".join(lines)
    s = re.sub(r'MR::start(?:Level)?Sound\(([^,()]+(?:\([^()]*\))?),\s*"SE_([A-Z0-9_]+)"',
               lambda m: f'MR::startActionSound({m.group(1)}, "{camel(m.group(2))}"', s)
    # binder getters return const TVec3f& in Garigari (Petari: pointer). Same mangled name, so just adapt uses.
    g = r"MR::get(?:Ground|Wall|Roof|Binded)(?:Normal|HitPos)\(|MR::getBindedFixReactionVector\("
    s = re.sub(r"\*\s*(" + g + ")", r"\1", s)
    s = re.sub(r"(TVec3f\*\s*\w+\s*=\s*)(" + g + ")", r"\1&\2", s)
    s = re.sub(r"(TVec3f\*\s*\w+\s*=\s*)\(?(" + g + r"[^;]*?\))\)?;", r"\1&\2;", s) if False else s
    # MR::getSensorPos returns const TVec3f* in Garigari (Petari: const TVec3f&). Without the deref,
    # `getSensorPos(a) - getSensorPos(b)` silently compiles as a POINTER difference.
    s = re.sub(r"(?<![*&\w])MR::getSensorPos\(", "*MR::getSensorPos(", s)
    s = s.replace("&*MR::getSensorPos(", "MR::getSensorPos(")
    # LiveActor::initSound(s32, bool) -> SMG2 initSound(s32, "<ClassName>", nullptr, TVec3f(0.0f))
    out, cls = [], None
    for l in s.split("\n"):
        m = re.match(r"^[\w:<>*& ]*?\b(\w+)::\w+\(", l)
        if m and not l.startswith(" "):
            cls = m.group(1)
        if cls:
            l = re.sub(r"\binitSound\((\w+), (?:false|true)\)", lambda mm: f'initSound({mm.group(1)}, "{cls}", nullptr, TVec3f(0.0f))', l)
        out.append(l)
    return "\n".join(out)


def main():
    out = "--stdout" in sys.argv
    for f in [a for a in sys.argv[1:] if not a.startswith("--")]:
        s = open(f, encoding="utf-8", errors="surrogateescape").read()
        r = rewrite(s)
        if out:
            sys.stdout.write(r)
        elif r != s:
            open(f, "w", encoding="utf-8", errors="surrogateescape").write(r)


if __name__ == "__main__":
    main()

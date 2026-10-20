#!/usr/bin/env python3
"""A texgen whose source is a texture coordinate the current vertex format does not supply reads zero
(the hardware vertex pipe fetches nothing). SMG2 does this (e.g. on the file-select scene); Aurora's shader
generator aborted with "unmapped vtx attr 13". Usage: patch_aurora_missing_texcoord.py <shader.cpp> <output>."""
from pathlib import Path
import sys

ANCHOR = '''    if (attr == GX_VA_CLR0 || attr == GX_VA_CLR1) {
      return "vec4f(0.0, 0.0, 0.0, 0.0)"s;
    }
    UNLIKELY FATAL("unmapped vtx attr {}", underlying(attr));'''
REPLACEMENT = '''    if (attr == GX_VA_CLR0 || attr == GX_VA_CLR1) {
      return "vec4f(0.0, 0.0, 0.0, 0.0)"s;
    }
    if (attr >= GX_VA_TEX0 && attr <= GX_VA_TEX7) {
      // Texture coordinate not in the vertex format: the vertex pipe supplies zero.
      return "vec2f(0.0, 0.0)"s;
    }
    UNLIKELY FATAL("unmapped vtx attr {}", underlying(attr));'''

text = Path(sys.argv[1]).read_text()
if text.count(ANCHOR) != 1:
    raise SystemExit("missing-texcoord patch anchor mismatch")
Path(sys.argv[2]).parent.mkdir(parents=True, exist_ok=True)
Path(sys.argv[2]).write_text(text.replace(ANCHOR, REPLACEMENT))

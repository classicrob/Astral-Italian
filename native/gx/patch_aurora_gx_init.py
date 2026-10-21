#!/usr/bin/env python3
"""Restore the SDK GXInit attribute-format defaults in Aurora's generated provider.

Compose after patch_aurora_allocations.py and before patch_aurora_sync.py.
This changes only shadow VAT initialization, through the existing SDK setter.
"""
from pathlib import Path
import sys

ANCHOR = "  GXClearVtxDesc();\n  GXInvalidateVtxCache();\n"
DEFAULTS = """\
  // Restore the SDK default component formats in every VAT bank.
  static const GXVtxAttrFmtList defaults[] = {
      {GX_VA_POS, GX_POS_XYZ, GX_F32, 0},
      {GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0},
      {GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0},
      {GX_VA_CLR1, GX_CLR_RGBA, GX_RGBA8, 0},
      {GX_VA_TEX0, GX_TEX_ST, GX_F32, 0},
      {GX_VA_TEX1, GX_TEX_ST, GX_F32, 0},
      {GX_VA_TEX2, GX_TEX_ST, GX_F32, 0},
      {GX_VA_TEX3, GX_TEX_ST, GX_F32, 0},
      {GX_VA_TEX4, GX_TEX_ST, GX_F32, 0},
      {GX_VA_TEX5, GX_TEX_ST, GX_F32, 0},
      {GX_VA_TEX6, GX_TEX_ST, GX_F32, 0},
      {GX_VA_TEX7, GX_TEX_ST, GX_F32, 0},
      {GX_VA_NULL, static_cast<GXCompCnt>(0), static_cast<GXCompType>(0), 0},
  };
  for (i = 0; i < GX_MAX_VTXFMT; i++) {
    GXSetVtxAttrFmtv(static_cast<GXVtxFmt>(i), defaults);
  }
"""


def patch(text):
    if DEFAULTS in text:
        raise ValueError("Aurora GXInit defaults are already patched")
    signature = "GXFifoObj* GXInit(void* base, u32 size) {"
    if text.count(signature) != 1 or text.count(ANCHOR) != 1:
        raise ValueError("Aurora GXInit default-format anchor mismatch")
    if not text.index(signature) < text.index(ANCHOR) < text.index("  return &sFifoObj;"):
        raise ValueError("Aurora GXInit default-format anchor is outside GXInit")
    return text.replace(ANCHOR, ANCHOR + DEFAULTS)


def main():
    if len(sys.argv) != 3:
        raise SystemExit("usage: patch_aurora_gx_init.py SOURCE OUTPUT")
    source, output = map(Path, sys.argv[1:])
    result = patch(source.read_text())
    output.parent.mkdir(parents=True, exist_ok=True)
    if not output.exists() or output.read_text() != result:
        output.write_text(result)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""5.4 v2 spot check: pure-Python PNG diff (no Pillow on the server) of off vs forced-all surface dumps.
Same metric as SOL 2's harness: mean absolute RGB difference over all pixels and channels (0..255, alpha ignored)
and the max per-channel difference. Pairs files by name; fails if a pair is missing or the sizes differ.
  imgdiff54.py <off dump dir> <all dump dir> [--threshold 0.01] -> JSON on stdout; exit 1 if any mean >= threshold
"""
import json, os, struct, sys, zlib

def decode(path):
    data = open(path, 'rb').read()
    assert data[:8] == b'\x89PNG\r\n\x1a\n', path
    pos, idat, w = 8, [], None
    while pos < len(data):
        n, kind = struct.unpack('>I4s', data[pos:pos + 8]); body = data[pos + 8:pos + 8 + n]; pos += 12 + n
        if kind == b'IHDR':
            w, h, depth, ctype, _, _, interlace = struct.unpack('>IIBBBBB', body)
            assert depth == 8 and ctype in (2, 6) and interlace == 0, (path, depth, ctype, interlace)
            bpp = 4 if ctype == 6 else 3
        elif kind == b'IDAT':
            idat.append(body)
    raw = zlib.decompress(b''.join(idat)); stride = w * bpp
    out = bytearray(h * stride); prev = bytearray(stride)
    for y in range(h):
        f = raw[y * (stride + 1)]; line = bytearray(raw[y * (stride + 1) + 1:(y + 1) * (stride + 1)])
        if f == 1:
            for i in range(bpp, stride): line[i] = (line[i] + line[i - bpp]) & 255
        elif f == 2:
            for i in range(stride): line[i] = (line[i] + prev[i]) & 255
        elif f == 3:
            for i in range(stride): line[i] = (line[i] + ((line[i - bpp] if i >= bpp else 0) + prev[i]) // 2) & 255
        elif f == 4:
            for i in range(stride):
                a = line[i - bpp] if i >= bpp else 0; b = prev[i]; c = prev[i - bpp] if i >= bpp else 0
                p = a + b - c; pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                line[i] = (line[i] + (a if pa <= pb and pa <= pc else b if pb <= pc else c)) & 255
        out[y * stride:(y + 1) * stride] = line; prev = line
    return w, h, bpp, bytes(out)

def diff(a, b):
    wa, ha, ba, da = decode(a); wb, hb, bb, db = decode(b)
    assert (wa, ha) == (wb, hb), (a, b, wa, ha, wb, hb)
    total = mx = 0
    for i in range(0, len(da), ba):
        for c in range(3):
            d = abs(da[i + c] - db[i + c]); total += d
            if d > mx: mx = d
    return {'frame': os.path.basename(a), 'size': [wa, ha], 'mean_abs_rgb': total / (wa * ha * 3), 'max': mx}

def main():
    off, alld = sys.argv[1], sys.argv[2]
    thr = float(sys.argv[sys.argv.index('--threshold') + 1]) if '--threshold' in sys.argv else 0.01
    names = sorted(n for n in os.listdir(off) if n.endswith('.png'))
    assert names and names == sorted(n for n in os.listdir(alld) if n.endswith('.png')), 'unpaired dumps'
    rows = [diff(os.path.join(off, n), os.path.join(alld, n)) for n in names]
    mean = sum(r['mean_abs_rgb'] for r in rows) / len(rows)
    res = {'pairs': rows, 'mean_abs_rgb': mean, 'max': max(r['max'] for r in rows), 'threshold': thr,
           'result': 'PASS' if mean < thr else 'FLAG'}
    print(json.dumps(res)); sys.exit(0 if mean < thr else 1)

main()

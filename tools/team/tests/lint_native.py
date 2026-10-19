#!/usr/bin/env python3
"""Check actual controller big-integer helpers against Python integers on the host."""
import ctypes
import os
from pathlib import Path
import random
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]
WORD = ctypes.c_uint32
POINTER = ctypes.POINTER(WORD)
MASK = (1 << 32) - 1


def operand(value):
    limbs = []
    while value:
        limbs.append(value & MASK)
        value >>= 32
    if not limbs:
        limbs = [0]
    # Space for the largest 1024-bit intermediate, zero extension and sentinel.
    return (WORD * 68)(len(limbs), *limbs)


def integer(words):
    assert 1 <= words[0] < len(words)
    return sum(words[i + 1] << (32 * i) for i in range(words[0]))


def exercise(library):
    signatures = [
        ("LINTCmp", [POINTER, POINTER], ctypes.c_int32),
        ("LINTMsb", [POINTER], WORD),
        ("LINTLshift", [POINTER, POINTER, WORD], None),
        ("LINTSub", [POINTER, POINTER, POINTER], None),
        ("LINTMul", [POINTER, POINTER, POINTER], None),
    ]
    for name, arguments, result in signatures:
        function = getattr(library, name)
        function.argtypes = arguments
        function.restype = result
    rng = random.Random(519)
    edges = [0, 1, MASK, 1 << 32, (1 << 512) - 1, 1 << 1023]
    cases = [(x, y) for x in edges for y in edges[:-1]]
    cases += [(rng.getrandbits(rng.randrange(1, 1025)),
               rng.getrandbits(rng.randrange(1, 513))) for _ in range(1000)]
    checks = 0
    for x, y in cases:
        left, right = operand(x), operand(y)
        assert library.LINTCmp(left, right) == ((x > y) - (x < y))
        assert library.LINTMsb(left) == x.bit_length()
        checks += 2
        high, low = max(x, y), min(x, y)
        for inplace in (False, True):
            left = operand(high)
            output = left if inplace else operand(0)
            library.LINTSub(output, left, operand(low))
            assert integer(output) == high - low
            checks += 1
        # Actual certificate multiplies have at most sixteen limbs per operand.
        a, b = x & ((1 << 512) - 1), y & ((1 << 512) - 1)
        output = operand(0)
        library.LINTMul(output, operand(a), operand(b))
        assert integer(output) == a * b
        left = operand(a)
        output = operand(0)
        library.LINTMul(output, left, left)
        assert integer(output) == a * a
        output = operand(0)
        shift = rng.randrange(512)
        library.LINTLshift(output, operand(a), shift)
        assert integer(output) == a << shift
        checks += 3
    return checks


def main():
    with tempfile.TemporaryDirectory(prefix="lint-native-") as temporary:
        for mode, flags in [("optimized", []),
                            ("ubsan", ["-fsanitize=undefined", "-fno-sanitize-recover=all"])]:
            output = Path(temporary) / (mode + ".dylib")
            subprocess.run([
                os.environ.get("CXX", "clang++"), "-std=c++11", "-O2", *flags,
                "-x", "c++", "-shared", "-I" + str(ROOT / "libs/RVL_SDK/include"),
                str(ROOT / "src/RVL_SDK/wpad/lint.c"), "-o", str(output),
            ], check=True)
            print(f"{mode}: {exercise(ctypes.CDLL(str(output)))} arithmetic checks passed")


if __name__ == "__main__":
    main()

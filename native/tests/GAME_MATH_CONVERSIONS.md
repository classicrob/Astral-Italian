# Native game math conversions (U066–072)

Run the actual production functions with ASan, UBSan and float-cast-overflow:

```sh
python3 native/tests/game_math_conversion_tests.py clang++
```

The runner builds only MathUtil, MtxUtil, JMATrigonometric and the focused test
in a temporary directory. CTest registers it as `native_game_math_conversion`.
It needs the canonical JMath conversion header from dependency
`f0ae8ef284419a67dd3d1c2c1ae043f840f578c4`; no local helper implementation is used.
The controls compile successfully, then must fail at runtime: old Math casts,
old rotation casts, and an incorrect five-bit-only shift count.

The SB4E01 target uses signed-word `fctiwz` before narrowing, not s16 saturation:

| Site | Target | Conversion and use |
| --- | --- | --- |
| `MR::floatToFixed16` x/y/z | 8003DA90–8003DB0C | `fmuls` at +38/+3C/+40, `fctiwz` at +44/+48/+4C, `sth` at +68/+6C/+70 |
| `MR::makeMtxRotate` float overload | 80044860–800448C0 | `fmuls` at +10/+14/+18, `fctiwz` at +1C/+20/+24, low16 `extsh` arguments, call signed-angle overload at +4C |
| `MR::makeMtxRotateY` | 800448D0–80044944 | `fmuls` at +10, `fctiwz` at +1C, table byte offset `(word << 1) & 0x1FFF8` at +28 |

Rotation multiplier is binary32 `0x43360B61` (182.04444885253906).
The native paths retain one binary32 product before canonical `truncToS16`.
NaN/negative word overflow give INT_MIN, positive word overflow gives INT_MAX;
low16 narrowing then yields 0 or -1 respectively. Ordinary finite values wrap:
32768 becomes -32768; 65535.75 truncates to -1 after narrowing.

`floatToFixed16` also reproduces the target's `slw` at +8: `q & 63` is the
count, counts 32–63 give zero, and count 31 gives signed INT_MIN before
conversion to the binary32 scale. Defined native branches cover every u8 `q`.
No change is made to the separate, unassigned `fixed16ToFloat` function.
All game edits are GALAXY_NATIVE-only; original Wii expressions remain intact.

Tests cover fractional/negative values, halfword wrap, signed-word boundaries,
finite overflow, NaN/Inf, destination bounds, all 256 shift inputs and rotation
results. Matrix reference multiplication uses the existing JMath lookup table
and a small tolerance; it is a conversion regression test, not a proof of all
native matrix arithmetic or a live-game runtime validation.

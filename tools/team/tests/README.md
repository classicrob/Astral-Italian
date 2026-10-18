# Portable JSystem math tests

Run from the repository root:

```sh
python3 tools/team/tests/jmath_native.py
tools/team/tests/run_jgeometry_native.sh
```

Both runners compile actual repository headers with a native C++11 compiler and exercise optimized and AddressSanitizer/UndefinedBehaviorSanitizer builds. They require Clang or a compatible compiler, sanitizer runtimes, and Python 3 for JMath. No replacement SDK function bodies are provided. JMath drops unused SDK-dependent Euler/matrix functions at link time; JGeometry also checks linking headers from two translation units.

JGeometry covers vector construction, assignment, zeroing, dot/distance, aliases, short-vector alignment, temporary results, and matrix translation. Its low-level `subInternal` deliberately preserves the existing PPC helper's reversed Z subtraction; normal `TVec3::sub` uses conventional subtraction. The script also compares CodeWarrior preprocessed headers with HEAD; this comparison is useful while reviewing uncommitted changes.

See [jmath_native.md](jmath_native.md) for arithmetic contracts and numerical limits. These tests do not establish SDK ABI compatibility or run the native game.

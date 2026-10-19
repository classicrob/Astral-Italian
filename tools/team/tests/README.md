# Portable JSystem math tests

Run from the repository root:

```sh
python3 tools/team/tests/jmath_native.py
tools/team/tests/run_jgeometry_native.sh
```

Both runners compile actual repository headers with a native C++11 compiler and exercise optimized and AddressSanitizer/UndefinedBehaviorSanitizer builds. They require Clang or a compatible compiler, sanitizer runtimes, and Python 3 for JMath. No replacement SDK function bodies are provided. JMath drops unused SDK-dependent Euler/matrix functions at link time; JGeometry also checks linking headers from two translation units.

JGeometry covers vector construction, assignment, zeroing, dot/distance, aliases, short-vector alignment, temporary results, and matrix translation. Its low-level `subInternal` deliberately preserves the existing PPC helper's reversed Z subtraction; normal `TVec3::sub` uses conventional subtraction. The script also compares CodeWarrior preprocessed headers with HEAD; this comparison is useful while reviewing uncommitted changes.

See [jmath_native.md](jmath_native.md) for arithmetic contracts and numerical limits. These tests do not establish SDK ABI compatibility or run the native game.

SDK address macros also have a native regression fixture for pointer-width address translation, memory-region tests and 32-byte alignment:

```sh
clang++ -std=c++11 -O2 -fsanitize=undefined -fno-sanitize-recover=all \
    -Ilibs/RVL_SDK/include tools/team/tests/os_address_native.cpp -o /tmp/os-address-native
/tmp/os-address-native
```

The full Wii build and per-unit matched-byte guard check that these address-width changes preserve the 32-bit target.

Controller certificate integer helpers have an independent Python-integer reference test:

```sh
python3 tools/team/tests/lint_native.py
```

It compiles the actual source in optimized and UBSan modes and checks carries, borrows, zero normalization, shifts, input squares and in-place subtraction on a native host. Shift/multiply use separate output storage, matching the original controller callers. This test does not replace the original-PPC comparison used during recovery.

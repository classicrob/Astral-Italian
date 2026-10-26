# Registered fixture repair (5.4.3)

Base: main `7fdb1c38c55d3384439524e789199289d6b7bd82`.
Only fixture sources, BUILD_TESTING executable wiring, and proof tools change.
No production source, SDK header, assertion threshold, stimulus or comparison
predicate changes. The startup fixture's obsolete product-name literal is updated
to the actual `Astral Italian` title.

| Registered check | Stale fixture | Repair |
| --- | --- | --- |
| native_platform_audio | AIGetDSPSampleRate has no public declaration | Declare the existing exported function in the test TU. |
| native_rfl_boot | RFLGetAdditionalInfo now takes sanitizeNames | Pass TRUE, as the SDK's sanitized-name wrapper does. |
| native_platform_dvd | DVDPause / DVDResume / DVDCancelAsync lack public declarations | Declare the existing exports in the test TU. |
| native_game_read_input / native_input | Rebuilding a Rig resets WPAD, but leaves the real SDK's private kpadInitialized guard set | Include the existing unchanged-SDK test wrapper in the test executable only; reset its private guard with the fixture, under the interrupt lock. |
| native_pipeline_startup | Old MacioGalaxy title expectation | Expect the production Astral Italian title. |

The root input archive still compiles the original KPAD source. Only the test
executable provides its symbols through `kpad_test_support.cpp`; its original
archive member is consequently not extracted. This is already the standalone
fixture's SDK wrapper. The production input archive has no new definition or
source, and normal play never calls the reset hook. Standalone builds avoid a
second wrapper. Every original check and tolerance in input_tests.cpp remains.

## Before / after proof

At the initial source handoff, native results were pending PORT's server run.
The r1 results and subsequent source diagnosis appear below. Source inspection
or a Python proof-tool test is not a native PASS. In particular, the full native_input suite
may expose the already documented later Alt+W+D half-stick assertion after its
lifecycle blocker is repaired. Preserve that failure; do not adjust its predicate.

Use a root GX test build on this branch, with BUILD_TESTING and
CMAKE_EXPORT_COMPILE_COMMANDS enabled. Explicitly build these targets (some are
excluded from ALL):

```sh
cmake --build "$TEST_BUILD" --target petari_platform_audio_tests petari_rfl_boot_tests petari_platform_dvd_tests petari_input_tests petari_pipeline_startup_tests
python3 tools/port/check_stale_fixtures543.py --prepare --build "$TEST_BUILD" --out "$PROOF_OUT"
```

Caller owns server-heavy -> native locks, nice 0. After preparation finishes,
obtain a **new** quiet receipt (load 1/5 <= 8, no compiler/app, two Metal CPU samples
10 s apart with each delta < .05 s). Then, without a concurrent build/run:

```sh
python3 tools/port/check_stale_fixtures543.py --run --build "$TEST_BUILD" --out "$PROOF_OUT" --quiet-json "$QUIET_JSON"
```

The proof copies each baseline fixture verbatim with `git show` from the pinned
main commit, uses its fixed target's compile/link commands, and substitutes only
the fixture object. For baseline input it removes the added test SDK wrapper, so
the original archive SDK object is linked, as on main. Other production objects
and archives are shared, with object/archive/executable hashes checked before and after
runtime. Baseline compile errors, baseline runtime errors and fixed results are
all retained. REPORT.json requires the named original failure and an exit-0 after
result for each of the six registered checks; it never relabels a later failure
as success. Compile/link and runtime are separate phases so compile load cannot
contaminate the runtime comparison. A runtime report cannot be overwritten.

All child processes receive an isolated HOME/CFFIXED_USER_HOME and TMPDIR in the
new proof directory. No app, Metal namespace, disc, recording, user-cache listing
or user-cache cleanup is involved. The registered suite should then be built/run
by PORT separately; this paired proof is not a claim about unrelated fixtures.

## Server paired proof r1 and approved runtime corrections

PORT ran `7dff0b62` vs pinned main on the server, quiet receipt passed
(load 5.0 / 7.9), proof rc 1. Evidence: server
`port53/m1g/stale-fixtures-543-r1/REPORT.json` and per-test logs;
`win54/build-records/stale-fixtures-543-r1`.

- Main fails / fixed passes: native_platform_audio, native_game_read_input,
  native_pipeline_startup.
- native_rfl_boot and native_platform_dvd now compile, exposing the two incorrect
  runtime fixtures described below. The r1 failures remain preserved.
- native_input now runs 1033 checks and has one remaining failure:
  `Alt+W+D: half-length diagonal (0.616026)`. That is a separate, unwaived item;
  it is not a lifecycle-fix PASS.

### DVD: wrong fixture entry, not case-sensitive production

On pinned main, `sampleDisc()` in native/tests/platform_dvd_tests.cpp:164-173
contains `StageData/MarioFaceShipGalaxy/MarioFaceShipGalaxy.arc`, entry 7. The
failing assertion at line 256 instead requests `AstroGalaxy/ASTROGALAXY.ARC`,
which is absent. Case folding must not turn different names into a match.

The Wii SDK `src/RVL_SDK/dvd/dvdfs.c:48-59` lowercases both operands in `isSame`
and requires the path to end at NUL or slash. Native
`native/platform/dvd/dvd_fst.cpp:21-42` does the same with explicit ASCII folding;
its lookup invokes that predicate at line 534. Production already implements the
expected case-insensitive behavior. This conclusion follows from the retained
SDK source, not a new Wii measurement.

PORT approved correcting the existing case test to the actual MarioFaceShip
entry, adding mixed-case directory and file checks, and retaining the exact old
Astro path as an explicit missing-entry `-1` negative. No production fix.

### RFL: null arguments are WrongParam on both SDK paths

`src/RVLFaceLib/RFL_System.c:47-52` returns `RFLErrcode_WrongParam` for null
resource/work pointers **before** the `GALAXY_NATIVE` block at line 54. This is
therefore also the Wii code path. The fixture's null-resource `Fatal` expectation
was wrong. The enum distinguishes both values in
`libs/RVLFaceLib/include/RFL_Types.h`.

The preceding diagnostic about 17 archives belongs to the deliberately malformed
non-null resource and its successful Fatal check. Native validation at
`native/resource/rfl/rfl_native.cpp:351-352` rejects the mismatched count, then
RFL_System.c:59-61 returns Fatal. That behavior and its assertion stay unchanged.

PORT approved null-resource -> WrongParam and an additional null-work-buffer
WrongParam negative. No resource validator, SDK, or production change. These
approved corrections still need the incremental server before/after run; no
native execution was done on the audio-reserved M1.

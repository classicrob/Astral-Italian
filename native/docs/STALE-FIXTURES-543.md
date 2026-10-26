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

## Server paired proof r2: remaining production issues

PORT ran `489398d5` with a passed quiet receipt (load 6.5 / 6.8), preserved at
server `port53/m1g/stale-fixtures-543-r2` and
`win54/build-records/stale-fixtures-543-r2`.

Four of six checks are now repaired: native_platform_audio, native_rfl_boot,
native_game_read_input, native_pipeline_startup. The new DVD lookup checks pass.
The complete paired proof still FAILS; neither remaining failure is waived.

- **DVD host-path contract:** the next original assertion fails. The native-only
  hostPathForEntry helper promises empty for directories (header
  native/platform/include/petari/platform/dvd.hpp:63-66), but dvd_host.cpp:613-618
  omits the isDir guard and returns the directory path stored by the disc-FST
  loader. Entry 1 remains AudioRes; adding lookup assertions did not change the
  sample FST or its entry numbers. The invalid entry 99 is already guarded.
  Directory-scan and overlay loaders store empty directory paths, so the helper
  also varies by mount mode. This is a production defect, assigned separately to
  port/543-dvd-host-path. Its original assertion remains unchanged here.
- **Input radial encoding:** the unchanged Alt+W+D assertion still fails at
  length 0.616026. Native code independently inverts a cross clamp per axis,
  whereas the real SDK selects the circular clamp by default. This is a separate
  gameplay-feel defect, assigned to port/543-input-radial. The repair, its
  derived quantization limits and SDK-decoded tests belong to that production
  branch, not this fixture-only branch. This assertion is not waived here.

## Combined production-fix checkout

The paired fixture proof's default still rejects any production delta. To run
it on an integrated checkout containing the two separately approved repairs,
repeat these explicit options on **both** --prepare and --run:

```sh
--production-fix 610ded6eb47f5ef4ab5cc35f3bc01f00a17c5c86 --production-fix ec0d1424ad15fedff82f683cdb6ef356270cff5a
```

Only these exact pinned commits are accepted. Each must be based on the pinned
main, and the shipping DVD/input file blobs in HEAD must equal the approved
commit's blobs. Their changed paths are explicitly admitted into the scope;
unknown commits or different shipping code fail closed. Production objects and
archives are still shared and hash-checked between fixture controls.

The receipt explicitly labels this as MAIN FIXTURE SOURCE versus fixed fixtures
on the **same repaired production objects**, NOT a before/after production
comparison. Each repair's separate production regression test is also required:
native_dvd_host_path and native_input_stick. The original six proof outcomes and
all actual old server receipts remain distinct.

PORT's separate server build of input ec0d1424 reports native_input_stick PASS
(actual SDK circle-clamp vectors), with tape play/record PASS. The full input
executables on that independent branch still hit main's known Rig lifecycle;
they need this fixture branch in the combined checkout. DVD's first new target
failed to compile because mkdtemp lacked its POSIX include; 610ded6e adds unistd.h
without changing production. Its compile/run remains pending on the server.
The explicit production-fix DVD pin above is updated to that tested-source fix
line; ancestry is checked against the same pinned main, allowing its two commits.
No M1 compiler ran: the attempted nonblocking lock check refused before launch.

## Combined server r3: stall fixture and overlay fixture

PORT's local combined checkout `202bc5ed` contained fixtures 95a828ec, DVD
610ded6e and input ec0d1424. Quiet receipt passed. The separate production
regressions native_dvd_host_path and native_input_stick both PASS. The paired
proof remains 4/6 (audio, RFL, game-read input, startup); the repaired directory
and radial assertions pass before these next failures. Evidence is preserved at
server `port53/m1g/stale-fixtures-543-r3-combined` and
`win54/build-records/stale-fixtures-543-r3-combined`. The build rc2 came from a
nonexistent requested target name, not a failed actual target.

### Input: wrong clock AND ring size for a live-stall claim

The original stall fixture used Clock::Manual and explicitly called pumpReports
while withholding game reads. The shipping default is Clock::GameRead
(wpad_host.cpp:147,1450-1456); no reports advance without a KPAD read. Its host
alarm is installed only for explicit Clock::Alarm (:1407-1411). Headless keyboard
also selects GameRead (headless/main.cpp:439). refreshRaw retains press edges
(remote_model.cpp:269-277), and updateOutput consumes them on later virtual
reports (:564-569). The source therefore does not establish a lost live press.

There is also a concrete buffer mismatch: Rig called KPADInit, implemented as
KPADInitEx(nullptr,0) by native/input/smg2_extensions.cpp:16-17. That leaves the
16-sample internal ring (kpad.h:29; KPAD.c:1687-1689). WPadHolder.cpp:57-58 actually
supplies 120 external samples per channel, in addition to those 16. KPAD.c:1763-1776
wraps and caps the pending count at that combined capacity.

In the five-frame case, press begins after frame80/report266. The first resumed
read at frame86/report286 sees 20 new reports. The fixture's 16-slot ring has
lost all four 3g impulse samples (remote_model.cpp:27-35); only the return remains.
KPAD's 0.15 filter (WPadAcceleration.cpp:15-17; KPAD.c:280-306) then has a newest
x value about .9861, below the 1g WPadHVSwing threshold (WPadHVSwing.cpp:14-19).
An intermediate retained value briefly exceeds 1, but WPad is not updated at that
intermediate sample. With all 20 samples retained, newest x is about 1.0926.
These values are a source-formula reconstruction, not a new native measurement.
The model's read-within-21-reports heuristic then marks the small-ring case seen
(remote_model.cpp:690-695), so that lost small-ring impulse is not replayed.

PORT approved test-only changes: the gated Manual stall test now uses persistent
120/channel external storage and retains the exact-one-swing expectation at all
18 original stall lengths. A new GameRead test presses AND releases F while no
reads occur (1/5/40 nominal host frames), verifies no input-frame advance, and
requires exactly one swing after resume. It runs in native_game_read_input and
native_input, and is also available together with the game-buffer Manual stress
as `petari_input_tests --shake-stall-only`. A failure still stops the chain.

The old Manual/internal-16 stress remains as
`petari_input_tests --small-ring-shake-characterization`: it prints every original
stall length and observed swing count as CHARACTERIZATION, has no passing
expectation, no CTest registration and no acceptance verdict. No waveform,
replay heuristic, input clock or shipping code is changed to satisfy this case.

### DVD overlay: another stale Astro path

sampleDisc contains MarioFaceShipGalaxy, but the first overlay path still named
`stagedata/ASTROGALAXY/astrogalaxy.ARC`. applyOverlay correctly creates missing
parents (dvd_fst.cpp:370-373), counts a new file as added (:396-399), and only
counts an existing file as replaced (:400-401). This predicts 2 replacements,
3 additions and 5 new entries for the wrong path. The fixture expects 3/2 and
3 new entries, and then asserts that the original MarioFaceShip file grew,
confirming the replacement path is stale. PORT approved correcting that overlay
path to `stagedata/MARIOFACESHIPGALAXY/mariofaceshipgalaxy.ARC`. All existing
count, growth, readback and offset assertions remain unchanged.

These new test-only corrections await PORT's next server run. No native M1 job,
compiler attempt, production change or old-receipt reclassification is involved.

## Combined server r4: donor hold-to-skip integration is absent

PORT reports local combined checkout `28e3aed4` (fixtures4d895e0c + DVD610ded6e +
inputec0d1424), passed quiet receipt. Gated GameRead + game-buffer Manual stall
checks PASS (45 checks); native_dvd_host_path and native_input_stick PASS. Paired
proof is now 5/6: audio, RFL, DVD, game_read_input, startup. native_input remains
FAIL at `90 frames: the skip latches and A disappears`. The internal-16 Manual
characterization ran separately with no acceptance verdict; 16/20/30/40-frame
stalls each reported one swing in that build. All raw outcomes remain preserved
at server `port53/m1g/stale-fixtures-543-r4-combined` and
`win54/build-records/stale-fixtures-543-r4-combined`.

### Source verdict

This assertion assumes the donor SMG1 skip integration exists. It is absent in
SMG2, rather than a counter/latency error in the 90-frame machine:

- native/tests/input_tests.cpp:2376 claims WPadButton::update calls
  petari_skip_pad, but src/Game/System/WPadButton.cpp:8-32 contains no such call.
  It copies raw hold/release, computes trigger/repeat and returns.
- The test resets Skip state (:2380), enables the flag (:2381), and sets context
  (:2384). None steps the machine. native/input/skip.cpp:99-105 initializes active
  false; set_context (:110-113) only sets context; petari_skip_pad (:116-134) is
  the function that steps the machine and publishes active/progress.
- A source search finds NO petari_skip_pad call in the game. Holding A through
  90 rig frames therefore leaves active false and ordinary raw A held. This is
  not a one-frame hold-threshold offset. The independent Machine unit tests
  directly call frame and still test the exact90 boundary, but do not prove a
  game integration.
- native/STUBS.md:23-24 explicitly defers this SMG1 feature in SMG2. The
  CutsceneSkip.hpp header is absent. docs/port/HOOK_TRANSFER.md:131,173 classify
  both donor header and implementation as deferred-feature.
- native/app/CMakeLists.txt:93-98 guards the donor probe behind that missing
  header; frame_seam.cpp:975 correspondingly guards its calls. There is no hidden
  generated-source patch supplying the WPadButton hook in the input test target.

Thus this is an unsupported donor-integration expectation, not evidence that a
previously integrated SMG2 cutscene latch regressed. The ordinary Wii/SMG2 button
update is behaving as implemented. No production/test assertion is changed here,
and native_input has NOT been marked passed.

### Separate user-facing inconsistency

This does expose a product availability problem: native/input/mods.cpp:20-28
retains the SkipCutscenes name and description; the default is off, but enabling
it is allowed. native/home_menu/hbm_bridge.cpp:370-377 lists every registry mod,
and :446-448 toggles it without an SMG2-support filter. native/MODS.md also claims
the donor behavior. The UI therefore offers a feature the game does not integrate.

Recommendation for PORT's scope decision: keep the pure machine tests, label the
missing donor integration explicitly unavailable/NOT RUN rather than passing it,
and track/hide or disable the unsupported SMG2 UI feature separately. A real
cutscene-skip port would need scene-context producers and each game end path, not
only a WPadButton call. Adding just that call could hide A without ending the
cutscene and would not repair the missing feature. No new production fix, fake
fixture hook, assertion change, M1 build or native run has been made.

PORT approved the unavailable-feature reporting: pure Machine checks stay gated;
the donor body is kept behind PETARI_TEST_DONOR_SKIP (default OFF). The ordinary
native_input log emits `NOT RUN native_skip_donor_integration: UNAVAILABLE IN SMG2`
with STUBS/HOOK_TRANSFER references. The separate CTest
native_skip_donor_integration invokes `--donor-skip-integration` and returns 77,
with SKIP_RETURN_CODE=77, so it is reported SKIPPED rather than Passed. Paired
proof records this component as status NOT_RUN, executed=false, passed=false;
it credits only the checks actually executed. This is an explicit deferred
feature/backlog item, not an accepted SMG2 integration or a lowered hold threshold.

## Approved unsupported-mod repair in the combined checkout

The separate product branch `port/543-skip-unavailable` is pinned at
`fd3d120e8057695948a831d036159f2b402751a8`, off the same main. It hides the
unsupported donor toggle and refuses only its enable request, preserving stable
IDs, valid settings and startup. To include it in the combined paired proof,
append this third exact option on BOTH --prepare and --run:

```sh
--production-fix fd3d120e8057695948a831d036159f2b402751a8
```

Its registry/header, HBM bridge, shared row adapter/header and Home-menu CMake
blobs must all match that exact approved commit. The new production test is
`petari_supported_mods_tests` / `native_supported_mods`; build and run it in
addition to the DVD and circle-clamp regressions. This is still a MAIN FIXTURE
comparison on shared repaired production, never a main-production control.

The proof also invokes the separate donor CLI and requires exit77 plus exactly
one unavailable row with the source citations. Its receipt is SKIPPED / NOT_RUN,
executed=false / passed=false. Missing text, exit0, a timeout or missing citations
fails the reporting check; no feature execution is credited. Successful full
input execution must retain its own unavailable record as well.

The fixture validators are Python/source-only on the M1; native combined results
remain pending PORT's SERVER run. All r1-r4 receipts remain unchanged.

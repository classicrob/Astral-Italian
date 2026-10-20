# Native stubs inventory

The inventory below the SMG2 section is imported SMG1 reference. Its resolved
classifications and validation claims have not been established for SMG2.

## SMG2-only systems (P2 stubs)

These systems have original game sources but no validated SMG2 native integration:

| System | Source locations | Status |
| --- | --- | --- |
| Yoshi, tongue, fruit and transformations | `src/Game/Player/Yoshi*.cpp`, `src/Game/Player/MarioYoshi*.cpp`, `src/Game/MapObj/Yoshi*.cpp` | P2 |
| World map and galaxy travel | `src/Game/Map`, `src/Game/System/WorldMap*.cpp`, `src/Game/Util/WorldMapUtil.cpp` | P2 |
| Co-Star / second-player assist | `src/Game/NPC/SupportTico*.cpp`, `src/Game/Enemy/SupportTicoStop.cpp`, `src/Game/Util` | P2 |
| Cloud / Rock / Drill power-ups | `src/Game/Player` (`MarioMagic.cpp`, `MarioSMG2_*.inc`, `CloudMarioHat.cpp`) | P2 |
| Starship hub and NPC progression | `src/Game/MapObj/MarioFace*.cpp`, `src/Game/System/MarioFace*.cpp`, `src/Game/NPC` | P2 |
| SMG2 messages (LMS/MSBT) | `src/libms`, `src/Game/System/MessageSystem.cpp`, `src/Game/Screen` | P2; SMG1 BMG tests deferred |
| SMG2 stage catalogue, scenarios and smoke routes | `src/Game/System`, `src/Game/Scene`, `src/Game/Map` | P2; imported SMG1 fixtures need replacement |

**Editor deferred:** `native/editor` was deliberately not imported; editor build
targets and game links are removed from the SMG2 CMake skeleton. Native progress
and startup code no longer call editor runtime or patch-builder functions.
The SMG1 cutscene-skip probe is also deferred (P3): it is compiled and called only
when `include/Game/System/CutsceneSkip.hpp` exists. The imported SMG1
`J3DModelX::Flags` regression was removed because SMG2 has no such nested type;
SMG2 raw movement/draw words have their own checks in the native bitfield test.

## SMG2 SDK extension adapters (link preview)

`native/input/smg2_extensions.cpp` supplies SMG2's additional KPAD dependencies.
The imported `KPADInit`/`KPADRead` entry points forward to SMG2's real
`KPADInitEx(nullptr, 0)` / `KPADiRead(..., nullptr, FALSE)` implementations.
The native keyboard/mouse controller exposes no MotionPlus or Balance Board:
`KMPLSIsInit` returns false and `WPADiGetMplsStatus` reports disabled on valid
channels. They do not produce synthetic MotionPlus samples or board weights.

The following unsupported calls issue `OSPanic` naming the function and abort:
`KMPLSSetKpadRingBuffer`, `KMPLSSetSamplingCallback`, `KMPLSRead`,
`WBCSetupCalibration`, `WBCGetCalibrationStatus`, `WBCGetBatteryLevel`, `WBCRead`,
`WBCSetZEROPoint`, `WBCGetTGCWeight`, `WPADiControlMpls`, and `WPADControlBLC`.
These are P2 stubs; any game path reaching one needs an implementation before
that path can work. `native_smg2_sdk` checks absent-device queries and loud failure.

`WPADSetCallbackByKPAD` records KPAD's callback ownership and the normal WPAD
callback setters warn before overwriting it, matching the SDK's diagnostic role.
The `native_smg2_sdk` test also attempts real SMG2 KPAD startup. Its latest run
connects the native Nunchuk but fails the first report assertion: KPAD's initial
connection reset clears its buffered samples. Servicing that reset and reading
subsequent reports still needs verification; startup is not yet a passing test.

## Headless logic harness (P1/P2)

`PETARI_BUILD_HEADLESS` selects `native/app/headless/gx_null.cpp`, without Aurora,
SDL, a window, Metal or an audio output device. Shader/raster/texture draw-state
entry points are intentional no-ops, generated from the native GX declarations;
the exact preview inventory is in `native/app/headless/GX-NULL-API.md`. Unsupported
query/readback entry points call `OSPanic` and abort instead of returning fake
pixels or plausible output parameters. CPU projection, viewport/scissor,
texture metadata/size, vertex formats, display-list capture/padding, direct
FIFO bytes and platform token/draw-done callbacks have implementations.

The null processor completes on submission and reports breakpoint callbacks
immediately. It does not model GPU/FIFO stalls, command-state byte encodings,
display-list GPU execution, EFB/depth feedback or token-time snapshots. Direct
stream bytes, display-list payload bytes and discarded state calls are separate
counters. A logic replay reaching unsupported render feedback is incomplete.

Null audio still runs native AI/DSP/JAudio: the harness pulls and discards a
fixed virtual sample budget, services/drains AI interrupts and opens no device.
Fixed scheduling, original-DOL field mappings and repeated-run determinism have
not been validated. See [HARNESS.md](HARNESS.md) for exclusions, source hook
proposals, input/CSV contracts and the build queue. Only Python tests have run;
C++ configuration/builds/tests are awaiting the lead's explicit go signal.

## Imported SMG1 inventory

Native replacements that return a fixed value, do nothing, or stand in for Wii
behaviour, and whether the game depends on the real behaviour. Classes:

- **(a)** irrelevant natively, or deliberately equivalent (reason given);
- **(b)** relevant and wrong: fixed here, with a test;
- **(c)** unknown: none remain; each earlier (c) was resolved from its game call sites.

Sweep of 2026-09-30. See "Method" to repeat it.

## Fixed: (b)

| What | Wii behaviour | Native before | Fix and test |
| --- | --- | --- | --- |
| `GXInvalidateTexAll` (Aurora no-op) plus `DCStoreRange`/`DCFlushRange` (fences only) | The GPU reads texels from memory. A texture the CPU rewrites in place, stores and then invalidates shows the new texels at its next draw. | Aurora uploads once per texture object (`texObjId`, `texDataVersion`). Only `GXInitTexObj`/`GXInitTexObjData` change those, so a persistent texture rewritten in place kept its first upload. Affected: SnowFloor and SnowFloorTile (snow dug by Mario, enemies and fireballs; SnowCapsuleGalaxy), NormalMapBase `mTextures`, and Mario's dark-mask dissolve (`MarioActor::updateDarkMask`). Fur and Mii textures are written before their first load, so they were not affected. | `native/platform/os/os_cache.cpp` logs every DC store, `DCZeroRange` and locked-cache DMA by 4 KiB page with a generation (`petari_dc_stored_since`). `native/gx/patch_aurora_texture.py` makes `GXLoadTexObj` bump `texDataVersion` when the texture's pages were stored since that object's last load; Aurora then re-hashes, and re-uploads only if the texels changed. Test: `native_texture_store`. It covers stores inside and outside the texture, NoSync and zero stores, fresh objects and double buffers, all on a registered game thread with a 64 KiB current JKR heap, which catches missing `HostAllocationScope`s. Mutation checks: without the store log, 7 checks fail; without either scope, 2 checks fail, or the test aborts with `bad_alloc` as the app did at the logo. Live (build/collision-5.log, SnowCapsuleGalaxy scenario 1 stage smoke, PASS): the four SnowFloor textures (128x128 I8, two floors each double-buffered) reload as the snow is rewritten, and no other texture does. Each loaded texture object is remembered for 10 idle minutes, longer than Aurora keeps an unused upload (600 frames at any rate of 1 fps or more), so an object seen for the first time never has a stale upload. |

The asm-equivalence audit found the other (b)s: native_math_util,
native_wii_asm_equivalence, and native/RELIABILITY.md.

## Platform shims (`native/`), linked into the app

Every C-linkage SDK symbol the app links was resolved to its defining object with
a link map (`-Wl,-map`). The short or constant ones are below. Everything else
has a real implementation; most are described in native/platform/STATUS.md.
Unsupported paths call `OSPanic` loudly instead of returning success
(`__DSP_exec_task`, `__DSP_remove_task`, `OSFillFPUContext`, `OSProtectRange`,
`PPCHalt`, `GXSetMisc(GX_MT_ABORT_WAIT_COPYOUT)`).

| Function | Native | Class | Why |
| --- | --- | --- | --- |
| `DCEnable`, `ICEnable`, `ICFlashInvalidate`, `LCEnable`/`LCDisable` state, `LCQueueWait` | nothing / fence | (a) | Host caches are coherent and nothing is generated at run time. Locked-cache DMA completes synchronously. |
| `DCInvalidateRange`, `DCInvalidate` | acquire fence | (a) | They pair with the producer's store fence. |
| `OSGetStackPointer` | 0 | (a) | Used only by JUTException's crash dump (native crash reporting is os_crash.cpp) and by SDK files that aren't compiled natively. |
| `VIGetDTVStatus` | 1 | (a) | Reports a progressive-capable display. `RenderMode.cpp:311` then honours the progressive setting, which is the intended Mac output. |
| `WPADGetWorkMemorySize` | 0 | (a) | There is no Bluetooth stack. HeapMemoryWatcher adds its fixed overhead to 0 (native/input/README.md). |
| `DSPCheckMailToDSP`, `DSPAssertInt` | 0 / no-op | (a) | The DSP device handles mail synchronously, so JAudio's "wait until the mailbox is empty" loops fall through. |
| `ESP_InitLib`, `ESP_CloseLib` | 0 | (a) | No ticket services are needed. `ESP_GetDataDir` is implemented (save paths). |
| `NWC24*` (all message calls) | `NWC24OpenLib` returns `DISABLED`; others return `NOT_READY`; `GetErrorCode` returns 0 | (a) | Models a console with WiiConnect24 turned off; the service is discontinued. NWC24Messenger handles `DISABLED`: background letters such as the staff/Luigi mail are dropped, and foreground sends show the game's WiiConnect24 notice. Tests: platform_nwc24_tests (11). Product note: if a foreground send exists on the story path, that notice appears as on a WC24-off Wii. |
| `DVDCloseDir` | TRUE | (a) | Directory handles hold no host resources. |
| default reset/power callbacks | empty | (a) | These match the SDK defaults; the game installs its own. |
| `JKRSolidHeap::do_free`, `JSU*Stream` destructors | empty | (a) | They are empty on the Wii too. |
| `OSInit` (libpetari_boot) | real native boot | (a) | This is the native entry. |
| HOME Button Menu `HBM*RSO` | native menu | (a) | The RSO module is PowerPC code; native/home_menu serves the entry points. |

## Aurora (third party), linked no-ops

| Function | Class | Why |
| --- | --- | --- |
| `GXSetZTexture` | (b), worked around | Z-texture replace is not implemented. The two clears that use it (`MainLoopFramework` and `DrawUtil`) draw on the orthographic far plane natively instead (PLAYTEST.md "Depth clears"). The other callers (`GXInit`, `ut_CharWriter`, `ImageEffectLocalUtil`) only disable it. |
| `GXInvalidateTexAll` | (b), fixed | See the fix above. |
| `GXInitFifoBase` | (a) | The FIFO is owned by Aurora and the GX sync layer (docs/dev/GX_SYNC_PLAN.md). |
| `THPInit` returns TRUE | (a) | The decoder needs no setup. |

The display-copy setters (`GXSetDispCopy*`, `GXCopyDisp`, `GXSetCopyFilter`)
are stubs in upstream Aurora but real in the patched copy
(patch_aurora_present.py).

## `GALAXY_NATIVE` branches in `src/` and `libs/`

Every branch whose native side is empty, one statement, or a constant was
listed and read. Most replace Wii asm or layout code with equivalents; the
asm cases are covered by native_wii_asm_equivalence. The ones that change
behaviour:

| Where | Native | Class | Why |
| --- | --- | --- | --- |
| `LayoutManager::getIndexOfPane` | -1 for a missing pane (the Wii returns 0, the root) | (a), hardening | Every caller either guards against -1 or first checks `getPane` on the same tree. Misses are reported (asset_diagnostics), and smoke PASS requires zero (build/asset-reference-report.md). |
| `J3DAnmLoader` | rejects malformed or unsupported (vertex-color) animation files with an OSReport | (a) | The disc has none: all 4753 animations load and evaluate (petari_j3d_anim_load_tests --assets: bck 3232, bpk 55, brk 503, btk 383, btp 422, bva 158; 0 failed). |
| `osdsp_task` | skips `OSClearContext`/`OSSetCurrentContext` around the DSP callback | (a) | There are no PowerPC contexts natively. |
| `db_assert` stack walk | omitted | (a) | Debug output only. |
| `KinopioAstro.cpp` `JKRArchive::getExpandedResSize`, `RFL_System.c` `RFLGetLastReason`, `MessageEditorMessageTag` | Wii copy compiled out | (a) | Each has a native definition elsewhere with the same body (`JKRArchivePri.cpp`, the other branch, `MessageEditorMessageTagNative.cpp`). |

No `GALAXY_NATIVE` block returns unconditionally before game logic, except two
complete native replacements (`JKRAramPiece::startDMA` and
`ResFont::SetResource`).

## Method

1. `python3 native/tools/stub_scan.py native` flags empty, constant, `(void)`-only
   and marked bodies (constructors appear too and are skipped by eye). The same
   scan was run on the Aurora Dolphin layer and on the patched Aurora copies in
   `build/macos-gx/native/gx`.
2. Relink the app with `-Wl,-map,<file>` and resolve every C-linkage SDK symbol
   to its defining object, so nothing is missed because it lives outside
   `native/`: the game's own HBM wrapper, Aurora, RVL SDK files compiled
   natively, and test-only stubs (none are linked).
3. List `GALAXY_NATIVE` branches in `src/` and `libs/` whose native side is
   short, empty or constant, and native-only blocks that return unconditionally.
4. For each candidate, read the game's call sites and decide (a), (b) or (c).

Not covered: functions with real but possibly incomplete bodies, such as the
defaults in SC settings or WPAD extension handling. Those are behaviour audits,
not stubs.

## Headless native presentation annotations

`petari_gx_pipeline_stage_begin` discards/counts renderer pipeline preparation;
there is no renderer or pipeline database in this target.
`petari_gx_waiting_for_pipeline` returns false because that pipeline worker does
not exist. `petari_progress_badge` and `petari_progress_star_get` discard/count
the native personal-progress overlay notifications. These hooks have no game
progress writes or game-state return values; the harness does not qualify the
native personal-progress display/record. The real game save/progress logic
still runs against isolated NAND. Implementations live only in the headless
GX backend; the normal app retains its actual renderer/progress providers.
`petari_gx_sync_held_ticket` uses the existing native GX sync adapter, including
its real held-ticket state, rather than a constant stub.

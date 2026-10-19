# Native link preview — Sol A

Snapshot: 2026-10-19, before the hook-transfer redo. Normal worktree configuration based on hook commit `0368a27b`, plus the native adaptations listed in [FILES-LINK.txt](FILES-LINK.txt). The current checkout may already have newer lead commits. This is a diagnostic snapshot, not a working executable or a claim that the current redo has these exact failures.

**The normal app build stops before linking.** It produced 1,538 of 1,844 game objects. A separate strict link over the successfully compiled inputs finished with **2,965 distinct unresolved Mach-O symbols**. Most remaining failures are game/library providers, not new hardware APIs.

All builds and the diagnostic link occurred before the pause. After the lead reserved the machine for Wii builds, this report used only existing logs, `nm`/`c++filt`, source reads, and documentation generation. No CMake, clang, linker, or test executable was run during the pause. Source reads during this analysis used the stable `ht2/overlay` or the read-only donor, rather than the fluctuating worktree `src/include/libs`.

## Method and limits

- `build/macos-arm64` configured normally. The component keep-going build exposed native SDK API differences and source/header blockers.
- `build/macos-gx` configured the normal app with GX enabled, arm64, Debug, ASan/UBSan, and cached Aurora/SDL/Dawn sources. Exact configure arguments: [configure-main-gx-command.json](configure-main-gx-command.json).
- The normal `nice -n 10 cmake --build build/macos-gx -j 6 --target petari -- -k` stopped on compile/dependency failures: [build-main-petari-initial.log](build-main-petari-initial.log).
- Generated per-target Make rules were then used to attempt source/library inputs hidden behind failed dependencies. Their results are in [link-input-builds.json](link-input-builds.json). These attempts do not establish that whole targets built.
- [partial_link.py](partial_link.py) starts from CMake's actual `petari` link command and keeps its frameworks, third-party archives, sanitizer flags, and pinned Aurora symbols. It omits absent objects and expands each absent archive into whatever objects for that target actually compiled. It also removes `-dead_strip` and forces `_petari_game_main` to expose a broader inventory. Existing archives retain normal lazy extraction.
- Result: 1,710 direct object inputs plus 33 distinct existing archives. There were 321 absent original input entries, including repeated archives. See [partial-link-manifest.json](partial-link-manifest.json), [partial-link-args.json](partial-link-args.json), and [partial-link.log](partial-link.log). The resulting link failed; no runnable preview was produced.
- `nm -A -P -g` covered all 1,743 distinct object/archive input files. Every linker entry mapped to raw names. The 2,965 entries have 2,962 unique demangled displays because constructor ABI variants share displays. Counts below use raw names, not demangled display names.
- Reference counts below mean distinct input object locations, not source calls or relocations. Archive reference counts include unselected members, but the reported undefined set itself comes from the strict linker output. This is not the unresolved set of a hypothetical fully compiled app with dead stripping enabled.

## Counts by origin

| Class | Unresolved raw symbols | Meaning |
| --- | ---: | --- |
| SMG2 game code | 2,696 | Missing compiled providers, missing source definitions, or an ABI/declaration mismatch; not 2,696 separate compile errors |
| JSystem / nw4r / heap operators | 213 | 178 JSystem/allocator symbols and 35 nw4r symbols; not all are SMG2 additions |
| SDK with an existing provider absent from this link | 41 | ARAM 6, DSP 11, MEM 2, OS arena/boot 4, RFL 18 |
| SDK native implementation still missing | 1 | `OSReturnToDataManager` |
| Native app integration | 14 | Unbuilt bridges, game entry, and imported SMG1 feature probes |
| **Total** | **2,965** | Exhaustive for this diagnostic snapshot |

The name/signature adaptation class has **zero remaining SDK undefined symbols in this snapshot**, after adding the two legacy KPAD forwarding entry points. Fourteen additional input dependencies also resolve through native adapters, including eleven explicitly loud unsupported operations; resolution does not mean those devices are supported.

Full per-symbol names, reference counts, and raw ABI spellings: [UNDEFINED-SYMBOLS.md](UNDEFINED-SYMBOLS.md). Machine-readable data: [link-classification.json](link-classification.json), [undefined-inventory.json](undefined-inventory.json). Exact SDK declarations, provider names, and donor equivalents: [SDK-APIS.md](SDK-APIS.md).

## Highest-impact game providers

| Top unresolved symbol | Referencing objects | Provider finding |
| --- | ---: | --- |
| `NameObj::endMovement()` | 873 | Defined in `src/Game/Util/FootPrint.cpp`, not NameObj.cpp; FootPrint did not compile |
| `NameObj::startMovement()` | 873 | Same FootPrint provider |
| `MR::isFirstStep(LiveActor const*)` | 388 | `src/Game/Util/LiveActorUtil.cpp` did not compile |
| `MR::startAction(LiveActor const*, char const*)` | 367 | `src/Game/Util/ActorAnimUtil.cpp` did not compile |
| `MR::invalidateClipping(LiveActor*)` | 334 | LiveActorUtil |
| `MR::isActionEnd(LiveActor const*)` | 239 | ActorAnimUtil |
| `MR::isGreaterStep(LiveActor const*, int)` | 177 | Game utility provider absent |
| `MR::isStep(LiveActor const*, int)` | 172 | Game utility provider absent |
| `MR::isDead(LiveActor const*)` | 148 | Game utility provider absent |
| `MR::setPosition(LiveActor*, TVec3f const&)` | 138 | Game utility provider absent |
| `MR::validateClipping(LiveActor*)` | 128 | LiveActorUtil |
| `ActorStateBaseInterface::update()` | 124 | Declared in the SMG2 header, no definition found in overlay source; donor has `src/Game/LiveActor/ActorStateBase.cpp` |
| `MR::hideModel(LiveActor*)` | 123 | Game utility provider absent |
| `MR::zeroVelocity(LiveActor*)` | 115 | Game utility provider absent |
| `MR::getSceneObjHolder()` | 109 | SceneObjHolder provider did not compile |
| `SceneObjHolder::getObj(int) const` | 109 | `src/Game/Scene/SceneObjHolder.cpp` did not compile |
| `vtable for ActorStateBaseInterface` | 103 | The missing out-of-line `update` is its key-function provider |

FootPrint's first recorded error is the explicit `TVec3<float>::negate` instantiation at line 24. Broad game failures also include JAudio header errors (`JASGadget.hpp:74`, dependent-base lookup in `JAUBankTable.hpp:45`, and `JAUSectionHeap.hpp:128`); these stop many unrelated game providers. GameSystem additionally hits missing `TRACE_BOOT` and `PETARI_FRAME_MARK_END_FRAME` declarations.

One game ABI issue is distinguishable from a missing object: `float MR::sqrt<float>(float)` has an unresolved raw name `__ZN2MR4sqrtIfEEff`, while existing template instantiations define `__ZN2MR4sqrtIfEET_S1_`. They demangle identically. `CameraViewInterpolator.cpp` defines `MR_SQRT_F32_OUTLINE`, selecting an explicit-specialization declaration in MathUtil.hpp. Treat that declaration/definition pair separately from ordinary missing template instances.

## JSystem and nw4r

Top library references are `operator new(size_t, JKRHeap*, int)` (150 objects), `operator new[](size_t, int)` (23), `operator new[](size_t, JKRHeap*, int)` (20), `JKRHeap::sSystemHeap` (14), and `JKRHeap::sCurrentHeap` (11). `JSUMemoryInputStream::setBuffer`, `JSUMemoryOutputStream::setBuffer`, and both stream vtables each occur in eight objects. J3DFrameCtrl methods, JASTrack, JPA resource/field operations, and nw4r layout/material/text writing follow.

| Target | Objects built / configured | First remaining blockers |
| --- | ---: | --- |
| `petari_heaps` | 5 / 8 | JKRHeap/JKRExpHeap/JKRUnitHeap; pointer-to-32-bit casts |
| `petari_kernel` | 18 / 22 | JKRAramPiece, JKRAramArchive, JKRMemArchive, JKRDecomp pointer casts |
| `petari_j3d` | 31 / 34 | J3DAnimation missing `petari_generic_world_grey_animation`; J3DAnmLoader casts; JUTAssert `__va_list` |
| `petari_game_libraries` | 101 / 142 | JAudio headers; J2DPane `DEG_TO_RAD`; JSUMemoryStream pointer casts; nw4r native helper declarations / wchar headers; particle Overwrite.cpp casts |

The 35 nw4r symbols include `AnimResource`, `AnimPaneTree`, animation-list operations, Material accessors, and `TextWriterBase<char16_t>`. `lyt_animation.cpp`, `lyt_material.cpp`, and `ut_TextWriterBase.cpp` failed to compile. Several similarly named APIs already exist in the modified donor; the total must not be presented as 35 new SMG2 APIs. The pending hook-transfer redo may restore donor fixes for these.

SMG2 LMS/MSBT sources were added to the native game-library source list: four `src/libms/*.c` units, compiled as C++ with generated UTF-16 inputs. All four currently stop at `<wchar_t.h>` in `libs/libms/include/libms.h`. Their absence can hide further calls, so this snapshot is not an exhaustive LMS or new-SDK dependency inventory.

`JUTWarningConsole_f` is also missing. The overlay and donor expose `JUTWarningConsole_f_va`, while test-only heap diagnostics define the variadic name. A real forwarding implementation can be considered after the build reservation; do not link test diagnostics into the app to conceal it.

## SDK differences and existing providers

The **one remaining native API gap** is:

```c
void OSReturnToDataManager(void);
```

Header: overlay `libs/RVL_SDK/include/revolution/os/OSReset.h:44`. SMG2 calls it from `GameSystemResetAndPowerProcess::exitApplication`. The donor's `native/platform/STATUS.md` explicitly lists it as unimplemented. `OSReturnToMenu(void)` in `native/platform/os/os_power.cpp` is a nearby host exit operation, not an exact Data Manager implementation. A native implementation should deliberately record/handle this exit intent or fail loudly; no implementation was added during the build pause.

The 41 other SDK gaps have providers already: ARAM in `native/platform/aram/aram_host.cpp`; DSP mailbox/state in `native/platform/audio/dsp_device.cpp`; MEM allocator in SDK mem sources; MEM2 arena functions in `native/memory/arena.cpp`; OSInit in `native/memory/boot.cpp`; RFL in its 13 source units. Some had not been attempted because dependencies failed. Do not count these as 41 missing SDK implementations.

`RFLGetAdditionalInfoWithSanitizedNames` is a useful changed-name example: SMG2 already supplies the wrapper in `RFL_DataUtility.c`, calling its `RFLGetAdditionalInfo(..., TRUE)` path. The donor has `RFLGetAdditionalInfo`, but the SMG2 wrapper/sanitization argument is additional. Its provider is compile-blocked by the SDK MEM header, so adding a native alias would duplicate existing game-library behavior.

Before the pause, native input gained:

- `KPADInit` / `KPADRead` forwarding to SMG2 `KPADInitEx(nullptr, 0)` / `KPADiRead(..., nullptr, FALSE)`.
- `KMPLSIsInit` and `WPADiGetMplsStatus` absent-device queries, plus callback ownership handling through `WPADSetCallbackByKPAD`.
- Eleven `OSPanic` + abort operations for unavailable MotionPlus/Balance Board paths, listed in native/STUBS.md and SDK-APIS.md. They are P2 work, not simulated successful input.

No unresolved SC/NAND/CNT/ARC/HBM names appear in the current snapshot. More may become visible when missing callers compile.

## Native app integration

Fourteen unresolved symbols divide into three groups:

- Eight names have unbuilt providers: physical-address conversion (2, arena.cpp), VI attach (1), AudioSDL install/counter (2), platform audio counter (1), and frame phase/pause state (2). `petari_app_platform` compiled **8/9** units; **frame_phase.cpp**, not input_script_probe.cpp, is the missing unit, due to JAudio headers. VI/audio providers were not reached by the normal build.
- `_petari_game_main` exists in the hooked GameSystem.cpp but that game unit failed to compile.
- Five imported SMG1 feature names have no corresponding compiled SMG2 feature: `_petari_camera_orbit_active` and `_petari_cappy_{bounces,collected,hits,state}`. Guard/defer those native camera/smoke feature paths or port the feature deliberately. Zero-return symbol fillers would misrepresent the smoke result.

All seven app executable units compiled after guarding deferred editor includes/initialization and removing editor_m4_smoke.cpp. That is object compilation only, not a complete app build.

## Validation and next steps after “go”

Latest recorded selected tests: **native_core PASS** (13,187 checks), **native_input_remap PASS** (77 checks), **native_smg2_sdk FAIL**. See [ctest-link-adapters.log](ctest-link-adapters.log). The new test's absent-device and abort checks run before its failing KPAD report assertion.

Source analysis explains the KPAD failure: after connection, the first `KPADiRead` sees `resetReq`, calls `reset_kpad`, and clears `bufCount` (`KPAD.c:1422`, reset body line 220). The test pumps 80 reports and expects that first read to return reports. It must service the initial reset, pump subsequent reports, then assert the next read. This is a diagnosis, not a verified fix. No test binary was rerun during the pause.

Three unapplied source/header patch proposals already exist in [patches/](patches/): native-only `__MEM2End` declaration, C-compatible `<stddef.h>` inside SDK MEM's extern-C region, and native UTF-16 include for libms. They were `git apply --check` clean against the earlier hooked snapshot; re-check after the redo. No `src/include/libs` changes were made by this lane.

After explicit permission to resume builds: use `nice -n 10`, at most `-j 6`, and check the 40 GiB disk floor. Reconfigure/regenerate against the completed hook redo, repair the KPAD regression and remaining native integration/API gaps, then rebuild the small provider targets before attempting the full app again. Generate a fresh inventory; this report's 306 missing game objects and 2,965 unresolved names should not be reused as current-state counts after that rebuild.

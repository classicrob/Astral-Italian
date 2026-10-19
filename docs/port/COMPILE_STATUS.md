# Native compilation baseline

Audited 2026-10-19 on arm64 macOS with Apple Clang 21.0.0 (clang-2100.3.34.2).
The primary audit ran 16:27:57–16:28:33 EDT. This is the initial SMG2 checkout
baseline before the requested transferred-hook overlay pass. These are C++17
syntax checks with `GALAXY_NATIVE=1`, `-ferror-limit=5`, `-Werror=return-type`,
and no RTTI; passing does not establish linking or runtime behavior. The audit
uses the copied native compatibility headers and generated Shift-JIS/UTF-16
sources under `build/audit-sjis`, with diagnostics mapped back to original files.
No game or library sources were changed for this audit.

## Reproduce

From `/Users/roberthaisfield/MacioGalaxy2-wt/port`:

```sh
nice -n 10 python3 native/tools/audit_compile.py --jobs 6 --output build/native-audit-baseline.json src/Game src/JSystem src/nw4r
nice -n 10 python3 native/tools/audit_compile.py --jobs 6 --output build/native-audit-rvl-sdk.json src/RVL_SDK
nice -n 10 python3 native/tools/audit_compile.py --jobs 6 --output build/native-audit-libms.json src/libms
python3 tools/port/classify_audit.py build/native-audit-baseline.json build/native-audit-rvl-sdk.json build/native-audit-libms.json --output build/native-audit-classified.md --json-output build/native-audit-classified.json
```

Audit exit status 1 indicates failing translation units; reports were written
successfully. Raw JSON and classified artifacts are also preserved under
`~/MacioGalaxy2-team/scratch-port/solb/`.

## Totals and scope

| Source group | Checked | Passing | Failing |
| --- | ---: | ---: | ---: |
| Game C++ | 1844 | 121 | 1723 |
| JSystem C++ | 170 | 56 | 114 |
| nw4r C++ | 29 | 11 | 18 |
| Primary baseline | 2043 | 188 | 1855 |
| RVL_SDK C++ | 1 | 0 | 1 |
| libms C++ | 0 | 0 | 0 |
| Combined C++ | 2044 | 188 | 1856 |

The tool discovers `.cpp` files only. RVL_SDK's 301 `.c` files and libms's four
`.c` files were not checked; the zero-file libms row is not a successful syntax
validation. RVLFaceLib, Runtime, MSL_C, and MetroTRK are outside this pass.

The read-only SMG1 reference reports **1605/1605 Game** and **200/200
JSystem/nw4r** passing as of 2026-09-28 after its native hooks (see
`~/MacioGalaxy/native/COMPILE_STATUS.md`). SMG2's Game result is **121/1844**
(6.6%). Different source inventories make these counts a readiness comparison,
not a file-for-file regression test.

## P1 directory worklist

Ownership follows the Sol B brief. Library and new-unit work belongs to m1;
unlisted Game directories are unassigned. Counts use each translation unit's
first error. Unknown symbols are named in the ranked errors below and in each
failing file's `first_error.symbol` in the classified JSON.

Checked **2044**; passing **188**; failing **1856**.

| Directory | Lane | Checked | Passing | Failing | Top 3 first-error classes |
| --- | --- | --- | --- | --- | --- |
| src/Game/Animation | core | 11 | 4 | 7 | unknown type / identifier: 4; inline asm / asm function: 1; template: 1 |
| src/Game/AreaObj | objects | 39 | 0 | 39 | unknown type / identifier: 39 |
| src/Game/AudioLib | core | 43 | 7 | 36 | other: 23; unknown type / identifier: 8; ambiguous / overload: 5 |
| src/Game/Boss | objects | 166 | 0 | 166 | unknown type / identifier: 165; other: 1 |
| src/Game/Camera | core | 126 | 4 | 122 | unknown type / identifier: 122 |
| src/Game/Demo | core | 39 | 8 | 31 | unknown type / identifier: 29; ambiguous / overload: 1; template: 1 |
| src/Game/Effect | core | 29 | 8 | 21 | unknown type / identifier: 16; other: 3; ambiguous / overload: 2 |
| src/Game/Enemy | enemies | 158 | 1 | 157 | unknown type / identifier: 154; other: 2; ambiguous / overload: 1 |
| src/Game/GameAudio | core | 7 | 2 | 5 | other: 4; unknown type / identifier: 1 |
| src/Game/Gravity | core | 16 | 0 | 16 | unknown type / identifier: 16 |
| src/Game/LiveActor | core | 80 | 7 | 73 | unknown type / identifier: 71; other: 2 |
| src/Game/Map | core | 71 | 9 | 62 | unknown type / identifier: 62 |
| src/Game/MapObj | objects | 333 | 6 | 327 | unknown type / identifier: 326; other: 1 |
| src/Game/NPC | enemies | 92 | 1 | 91 | unknown type / identifier: 91 |
| src/Game/NWC24 | core | 7 | 1 | 6 | unknown type / identifier: 5; ambiguous / overload: 1 |
| src/Game/NameObj | core | 13 | 8 | 5 | unknown type / identifier: 5 |
| src/Game/Player | astra | 116 | 10 | 106 | unknown type / identifier: 105; template: 1 |
| src/Game/RhythmLib | enemies | 15 | 0 | 15 | other: 7; unknown type / identifier: 6; pointer to integer (64-bit hazard): 2 |
| src/Game/Ride | objects | 33 | 0 | 33 | unknown type / identifier: 33 |
| src/Game/Scene | core | 23 | 3 | 20 | unknown type / identifier: 17; other: 3 |
| src/Game/Screen | enemies | 140 | 3 | 137 | unknown type / identifier: 132; inline asm / asm function: 4; template: 1 |
| src/Game/Speaker | core | 7 | 3 | 4 | other: 4 |
| src/Game/System | core | 164 | 27 | 137 | unknown type / identifier: 99; template: 12; pointer to integer (64-bit hazard): 11 |
| src/Game/Util | core | 116 | 9 | 107 | unknown type / identifier: 96; other: 4; template: 4 |
| src/JSystem/J2DGraph | m1 | 7 | 3 | 4 | unknown type / identifier: 4 |
| src/JSystem/J3DGraphAnimator | m1 | 11 | 1 | 10 | unknown type / identifier: 10 |
| src/JSystem/J3DGraphBase | m1 | 13 | 3 | 10 | unknown type / identifier: 8; other: 1; ambiguous / overload: 1 |
| src/JSystem/J3DGraphLoader | m1 | 7 | 0 | 7 | unknown type / identifier: 5; pointer to integer (64-bit hazard): 1; ambiguous / overload: 1 |
| src/JSystem/JAudio2 | m1 | 72 | 24 | 48 | other: 43; pointer to integer (64-bit hazard): 4; unknown type / identifier: 1 |
| src/JSystem/JGadget | m1 | 2 | 2 | 0 | — |
| src/JSystem/JKernel | m1 | 23 | 7 | 16 | ambiguous / overload: 12; pointer to integer (64-bit hazard): 3; other: 1 |
| src/JSystem/JMath | m1 | 3 | 2 | 1 | template: 1 |
| src/JSystem/JParticle | m1 | 16 | 3 | 13 | unknown type / identifier: 12; ambiguous / overload: 1 |
| src/JSystem/JSupport | m1 | 5 | 4 | 1 | pointer to integer (64-bit hazard): 1 |
| src/JSystem/JUtility | m1 | 11 | 7 | 4 | unknown type / identifier: 2; ambiguous / overload: 2 |
| src/RVL_SDK | m1 | 1 | 0 | 1 | MWCC-only syntax: 1 |
| src/libms | m1 | 0 | 0 | 0 | — |
| src/nw4r/db | m1 | 3 | 0 | 3 | inline asm / asm function: 2; template: 1 |
| src/nw4r/lyt | m1 | 14 | 3 | 11 | pointer to integer (64-bit hazard): 8; inline asm / asm function: 3 |
| src/nw4r/math | m1 | 2 | 1 | 1 | inline asm / asm function: 1 |
| src/nw4r/ut | m1 | 10 | 7 | 3 | pointer to integer (64-bit hazard): 1; inline asm / asm function: 1; missing header: 1 |

## Ranked root causes

Counts group each file's first error by diagnostic location and normalized message. Later errors are masked; these counts do not establish how many files a fix will make pass.

| Rank | Files | Header/source:line | First error | Class |
| ---: | ---: | --- | --- | --- |
| 1 | 1638 | libs/JSystem/include/JSystem/JGeometry/TMatrix.hpp:793 | use of undeclared identifier 'get' | unknown type / identifier |
| 2 | 83 | libs/JSystem/include/JSystem/JAudio2/JASHeapCtrl.hpp:296 | 'operator new' takes type size_t ('unsigned long') as 1st parameter | other |
| 3 | 26 | libs/JSystem/include/JSystem/JKernel/JKRDvdRipper.hpp:53 | no matching function for call to 'loadToMainRAM' | ambiguous / overload |
| 4 | 18 | libs/RVL_SDK/include/revolution/mem/heapCommon.h:37 | cast from pointer to smaller type 'UIntPtr' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 5 | 16 | /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/c++/v1/__type_traits/enable_if.h:20 | templates must have C++ linkage | template |
| 6 | 12 | libs/nw4r/include/nw4r/math/arithmetic.h:10 | invalid output constraint '=f' in asm | inline asm / asm function |
| 7 | 12 | libs/JSystem/include/JSystem/JAudio2/JASGadget.hpp:74 | member initializer 'JASPtrTable' does not name a non-static data member or base class | other |
| 8 | 5 | libs/JSystem/include/JSystem/JAudio2/JASSeqReader.hpp:35 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 9 | 3 | /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/c++/v1/__cstddef/byte.h:57 | templates must have C++ linkage | template |
| 10 | 2 | libs/JSystem/include/JSystem/J3DGraphBase/J3DDrawBuffer.hpp:34 | non-void function does not return a value [-Werror,-Wreturn-type] | other |
| 11 | 2 | libs/JSystem/include/JSystem/JUtility/JUTAssert.hpp:17 | unknown type name '__va_list'; did you mean 'va_list'? | unknown type / identifier |
| 12 | 1 | src/Game/Animation/XanimeResourceTable.cpp:351 | cannot jump from switch statement to this case label | other |
| 13 | 1 | src/Game/AudioLib/AudEffector.cpp:34 | no matching function for call to 'operator new[]' | ambiguous / overload |
| 14 | 1 | src/Game/Demo/DemoStartRequestHolder.cpp:193 | explicit specialization of 'push_back' after instantiation | template |
| 15 | 1 | src/Game/Effect/EffectObj10x10x10SyncClipping.cpp:9 | taking the address of a temporary object of type 'TVec3f' (aka 'TVec3<float>') [-Waddress-of-temporary] | other |
| 16 | 1 | src/Game/Effect/EffectObj20x20x10SyncClipping.cpp:11 | taking the address of a temporary object of type 'TVec3f' (aka 'TVec3<float>') [-Waddress-of-temporary] | other |
| 17 | 1 | src/Game/Effect/EffectObj50x50x10SyncClipping.cpp:9 | taking the address of a temporary object of type 'TVec3f' (aka 'TVec3<float>') [-Waddress-of-temporary] | other |
| 18 | 1 | include/Game/Util/Functor.hpp:27 | no matching function for call to 'operator new' | ambiguous / overload |
| 19 | 1 | src/Game/Map/AwakeControllerHolder.cpp:42 | no member named 'mem_func' in namespace 'std' | unknown type / identifier |
| 20 | 1 | src/Game/NWC24/NWC24System.cpp:22 | no matching function for call to 'operator new[]' | ambiguous / overload |
| 21 | 1 | src/Game/System/DrawBufferExecuter.cpp:47 | no member named 'mem_func' in namespace 'std' | unknown type / identifier |
| 22 | 1 | src/Game/System/DrawSyncManager.cpp:81 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 23 | 1 | libs/libms/include/libms.h:5 | 'wchar_t.h' file not found | missing header |
| 24 | 1 | src/Game/System/OSThreadWrapper.cpp:29 | no matching function for call to 'operator new[]' | ambiguous / overload |
| 25 | 1 | src/Game/Util/DemoUtil.cpp:4 | explicit instantiation refers to member function 'JGeometry::TVec3<float>::TVec3' that is not an instantiation | template |
| 26 | 1 | src/Game/Util/JMapInfo.cpp:64 | cannot jump from switch statement to this case label | other |
| 27 | 1 | src/JSystem/J2DGraph/J2DManage.cpp:6 | use of undeclared identifier 'strcmp' | unknown type / identifier |
| 28 | 1 | src/JSystem/J2DGraph/J2DPane.cpp:213 | use of undeclared identifier 'DEG_TO_RAD' | unknown type / identifier |
| 29 | 1 | src/JSystem/J3DGraphBase/J3DShapeDraw.cpp:27 | no matching function for call to 'operator new[]' | ambiguous / overload |
| 30 | 1 | src/JSystem/J3DGraphLoader/J3DAnmLoader.cpp:230 | cast from pointer to smaller type 's32' (aka 'int') loses information | pointer to integer (64-bit hazard) |
| 31 | 1 | src/JSystem/J3DGraphLoader/J3DShapeFactory.cpp:102 | no matching function for call to 'operator new[]' | ambiguous / overload |
| 32 | 1 | src/JSystem/JAudio2/JASCalc.cpp:71 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 33 | 1 | src/JSystem/JAudio2/JAUSeqDataBlockMgr.cpp:136 | cannot initialize a parameter of type 'void (*)(u32, u32)' (aka 'void (*)(unsigned int, unsigned int)') with an lvalue of type 'void (u32, uintptr_t)' (aka 'void (unsigned int, unsigned long)'): type mismatch at 2nd parameter ('u32' (aka 'unsigned int') vs 'uintptr_t' (aka 'unsigned long')) | other |
| 34 | 1 | src/JSystem/JKernel/JKRAramBlock.cpp:30 | no matching function for call to 'operator new' | ambiguous / overload |
| 35 | 1 | src/JSystem/JKernel/JKRAramHeap.cpp:15 | no matching function for call to 'operator new' | ambiguous / overload |
| 36 | 1 | src/JSystem/JKernel/JKRAramStream.cpp:13 | no matching function for call to 'operator new' | ambiguous / overload |
| 37 | 1 | src/JSystem/JKernel/JKRArchivePub.cpp:10 | out-of-line definition of 'check_mount_already' does not match any declaration in 'JKRArchive' | other |
| 38 | 1 | src/JSystem/JKernel/JKRDecomp.cpp:12 | no matching function for call to 'operator new' | ambiguous / overload |
| 39 | 1 | src/JSystem/JKernel/JKRExpHeap.cpp:86 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 40 | 1 | src/JSystem/JKernel/JKRHeap.cpp:78 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 41 | 1 | src/JSystem/JKernel/JKRUnitHeap.cpp:41 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 42 | 1 | src/JSystem/JMath/JMATrigonometric.cpp:6 | explicit specialization of 'atan2_' after instantiation | template |
| 43 | 1 | src/JSystem/JParticle/JPAResourceLoader.cpp:31 | no matching function for call to 'operator new[]' | ambiguous / overload |
| 44 | 1 | src/JSystem/JSupport/JSUMemoryStream.cpp:16 | cast from pointer to smaller type 'int' loses information | pointer to integer (64-bit hazard) |
| 45 | 1 | src/JSystem/JUtility/JUTException.cpp:81 | no matching function for call to 'operator new' | ambiguous / overload |
| 46 | 1 | src/JSystem/JUtility/JUTTexture.cpp:9 | no matching function for call to 'operator new[]' | ambiguous / overload |
| 47 | 1 | src/nw4r/lyt/lyt_texMap.cpp:73 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | pointer to integer (64-bit hazard) |
| 48 | 1 | src/nw4r/ut/ut_ResFont.cpp:9 | cast from pointer to smaller type 's32' (aka 'int') loses information | pointer to integer (64-bit hazard) |
| 49 | 1 | src/nw4r/ut/ut_TextWriterBase.cpp:2 | 'wchar_io.h' file not found | missing header |
| 50 | 1 | src/RVL_SDK/os/init/__ppc_eabi_init.cpp:11 | '__declspec' attributes are not enabled; use '-fdeclspec' or '-fms-extensions' to enable support for __declspec attributes | MWCC-only syntax |

## Top 20 distinct first-error messages

| Rank | Files | Normalized message |
| ---: | ---: | --- |
| 1 | 1638 | use of undeclared identifier 'get' |
| 2 | 83 | 'operator new' takes type size_t ('unsigned long') as 1st parameter |
| 3 | 26 | no matching function for call to 'loadToMainRAM' |
| 4 | 19 | templates must have C++ linkage |
| 5 | 18 | cast from pointer to smaller type 'UIntPtr' (aka 'unsigned int') loses information |
| 6 | 12 | invalid output constraint '=f' in asm |
| 7 | 12 | member initializer 'JASPtrTable' does not name a non-static data member or base class |
| 8 | 11 | cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information |
| 9 | 7 | no matching function for call to 'operator new[]' |
| 10 | 6 | no matching function for call to 'operator new' |
| 11 | 3 | taking the address of a temporary object of type 'TVec3f' (aka 'TVec3<float>') [-Waddress-of-temporary] |
| 12 | 2 | cannot jump from switch statement to this case label |
| 13 | 2 | no member named 'mem_func' in namespace 'std' |
| 14 | 2 | non-void function does not return a value [-Werror,-Wreturn-type] |
| 15 | 2 | unknown type name '__va_list'; did you mean 'va_list'? |
| 16 | 2 | cast from pointer to smaller type 's32' (aka 'int') loses information |
| 17 | 1 | explicit specialization of 'push_back' after instantiation |
| 18 | 1 | 'wchar_t.h' file not found |
| 19 | 1 | explicit instantiation refers to member function 'JGeometry::TVec3<float>::TVec3' that is not an instantiation |
| 20 | 1 | use of undeclared identifier 'strcmp' |

## Shared-header interpretation

`libs/JSystem/include/JSystem/JGeometry/TMatrix.hpp:793` is the first reported
failure in 1,638 translation units (88.3% of primary-baseline failures). The
undeclared identifier is `get`; additional failures will become visible after
this shared blocker is handled. Counts describe first-error occurrences, not
predictions of how many units will pass after a change.

The C++ linkage diagnostics appear in libc++ `enable_if.h:20` (16 files) and
`byte.h:57` (3 files). Their diagnostics point to the `extern "C"` block starting
at `libs/RVL_SDK/include/revolution/mem/list.h:5`; this is the repository location
for the library lane to investigate.

## Transferred hooks: pending

The lead has not announced an overlay, and
`~/MacioGalaxy2-team/scratch-port/overlay` did not exist at handoff. No hooked
results are claimed. The audit now accepts `--root`, including overlays that
reuse this checkout's native headers when they have no `native/` directory.
After the lead announces the overlay, run:

```sh
nice -n 10 python3 native/tools/audit_compile.py --root ~/MacioGalaxy2-team/scratch-port/overlay --jobs 6 --output ~/MacioGalaxy2-team/scratch-port/solb/native-audit-hooked.json src/Game src/JSystem src/nw4r
python3 tools/port/classify_audit.py build/native-audit-baseline.json build/native-audit-rvl-sdk.json build/native-audit-libms.json --overlay ~/MacioGalaxy2-team/scratch-port/solb/native-audit-hooked.json --output build/native-audit-comparison.md
```

The classifier adds a “With transferred hooks (passing/checked)” column when
`--overlay` is provided. Audit optional SDK/libms paths as well before interpreting
their overlay comparison rows; a `0/0` cell means no units were checked.

## Tool validation

`python3 -m unittest discover -s tools/port -p 'test_*.py'` passes four tests
covering error classes, symbol extraction, warning/note handling, fatal/driver
diagnostics, shared-header aggregation, ownership, duplicate inputs, empty source
groups, and the hook comparison column. A one-file temporary source-tree smoke
check passed through `--root`, including native-header fallback and relative
output placement. No source fixes or commits were made.

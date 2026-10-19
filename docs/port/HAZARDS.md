# Native ABI and endianness hazard worklist

Swept 2026-10-19T20:52:42.426548+00:00; source root: `/Users/roberthaisfield/MacioGalaxy2-team/scratch-port/ht2/overlay`. Native target: arm64, little-endian, 8-byte pointers; Wii: PPC, big-endian, 4-byte pointers.

Scanned **5329 files**; **1851 candidate locations**. Counts are one finding per file/line/class; classes can overlap.

This is a lexical worklist with declaration lookup, not a full C++ analysis or proof of runtime failure. High confidence means a pointer/address expression, compiler-confirmed truncation, a known wire record, or a mixed pointer/value union was identified. Review rows retain unresolved numeric casts and runtime layouts separately. Comments, string literals, standalone Wii offset annotations, numeric conversions, and recognized native-excluded branches do not create findings.

Declarations are resolved in the current file and two include levels, using the nearest earlier declaration or an unambiguous header declaration. Full scope, overload, typedef, macro, and inheritance resolution is unavailable. Pointer subtraction is treated as a numeric difference; byte-array elements are numeric. A zero count is not a safety guarantee.

Suppressed nonpointer casts: 714. Serialized records with conversion evidence: 162; retained in the inventory, excluded from unresolved findings. Recognized GALAXY_NATIVE-inactive branches are masked before declaration lookup. Conversion evidence is structural, not runtime validation; unsupported variants remain possible.

## Reproduce

```sh
nice -n 10 python3 tools/port/hazard_sweep.py --root /Users/roberthaisfield/MacioGalaxy2-team/scratch-port/ht2/overlay --reference ~/MacioGalaxy --audit ~/MacioGalaxy2-team/scratch-port/solb/native-audit-hooked.json --baseline ~/MacioGalaxy2-team/scratch-port/solb/native-hazards-worktree-baseline.json --output docs/port/HAZARDS.md --json-output build/native-hazards.json
python3 -m unittest discover -s tools/port -p 'test_hazard_sweep.py'
```

Reference: `/Users/roberthaisfield/MacioGalaxy`, commit `16b9e13ac05d6af36666e69b024c91dd0acc69ec`; native patch comparisons use pristine Petari `e5bc761c0` through reference HEAD. The reference was read only. Input content digest: `b8b6e107341f781feb662fa379543746e287be2989441fe8315cee462a96edf4`.

## Per-class totals

The earlier worktree snapshot is preserved for context. Branch masking and the curated record inventory were refined afterward, so this column is not a controlled measurement of fixes. The compiler comparison in COMPILE_STATUS.md uses the original raw baseline and the hooked overlay.

| Class | Earlier worktree snapshot | Overlay high | Overlay review | Overlay total |
| --- | ---: | ---: | ---: | ---: |
| bitfield-layout | 39 | 16 | 21 | 37 |
| cast-unresolved | 2299 | 0 | 1053 | 1053 |
| endian-byte-union | 2 | 2 | 0 | 2 |
| endian-disc-record | 166 | 24 | 0 | 24 |
| endian-raw-access | 354 | 5 | 334 | 339 |
| integer-pointer-storage | 54 | 0 | 71 | 71 |
| packed-layout | 11 | 0 | 11 | 11 |
| pointer-alignment-review | 0 | 0 | 0 | 0 |
| pointer-size-assumption | 0 | 0 | 0 | 0 |
| pointer-storage32 | 3 | 0 | 2 | 2 |
| pointer-truncation | 295 | 298 | 0 | 298 |
| union-pointer-alias | 14 | 4 | 10 | 14 |

## Triage and format handling

1. Prioritize remaining Game/JSystem/nw4r pointer truncations, especially SMG2-only conversation/control code and address fields with no matching reference patch. Unresolved casts are review candidates; they are not counted as proven pointer conversions.
2. LMS/MSBT/MSBF is SMG2-only relative to this reference. MsbHeader, MsbBlockHeader, MsbLabelBucket, MsbfNode, and MsbtAttribute overlay big-endian data in src/libms/commonlib.c, flowfile.c, and msgfile.c. Counts, offsets, label values, branch tables, attributes, and UTF-16 text all need explicit wire decoding. MsbInfo, MsbBlock, MsbtInfo, and MsbfInfo are host handles and should grow with native pointers. The BMG declarations in MessageHolder.hpp do not establish that SMG1 message hooks cover LMS.
3. JSU_BE(T) / JSU_BE_UNALIGNED(T) in JSystem/JSupport/JSUBigEndian.hpp preserve serialized field width and decode big-endian values. The reference applies them to JASBNKParser, JASWSParser, JASAramStream, JAUSeqCollection, JAUSoundAnimator, JAUSoundTable, and JAUStreamFileTable. JMap uses BigEndianValue wrappers. Width-preserving offset aliases alone do not swap bytes.
4. J3D model/animation, JPAC, KCL, BRLYT/BRLAN, BTI/ResTIMG, BRFNT, and RARC use resource conversion or separate host descriptors. Transfer both converters and their entry hooks. KCLFile and font records must not overwrite four-byte wire offsets with eight-byte pointers. The inventory distinguishes detected hooks from unconverted or SMG2-only records; same format names do not prove support for every block/version.
5. Mixed pointer/value unions such as ActorParamEntry and TalkMessageCtrl need all active aliases and initialization paths reviewed. Pointer-only unions are review items: ordinary class growth and Wii offset comments alone are not failures. RFL face/database bitfields need explicit bit-order decoding; the reference face codec is in native/resource/rfl/rfl_native.cpp, with entry hooks in RFL_Database.c and RFL_HiddenDatabase.c. Runtime-only bitfields are review items unless serialized or aliased to a packed word.
6. RVL_SDK, Runtime, MSL_C, and MetroTRK are lower priority than game/resource paths. Hardware address casts may disappear behind native platform implementations; their static presence does not prove they execute in the native application.

## Directory ownership

| Directory (source + headers) | Lane | Files scanned | Earlier candidates | Overlay high | Overlay review | Overlay classes |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| Game/<root> | unassigned | 1 | 0 | 0 | 0 | — |
| Game/Animation | core | 24 | 7 | 0 | 3 | cast-unresolved: 3 |
| Game/AreaObj | objects | 90 | 2 | 0 | 0 | — |
| Game/AudioLib | core | 81 | 39 | 0 | 5 | cast-unresolved: 4; endian-raw-access: 1 |
| Game/Boss | objects | 358 | 106 | 0 | 32 | cast-unresolved: 32 |
| Game/Camera | core | 258 | 27 | 0 | 10 | cast-unresolved: 10 |
| Game/Demo | core | 88 | 14 | 0 | 8 | cast-unresolved: 8 |
| Game/Effect | core | 63 | 8 | 0 | 1 | cast-unresolved: 1 |
| Game/Enemy | enemies | 331 | 84 | 1 | 35 | cast-unresolved: 34; pointer-truncation: 1; union-pointer-alias: 1 |
| Game/GameAudio | core | 20 | 7 | 0 | 3 | cast-unresolved: 3 |
| Game/Gravity | core | 32 | 2 | 0 | 0 | — |
| Game/LiveActor | core | 165 | 38 | 1 | 12 | cast-unresolved: 11; union-pointer-alias: 1; bitfield-layout: 1 |
| Game/Map | core | 171 | 65 | 0 | 37 | cast-unresolved: 36; endian-raw-access: 1 |
| Game/MapObj | objects | 808 | 186 | 0 | 109 | cast-unresolved: 100; endian-raw-access: 8; bitfield-layout: 1 |
| Game/NPC | enemies | 215 | 137 | 1 | 54 | cast-unresolved: 40; integer-pointer-storage: 14; union-pointer-alias: 1 |
| Game/NWC24 | core | 16 | 0 | 0 | 0 | — |
| Game/NameObj | core | 30 | 13 | 0 | 3 | cast-unresolved: 2; union-pointer-alias: 1 |
| Game/Player | astra | 241 | 160 | 0 | 79 | cast-unresolved: 76; bitfield-layout: 2; pointer-storage32: 1 |
| Game/RhythmLib | enemies | 32 | 25 | 0 | 2 | cast-unresolved: 2 |
| Game/Ride | objects | 67 | 25 | 0 | 22 | cast-unresolved: 22 |
| Game/Scene | core | 52 | 13 | 4 | 3 | pointer-truncation: 4; cast-unresolved: 3 |
| Game/Screen | enemies | 311 | 118 | 7 | 38 | cast-unresolved: 37; pointer-truncation: 7; endian-raw-access: 1 |
| Game/Speaker | core | 15 | 35 | 0 | 10 | cast-unresolved: 7; packed-layout: 2; endian-raw-access: 1 |
| Game/System | core | 322 | 273 | 21 | 114 | cast-unresolved: 89; pointer-truncation: 20; endian-raw-access: 19; integer-pointer-storage: 4; union-pointer-alias: 2; endian-disc-record: 1 |
| Game/Util | core | 218 | 219 | 1 | 46 | cast-unresolved: 27; endian-raw-access: 19; endian-byte-union: 1 |
| JSystem/<root> | m1 | 5 | 0 | 0 | 0 | — |
| JSystem/J2DGraph | m1 | 14 | 1 | 0 | 1 | cast-unresolved: 1 |
| JSystem/J3DGraphAnimator | m1 | 24 | 20 | 0 | 4 | cast-unresolved: 4 |
| JSystem/J3DGraphBase | m1 | 28 | 122 | 0 | 43 | cast-unresolved: 31; endian-raw-access: 12 |
| JSystem/J3DGraphLoader | m1 | 13 | 56 | 4 | 30 | cast-unresolved: 27; pointer-truncation: 4; endian-raw-access: 3 |
| JSystem/JAudio2 | m1 | 157 | 162 | 4 | 44 | cast-unresolved: 30; bitfield-layout: 7; endian-raw-access: 5; pointer-truncation: 4; integer-pointer-storage: 1; union-pointer-alias: 1 |
| JSystem/JGadget | m1 | 5 | 2 | 0 | 0 | — |
| JSystem/JGeometry | m1 | 7 | 5 | 0 | 5 | cast-unresolved: 4; endian-raw-access: 1 |
| JSystem/JKernel | m1 | 46 | 53 | 11 | 11 | pointer-truncation: 10; cast-unresolved: 8; endian-raw-access: 2; bitfield-layout: 1; integer-pointer-storage: 1 |
| JSystem/JMath | m1 | 6 | 6 | 0 | 5 | cast-unresolved: 5 |
| JSystem/JParticle | m1 | 39 | 21 | 8 | 13 | endian-raw-access: 9; endian-disc-record: 8; cast-unresolved: 4 |
| JSystem/JSupport | m1 | 17 | 9 | 2 | 1 | pointer-truncation: 2; cast-unresolved: 1 |
| JSystem/JUtility | m1 | 25 | 20 | 2 | 7 | cast-unresolved: 4; integer-pointer-storage: 3; endian-disc-record: 2 |
| MSL_C | m1 | 101 | 103 | 3 | 94 | endian-raw-access: 64; cast-unresolved: 28; pointer-truncation: 3; bitfield-layout: 2 |
| MSL_C++ | m1 | 1 | 0 | 0 | 0 | — |
| MetroTRK | m1 | 58 | 71 | 9 | 57 | endian-raw-access: 38; cast-unresolved: 10; pointer-truncation: 9; integer-pointer-storage: 9 |
| RVLFaceLib | m1 | 42 | 36 | 28 | 9 | bitfield-layout: 16; endian-disc-record: 7; endian-raw-access: 6; pointer-truncation: 4; integer-pointer-storage: 2; endian-byte-union: 1; union-pointer-alias: 1 |
| RVL_SDK | m1 | 638 | 851 | 228 | 517 | cast-unresolved: 326; pointer-truncation: 227; endian-raw-access: 140; integer-pointer-storage: 35; packed-layout: 8; union-pointer-alias: 5; bitfield-layout: 3; pointer-storage32: 1 |
| Runtime | m1 | 11 | 3 | 1 | 1 | pointer-truncation: 1; union-pointer-alias: 1 |
| include/<root> | unassigned | 2 | 0 | 0 | 0 | — |
| libms | m1 | 5 | 11 | 10 | 1 | endian-disc-record: 5; endian-raw-access: 5; packed-layout: 1 |
| nw4r/<root> | m1 | 1 | 0 | 0 | 0 | — |
| nw4r/db | m1 | 8 | 14 | 1 | 13 | cast-unresolved: 10; integer-pointer-storage: 2; pointer-truncation: 1; endian-raw-access: 1 |
| nw4r/lyt | m1 | 33 | 49 | 1 | 16 | cast-unresolved: 12; bitfield-layout: 4; endian-disc-record: 1 |
| nw4r/math | m1 | 6 | 0 | 0 | 0 | — |
| nw4r/ut | m1 | 28 | 19 | 1 | 4 | endian-raw-access: 3; pointer-truncation: 1; cast-unresolved: 1 |

## On-disc record inventory

This table names curated serialized records and their reference conversion strategy. The library lane must check version tags, nested tables, alignment, and the loader entry hook before reusing a converter. A same-path/name match establishes a declaration precedent only; it does not prove the binary formats are identical. Offset fields stored as u32 remain u32 on disc; relocate them into separate native descriptors when pointers are required.

| Format | Record at file:line | Pointer members | Current conversion evidence | Reference status |
| --- | --- | --- | --- | --- |
| BCSV/JMap | JMapItem — include/Game/Util/JMapInfo.hpp:24 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: include/Game/Util/JMapInfo.hpp — format precedent; validate SMG2 record/version before transfer |
| BCSV/JMap | JMapData — include/Game/Util/JMapInfo.hpp:32 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: include/Game/Util/JMapInfo.hpp — format precedent; validate SMG2 record/version before transfer |
| BMG legacy declarations (active use unproven) | MessageInfoBlock — include/Game/System/MessageHolder.hpp:16 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: include/Game/System/MessageHolder.hpp — format precedent; validate SMG2 record/version before transfer |
| BMG legacy declarations (active use unproven) | MessageDataBlock — include/Game/System/MessageHolder.hpp:24 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: include/Game/System/MessageHolder.hpp — format precedent; validate SMG2 record/version before transfer |
| BMG legacy declarations (active use unproven) | MessageFlowBlock — include/Game/System/MessageHolder.hpp:29 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: include/Game/System/MessageHolder.hpp — format precedent; validate SMG2 record/version before transfer |
| BMG legacy declarations (active use unproven) | MessageFLI1Block — include/Game/System/MessageHolder.hpp:37 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: include/Game/System/MessageHolder.hpp — format precedent; validate SMG2 record/version before transfer |
| BRFNT | BinaryFileHeader — libs/nw4r/include/nw4r/ut/binaryFileFormat.h:9 | no | conversion entry hook present: src/nw4r/ut/ut_ResFont.cpp (BuildNativeFont(brfnt); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/ut/ut_ResFont.cpp — format precedent; validate SMG2 record/version before transfer |
| BRFNT | BinaryBlockHeader — libs/nw4r/include/nw4r/ut/binaryFileFormat.h:18 | no | conversion entry hook present: src/nw4r/ut/ut_ResFont.cpp (BuildNativeFont(brfnt); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/ut/ut_ResFont.cpp — format precedent; validate SMG2 record/version before transfer |
| BRFNT | FontTextureGlyph — libs/nw4r/include/nw4r/ut/fontResourceFormat.h:13 | yes | conversion entry hook present: src/nw4r/ut/ut_ResFont.cpp (BuildNativeFont(brfnt); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/ut/ut_ResFont.cpp — format precedent; validate SMG2 record/version before transfer |
| BRFNT | FontWidth — libs/nw4r/include/nw4r/ut/fontResourceFormat.h:28 | yes | conversion entry hook present: src/nw4r/ut/ut_ResFont.cpp (BuildNativeFont(brfnt); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/ut/ut_ResFont.cpp — format precedent; validate SMG2 record/version before transfer |
| BRFNT | FontCodeMap — libs/nw4r/include/nw4r/ut/fontResourceFormat.h:35 | yes | conversion entry hook present: src/nw4r/ut/ut_ResFont.cpp (BuildNativeFont(brfnt); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/ut/ut_ResFont.cpp — format precedent; validate SMG2 record/version before transfer |
| BRFNT | FontInformation — libs/nw4r/include/nw4r/ut/fontResourceFormat.h:44 | yes | conversion entry hook present: src/nw4r/ut/ut_ResFont.cpp (BuildNativeFont(brfnt); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/ut/ut_ResFont.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | BinaryFileHeader — libs/nw4r/include/nw4r/lyt/resources.h:50 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | DataBlockHeader — libs/nw4r/include/nw4r/lyt/resources.h:59 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | Layout — libs/nw4r/include/nw4r/lyt/resources.h:64 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | Font — libs/nw4r/include/nw4r/lyt/resources.h:71 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | FontList — libs/nw4r/include/nw4r/lyt/resources.h:77 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | Texture — libs/nw4r/include/nw4r/lyt/resources.h:83 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | TextureList — libs/nw4r/include/nw4r/lyt/resources.h:89 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | TexMap — libs/nw4r/include/nw4r/lyt/resources.h:95 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | MaterialList — libs/nw4r/include/nw4r/lyt/resources.h:144 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | Pane — libs/nw4r/include/nw4r/lyt/resources.h:150 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | Picture — libs/nw4r/include/nw4r/lyt/resources.h:164 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | TextBox — libs/nw4r/include/nw4r/lyt/resources.h:171 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | WindowFrame — libs/nw4r/include/nw4r/lyt/resources.h:186 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | WindowContent — libs/nw4r/include/nw4r/lyt/resources.h:192 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | Window — libs/nw4r/include/nw4r/lyt/resources.h:199 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | Bounding — libs/nw4r/include/nw4r/lyt/resources.h:207 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | ExtUserData — libs/nw4r/include/nw4r/lyt/resources.h:209 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | SMG2-only declaration: no same-path/name reference record |
| BRLYT/BRLAN | ExtUserDataList — libs/nw4r/include/nw4r/lyt/resources.h:230 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | Group — libs/nw4r/include/nw4r/lyt/resources.h:240 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | MaterialResourceNum — libs/nw4r/include/nw4r/lyt/resources.h:247 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | Material — libs/nw4r/include/nw4r/lyt/resources.h:295 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | AnimationTagBlock — libs/nw4r/include/nw4r/lyt/resources.h:302 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | AnimationShareBlock — libs/nw4r/include/nw4r/lyt/resources.h:314 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | AnimationBlock — libs/nw4r/include/nw4r/lyt/resources.h:321 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | AnimationContent — libs/nw4r/include/nw4r/lyt/resources.h:331 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | AnimationInfo — libs/nw4r/include/nw4r/lyt/resources.h:339 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | AnimationTarget — libs/nw4r/include/nw4r/lyt/resources.h:346 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | HermiteKey — libs/nw4r/include/nw4r/lyt/resources.h:358 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BRLYT/BRLAN | StepKey — libs/nw4r/include/nw4r/lyt/resources.h:364 | no | conversion entry hook present: src/nw4r/lyt/lyt_layout.cpp (NativeNormalizeResource); format/version coverage still requires validation | MacioGalaxy has a fix: src/nw4r/lyt/lyt_nativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| BTI/ResTIMG | ResTIMG — libs/JSystem/include/JSystem/JUtility/JUTTexture.hpp:8 | no | conversion entry hook present: src/Game/System/ResourceHolder.cpp (JUTNativeNormalizeResTIMG); format/version coverage still requires validation | MacioGalaxy has a fix: src/JSystem/JUtility/JUTNativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D material tables | J3DZModeInfo — libs/JSystem/include/JSystem/J3DGraphBase/J3DStruct.hpp:28 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D material tables | J3DFogInfo — libs/JSystem/include/JSystem/J3DGraphBase/J3DStruct.hpp:35 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D material tables | J3DBlendInfo — libs/JSystem/include/JSystem/J3DGraphBase/J3DStruct.hpp:49 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D material tables | J3DTextureSRTInfo — libs/JSystem/include/JSystem/J3DGraphBase/J3DStruct.hpp:60 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D material tables | J3DTexMtxInfo — libs/JSystem/include/JSystem/J3DGraphBase/J3DStruct.hpp:113 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D material tables | J3DLightInfo — libs/JSystem/include/JSystem/J3DGraphBase/J3DStruct.hpp:126 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D material tables | J3DColorChanInfo — libs/JSystem/include/JSystem/J3DGraphBase/J3DStruct.hpp:135 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D material tables | J3DTexCoordInfo — libs/JSystem/include/JSystem/J3DGraphBase/J3DStruct.hpp:145 | yes | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D material tables | J3DNBTScaleInfo — libs/JSystem/include/JSystem/J3DGraphBase/J3DStruct.hpp:157 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D material tables | J3DTevOrderInfo — libs/JSystem/include/JSystem/J3DGraphBase/J3DStruct.hpp:164 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D material tables | J3DTevSwapModeTableInfo — libs/JSystem/include/JSystem/J3DGraphBase/J3DStruct.hpp:175 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D material tables | J3DTevStageInfo — libs/JSystem/include/JSystem/J3DGraphBase/J3DStruct.hpp:182 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D material tables | J3DIndTevStageInfo — libs/JSystem/include/JSystem/J3DGraphBase/J3DStruct.hpp:205 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D material tables | J3DTevSwapModeInfo — libs/JSystem/include/JSystem/J3DGraphBase/J3DStruct.hpp:218 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D material tables | J3DIndTexOrderInfo — libs/JSystem/include/JSystem/J3DGraphBase/J3DStruct.hpp:225 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D material tables | J3DIndTexMtxInfo — libs/JSystem/include/JSystem/J3DGraphBase/J3DStruct.hpp:236 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D material tables | J3DIndTexCoordScaleInfo — libs/JSystem/include/JSystem/J3DGraphBase/J3DStruct.hpp:242 | yes | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D material tables | J3DAlphaCompInfo — libs/JSystem/include/JSystem/J3DGraphBase/J3DStruct.hpp:254 | yes | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | JUTDataBlockHeader — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:17 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | JUTDataFileHeader — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:26 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmTransform_ANK1 — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:36 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmKeyTableBase — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:55 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmColorKeyTable — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:61 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmVtxColorIndexData — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:68 | yes | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmColorFullTable — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:73 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmVisibilityFullTable — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:84 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmTransformKeyTable — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:89 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmTransformFullTable — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:95 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmTexPatternFullTable — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:104 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmCRegKeyTable — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:111 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmKRegKeyTable — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:120 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmDataBlockHeader — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:129 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmDataHeader — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:134 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmVtxColorFullData — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:143 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmVisibilityFullData — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:159 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmTransformFullData — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:170 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmColorKeyData — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:183 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmTextureSRTKeyData — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:202 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmVtxColorKeyData — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:234 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmTexPatternFullData — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:250 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmTevRegKeyData — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:263 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmColorFullData — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:294 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmClusterKeyTable — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:310 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmTransformKeyData — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:314 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmClusterKeyData — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:327 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmClusterFullData — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:336 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DAnmClusterFullTable — libs/JSystem/include/JSystem/J3DGraphAnimator/J3DAnimation.hpp:345 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_animation.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DJointInitData — libs/JSystem/include/JSystem/J3DGraphLoader/J3DJointFactory.hpp:8 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DMaterialInitData — libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory.hpp:6 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DIndInitData — libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory.hpp:39 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DPatchingInfo — libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory.hpp:51 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DMaterialInitData_v21 — libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory_v21.hpp:10 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DModelBlock — libs/JSystem/include/JSystem/J3DGraphLoader/J3DModelLoader.hpp:57 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DModelFileData — libs/JSystem/include/JSystem/J3DGraphLoader/J3DModelLoader.hpp:62 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DModelInfoBlock — libs/JSystem/include/JSystem/J3DGraphLoader/J3DModelLoader.hpp:71 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DVertexBlock — libs/JSystem/include/JSystem/J3DGraphLoader/J3DModelLoader.hpp:78 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DEnvelopeBlock — libs/JSystem/include/JSystem/J3DGraphLoader/J3DModelLoader.hpp:87 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DDrawBlock — libs/JSystem/include/JSystem/J3DGraphLoader/J3DModelLoader.hpp:95 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DJointBlock — libs/JSystem/include/JSystem/J3DGraphLoader/J3DModelLoader.hpp:101 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DMaterialBlock — libs/JSystem/include/JSystem/J3DGraphLoader/J3DModelLoader.hpp:108 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DMaterialBlock_v21 — libs/JSystem/include/JSystem/J3DGraphLoader/J3DModelLoader.hpp:142 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DMaterialDLBlock — libs/JSystem/include/JSystem/J3DGraphLoader/J3DModelLoader.hpp:173 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DShapeBlock — libs/JSystem/include/JSystem/J3DGraphLoader/J3DModelLoader.hpp:183 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DTextureBlock — libs/JSystem/include/JSystem/J3DGraphLoader/J3DModelLoader.hpp:195 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DShapeInitData — libs/JSystem/include/JSystem/J3DGraphLoader/J3DShapeFactory.hpp:10 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DShapeMtxInitData — libs/JSystem/include/JSystem/J3DGraphLoader/J3DShapeFactory.hpp:21 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D model/animation | J3DShapeDrawInitData — libs/JSystem/include/JSystem/J3DGraphLoader/J3DShapeFactory.hpp:27 | no | conversion entry hook present: src/JSystem/J3DGraphLoader/J3DModelLoader.cpp (makeHostModelImage); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/j3d_model.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D name tables | ResNTAB — libs/JSystem/include/JSystem/JUtility/JUTNameTab.hpp:5 | no | none detected | MacioGalaxy has a fix: src/JSystem/JUtility/JUTNativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| J3D name tables | Entry — libs/JSystem/include/JSystem/JUtility/JUTNameTab.hpp:9 | no | none detected | MacioGalaxy has a fix: src/JSystem/JUtility/JUTNativeResource.cpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TFileHeader — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:14 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TOsc — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:22 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TPercData — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:32 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TChunk — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:40 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TEnvtChunk — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:45 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TOscChunk — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:49 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TListChunk — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:54 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TRandData — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:59 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TSenseData — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:65 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TVeloData — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:73 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TInstData — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:80 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TOffset — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:92 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TOsc — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:99 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TVmap — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:108 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TRandEffect — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:115 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TSenseEffect — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:121 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TKeymap — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:129 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TInst — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:135 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TPmap — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:146 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TPerc — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:154 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TOffsetData — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:162 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | THeader — libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp:169 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASBNKParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TOffset — libs/JSystem/include/JSystem/JAudio2/JASWSParser.hpp:14 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASWSParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TCtrlWave — libs/JSystem/include/JSystem/JAudio2/JASWSParser.hpp:24 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASWSParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TWave — libs/JSystem/include/JSystem/JAudio2/JASWSParser.hpp:34 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASWSParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TWaveArchive — libs/JSystem/include/JSystem/JAudio2/JASWSParser.hpp:49 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASWSParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TWaveArchiveBank — libs/JSystem/include/JSystem/JAudio2/JASWSParser.hpp:55 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASWSParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TCtrl — libs/JSystem/include/JSystem/JAudio2/JASWSParser.hpp:61 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASWSParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TCtrlScene — libs/JSystem/include/JSystem/JAudio2/JASWSParser.hpp:67 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASWSParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | TCtrlGroup — libs/JSystem/include/JSystem/JAudio2/JASWSParser.hpp:73 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASWSParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | THeader — libs/JSystem/include/JSystem/JAudio2/JASWSParser.hpp:81 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASWSParser.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | JAUSoundTableItem — libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp:9 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | JAUSoundTableSe — libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp:16 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | JAUSoundTableBgm — libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp:23 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | JAUSoundTableStream — libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp:31 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | JAUSoundTableRoot — libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp:102 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | JAUSoundTableSection — libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp:110 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | JAUSoundTableGroup — libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp:125 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | JAUSoundNameTableRoot — libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp:177 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | JAUSoundNameTableSection — libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp:185 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio banks/waves/tables | JAUSoundNameTableGroup — libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp:201 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio sequences/streams | Header — libs/JSystem/include/JSystem/JAudio2/JASAramStream.hpp:30 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASAramStream.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio sequences/streams | BlockHeader — libs/JSystem/include/JSystem/JAudio2/JASAramStream.hpp:47 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASAramStream.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio sequences/streams | JAUSeqCollectionData — libs/JSystem/include/JSystem/JAudio2/JAUSeqCollection.hpp:9 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JAUSeqCollection.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio sequences/streams | JAUSeqCollectionSoundTable — libs/JSystem/include/JSystem/JAudio2/JAUSeqCollection.hpp:16 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JAUSeqCollection.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio sequences/streams | JAUSoundAnimationSound — libs/JSystem/include/JSystem/JAudio2/JAUSoundAnimator.hpp:12 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JAUSoundAnimator.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio sequences/streams | JAUSoundAnimation — libs/JSystem/include/JSystem/JAudio2/JAUSoundAnimator.hpp:144 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JAUSoundAnimator.hpp — format precedent; validate SMG2 record/version before transfer |
| JAudio sequences/streams | BinaryStreamFileTable — libs/JSystem/include/JSystem/JAudio2/JAUStreamFileTable.hpp:6 | no | big-endian field/nested wrapper types present; no raw primitive or pointer fields detected | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JAUStreamFileTable.hpp — format precedent; validate SMG2 record/version before transfer |
| JPA/JPAC | JPABaseShapeData — libs/JSystem/include/JSystem/JParticle/JPABaseShape.hpp:9 | no | none detected | MacioGalaxy has a fix: native/resource/jpa_resource.cpp — format precedent; validate SMG2 record/version before transfer |
| JPA/JPAC | JPAClrAnmKeyData — libs/JSystem/include/JSystem/JParticle/JPABaseShape.hpp:248 | no | none detected | MacioGalaxy has a fix: native/resource/jpa_resource.cpp — format precedent; validate SMG2 record/version before transfer |
| JPA/JPAC | JPAChildShapeData — libs/JSystem/include/JSystem/JParticle/JPAChildShape.hpp:8 | no | none detected | MacioGalaxy has a fix: native/resource/jpa_resource.cpp — format precedent; validate SMG2 record/version before transfer |
| JPA/JPAC | JPADynamicsBlockData — libs/JSystem/include/JSystem/JParticle/JPADynamicsBlock.hpp:7 | no | none detected | MacioGalaxy has a fix: native/resource/jpa_resource.cpp — format precedent; validate SMG2 record/version before transfer |
| JPA/JPAC | JPAExTexShapeData — libs/JSystem/include/JSystem/JParticle/JPAExTexShape.hpp:7 | no | none detected | MacioGalaxy has a fix: native/resource/jpa_resource.cpp — format precedent; validate SMG2 record/version before transfer |
| JPA/JPAC | JPAExtraShapeData — libs/JSystem/include/JSystem/JParticle/JPAExtraShape.hpp:8 | no | none detected | MacioGalaxy has a fix: native/resource/jpa_resource.cpp — format precedent; validate SMG2 record/version before transfer |
| JPA/JPAC | JPAFieldBlockData — libs/JSystem/include/JSystem/JParticle/JPAFieldBlock.hpp:123 | no | none detected | MacioGalaxy has a fix: native/resource/jpa_resource.cpp — format precedent; validate SMG2 record/version before transfer |
| JPA/JPAC | JPATextureData — libs/JSystem/include/JSystem/JParticle/JPATexture.hpp:5 | no | none detected | MacioGalaxy has a fix: native/resource/jpa_resource.cpp — format precedent; validate SMG2 record/version before transfer |
| KCL | KC_PrismData — include/Game/Map/KCollision.hpp:9 | no | conversion entry hook present: src/Game/System/ResourceHolder.cpp (PetariNative::KCL::convertInPlace); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/kcl_collision.cpp — format precedent; validate SMG2 record/version before transfer |
| KCL | KCLFile — include/Game/Map/KCollision.hpp:31 | yes | conversion entry hook present: src/Game/System/ResourceHolder.cpp (PetariNative::KCL::convertInPlace); format/version coverage still requires validation | MacioGalaxy has a fix: native/resource/kcl_collision.cpp — format precedent; validate SMG2 record/version before transfer |
| LMS/MSBT/MSBF | MsbHeader — libs/libms/include/libms.h:18 | no | none detected | SMG2-only declaration: no same-path/name reference record |
| LMS/MSBT/MSBF | MsbBlockHeader — libs/libms/include/libms.h:32 | no | none detected | SMG2-only declaration: no same-path/name reference record |
| LMS/MSBT/MSBF | MsbLabelBucket — libs/libms/include/libms.h:56 | no | none detected | SMG2-only declaration: no same-path/name reference record |
| LMS/MSBT/MSBF | MsbfNode — libs/libms/include/libms.h:78 | no | none detected | SMG2-only declaration: no same-path/name reference record |
| LMS/MSBT/MSBF | MsbtAttribute — libs/libms/include/libms.h:84 | no | none detected | SMG2-only declaration: no same-path/name reference record |
| RARC | RarcHeader — libs/JSystem/include/JSystem/JKernel/JKRArchive.hpp:38 | no | conversion entry hook present: src/JSystem/JKernel/JKRMemArchive.cpp (setupNativeTables); format/version coverage still requires validation | MacioGalaxy has a fix: src/JSystem/JKernel/JKRArchivePri.cpp — format precedent; validate SMG2 record/version before transfer |
| RARC | RarcInfoBlock — libs/JSystem/include/JSystem/JKernel/JKRArchive.hpp:49 | no | conversion entry hook present: src/JSystem/JKernel/JKRMemArchive.cpp (setupNativeTables); format/version coverage still requires validation | MacioGalaxy has a fix: src/JSystem/JKernel/JKRArchivePri.cpp — format precedent; validate SMG2 record/version before transfer |
| RARC | SDIFileEntry — libs/JSystem/include/JSystem/JKernel/JKRArchive.hpp:61 | yes | conversion entry hook present: src/JSystem/JKernel/JKRMemArchive.cpp (setupNativeTables); format/version coverage still requires validation | MacioGalaxy has a fix: src/JSystem/JKernel/JKRArchivePri.cpp — format precedent; validate SMG2 record/version before transfer |
| RARC | SDIDirEntry — libs/JSystem/include/JSystem/JKernel/JKRArchive.hpp:76 | no | conversion entry hook present: src/JSystem/JKernel/JKRMemArchive.cpp (setupNativeTables); format/version coverage still requires validation | MacioGalaxy has a fix: src/JSystem/JKernel/JKRArchivePri.cpp — format precedent; validate SMG2 record/version before transfer |
| RFL database table | RFLiDatabase — libs/RVLFaceLib/include/RFLi_Database.h:12 | no | none detected | MacioGalaxy has a fix: src/RVLFaceLib/RFL_Database.c — format precedent; validate SMG2 record/version before transfer |
| RFL database table | RFLiTableData — libs/RVLFaceLib/include/RFLi_Format.h:9 | no | none detected | reference record exists; no verified native fix |
| RFL database table | RFLiHiddenDB — libs/RVLFaceLib/include/RFLi_HiddenDatabase.h:15 | no | none detected | MacioGalaxy has a fix: src/RVLFaceLib/RFL_HiddenDatabase.c — format precedent; validate SMG2 record/version before transfer |
| RFL serialized faces | RFLiCharData — libs/RVLFaceLib/include/RFLi_Types.h:260 | no | none detected | MacioGalaxy has a fix: native/resource/rfl/rfl_native.cpp — format precedent; validate SMG2 record/version before transfer |
| RFL serialized faces | RFLiStoreData — libs/RVLFaceLib/include/RFLi_Types.h:345 | no | none detected | MacioGalaxy has a fix: native/resource/rfl/rfl_native.cpp — format precedent; validate SMG2 record/version before transfer |
| RFL serialized faces | RFLiHiddenCharData — libs/RVLFaceLib/include/RFLi_Types.h:350 | no | none detected | MacioGalaxy has a fix: native/resource/rfl/rfl_native.cpp — format precedent; validate SMG2 record/version before transfer |
| RFL texture | RFLiTexture — libs/RVLFaceLib/include/RFLi_Texture.h:9 | no | none detected | MacioGalaxy has a fix: native/resource/rfl/rfl_native.cpp — format precedent; validate SMG2 record/version before transfer |
| Save chunks | BinaryDataChunkHolderChunkData — include/Game/System/BinaryDataChunkHolder.hpp:19 | yes | none detected | reference record exists; no verified native fix |

## Findings (high confidence first)

“MacioGalaxy has a fix” identifies either matching Wii code in a native patch hunk or a known format converter. It describes reference evidence, not an applied or tested SMG2 fix. Unmarked rows require manual comparison; absence of a marker does not prove absence of a reference fix.

### bitfield-layout — high (16)

| File:line | Lane | Evidence | Reference |
| --- | --- | --- | --- |
| libs/JSystem/include/JSystem/JKernel/JKRArchive.hpp:64 | m1 | serialized/mapped record bit order changes on LE; explicit masks or decode required; record: SDIFileEntry; fields: mFlag:8, mNameOffset:24 | review needed |
| libs/RVLFaceLib/include/RFL_DataUtility.h:24 | m1 | serialized/mapped record bit order changes on LE; explicit masks or decode required; record: RFLAdditionalInfo; fields: sex:1, bmonth:4, bday:5, color:4, favorite:1, height:7, build:7, reserved:3 | review needed |
| libs/RVLFaceLib/include/RFLi_Database.h:17 | m1 | serialized/mapped record bit order changes on LE; explicit masks or decode required; record: RFLiDatabase; fields: isolation:1, padding1:31 | review needed |
| libs/RVLFaceLib/include/RFLi_Format.h:13 | m1 | serialized/mapped record bit order changes on LE; explicit masks or decode required; record: RFLiTableData; fields: sex:1, next:15, padding2:1, prev:15 | review needed |
| libs/RVLFaceLib/include/RFLi_Types.h:143 | m1 | serialized/mapped record bit order changes on LE; explicit masks or decode required; record: <anonymous>; fields: type:3, color:3, texture:4, padding:6 | review needed |
| libs/RVLFaceLib/include/RFLi_Types.h:153 | m1 | serialized/mapped record bit order changes on LE; explicit masks or decode required; record: <anonymous>; fields: type:7, color:3, flip:1, padding:5 | review needed |
| libs/RVLFaceLib/include/RFLi_Types.h:163 | m1 | serialized/mapped record bit order changes on LE; explicit masks or decode required; record: <anonymous>; fields: type:6, color:3, scale:4, rotate:5, x:4, y:5, padding:5 | review needed |
| libs/RVLFaceLib/include/RFLi_Types.h:176 | m1 | serialized/mapped record bit order changes on LE; explicit masks or decode required; record: <anonymous>; fields: type:5, color:3, scale:4, rotate:5, x:4, y:5, padding:6 | review needed |
| libs/RVLFaceLib/include/RFLi_Types.h:189 | m1 | serialized/mapped record bit order changes on LE; explicit masks or decode required; record: <anonymous>; fields: type:4, scale:4, y:5, padding:3 | review needed |
| libs/RVLFaceLib/include/RFLi_Types.h:199 | m1 | serialized/mapped record bit order changes on LE; explicit masks or decode required; record: <anonymous>; fields: type:5, color:2, scale:4, y:5 | review needed |
| libs/RVLFaceLib/include/RFLi_Types.h:209 | m1 | serialized/mapped record bit order changes on LE; explicit masks or decode required; record: <anonymous>; fields: mustache:2, type:2, color:3, scale:4, y:5 | review needed |
| libs/RVLFaceLib/include/RFLi_Types.h:220 | m1 | serialized/mapped record bit order changes on LE; explicit masks or decode required; record: <anonymous>; fields: type:4, color:3, scale:4, y:5 | review needed |
| libs/RVLFaceLib/include/RFLi_Types.h:230 | m1 | serialized/mapped record bit order changes on LE; explicit masks or decode required; record: <anonymous>; fields: type:1, scale:4, x:5, y:5, padding:1 | review needed |
| libs/RVLFaceLib/include/RFLi_Types.h:249 | m1 | serialized/mapped record bit order changes on LE; explicit masks or decode required; record: <anonymous>; fields: sex:1, bmonth:4, bday:5, color:4, favorite:1, localOnly:1 | review needed |
| libs/RVLFaceLib/include/RFLi_Types.h:262 | m1 | serialized/mapped record bit order changes on LE; explicit masks or decode required; record: RFLiCharData; fields: padding0:1, sex:1, birthMonth:4, birthDay:5, favoriteColor:4, favorite:1, faceType:3, faceColor:3, faceTex:4, padding2:3, localonly:1, type:2, hairType:7, hairColor:3, hairFlip:1, padding3:5, eyebrowType:5, eyebrowRotate:5, padding4:6, eyebrowColor:3, eyebrowScale:4, eyebrowY:5, eyebrowX:4, eyeType:6, eyeRotate:5, eyeY:5, eyeColor:3, eyeScale:4, eyeX:4, padding5:5, noseType:4, noseScale:4, noseY:5, padding6:3, mouthType:5, mouthColor:2, mouthScale:4, mouthY:5, glassType:4, glassColor:3, glassScale:4, glassY:5, mustacheType:2, beardType:2, beardColor:3, beardScale:4, beardY:5, moleType:1, moleScale:4, moleY:5, moleX:5, padding8:1 | review needed |
| libs/RVLFaceLib/include/RFLi_Types.h:352 | m1 | serialized/mapped record bit order changes on LE; explicit masks or decode required; record: RFLiHiddenCharData; fields: padding0:1, sex:1, birthPadding:9, favoriteColor:4, favorite:1, faceType:3, faceColor:3, faceTex:4, padding2:3, localonly:1, type:2, hairType:7, hairColor:3, hairFlip:1, padding3:5, eyebrowType:5, eyebrowRotate:5, padding4:6, eyebrowColor:3, eyebrowScale:4, eyebrowY:5, eyebrowX:4, eyeType:6, eyeRotate:5, eyeY:5, eyeColor:3, eyeScale:4, eyeX:4, padding5:5, noseType:4, noseScale:4, noseY:5, padding6:3, mouthType:5, mouthColor:2, mouthScale:4, mouthY:5, glassType:4, glassColor:3, glassScale:4, glassY:5, mustacheType:2, beardType:2, beardColor:3, beardScale:4, beardY:5, moleType:1, moleScale:4, moleY:5, moleX:5, padding8:1 | review needed |

### endian-byte-union — high (2)

| File:line | Lane | Evidence | Reference |
| --- | --- | --- | --- |
| include/Game/Util/Color.hpp:70 | core | u32 overlaps four bytes; packed word order changes on little-endian | review needed |
| libs/RVLFaceLib/include/RFLi_NANDAccess.h:19 | m1 | u32 overlaps four bytes; packed word order changes on little-endian | review needed |

### endian-disc-record — high (24)

| File:line | Lane | Evidence | Reference |
| --- | --- | --- | --- |
| include/Game/System/BinaryDataChunkHolder.hpp:19 | core | Save chunks: review byte swapping and four-byte wire offsets before native use; record: BinaryDataChunkHolderChunkData | review needed |
| libs/JSystem/include/JSystem/JParticle/JPABaseShape.hpp:9 | m1 | JPA/JPAC: review byte swapping and four-byte wire offsets before native use; record: JPABaseShapeData | MacioGalaxy has a fix: native/resource/jpa_resource.cpp |
| libs/JSystem/include/JSystem/JParticle/JPABaseShape.hpp:248 | m1 | JPA/JPAC: review byte swapping and four-byte wire offsets before native use; record: JPAClrAnmKeyData | MacioGalaxy has a fix: native/resource/jpa_resource.cpp |
| libs/JSystem/include/JSystem/JParticle/JPAChildShape.hpp:8 | m1 | JPA/JPAC: review byte swapping and four-byte wire offsets before native use; record: JPAChildShapeData | MacioGalaxy has a fix: native/resource/jpa_resource.cpp |
| libs/JSystem/include/JSystem/JParticle/JPADynamicsBlock.hpp:7 | m1 | JPA/JPAC: review byte swapping and four-byte wire offsets before native use; record: JPADynamicsBlockData | MacioGalaxy has a fix: native/resource/jpa_resource.cpp |
| libs/JSystem/include/JSystem/JParticle/JPAExTexShape.hpp:7 | m1 | JPA/JPAC: review byte swapping and four-byte wire offsets before native use; record: JPAExTexShapeData | MacioGalaxy has a fix: native/resource/jpa_resource.cpp |
| libs/JSystem/include/JSystem/JParticle/JPAExtraShape.hpp:8 | m1 | JPA/JPAC: review byte swapping and four-byte wire offsets before native use; record: JPAExtraShapeData | MacioGalaxy has a fix: native/resource/jpa_resource.cpp |
| libs/JSystem/include/JSystem/JParticle/JPAFieldBlock.hpp:123 | m1 | JPA/JPAC: review byte swapping and four-byte wire offsets before native use; record: JPAFieldBlockData | MacioGalaxy has a fix: native/resource/jpa_resource.cpp |
| libs/JSystem/include/JSystem/JParticle/JPATexture.hpp:5 | m1 | JPA/JPAC: review byte swapping and four-byte wire offsets before native use; record: JPATextureData | MacioGalaxy has a fix: native/resource/jpa_resource.cpp |
| libs/JSystem/include/JSystem/JUtility/JUTNameTab.hpp:5 | m1 | J3D name tables: review byte swapping and four-byte wire offsets before native use; record: ResNTAB | MacioGalaxy has a fix: src/JSystem/JUtility/JUTNativeResource.cpp |
| libs/JSystem/include/JSystem/JUtility/JUTNameTab.hpp:9 | m1 | J3D name tables: review byte swapping and four-byte wire offsets before native use; record: Entry | MacioGalaxy has a fix: src/JSystem/JUtility/JUTNativeResource.cpp |
| libs/RVLFaceLib/include/RFLi_Database.h:12 | m1 | RFL database table: review byte swapping and four-byte wire offsets before native use; record: RFLiDatabase | MacioGalaxy has a fix: src/RVLFaceLib/RFL_Database.c |
| libs/RVLFaceLib/include/RFLi_Format.h:9 | m1 | RFL database table: review byte swapping and four-byte wire offsets before native use; record: RFLiTableData | review needed |
| libs/RVLFaceLib/include/RFLi_HiddenDatabase.h:15 | m1 | RFL database table: review byte swapping and four-byte wire offsets before native use; record: RFLiHiddenDB | MacioGalaxy has a fix: src/RVLFaceLib/RFL_HiddenDatabase.c |
| libs/RVLFaceLib/include/RFLi_Texture.h:9 | m1 | RFL texture: review byte swapping and four-byte wire offsets before native use; record: RFLiTexture | MacioGalaxy has a fix: native/resource/rfl/rfl_native.cpp |
| libs/RVLFaceLib/include/RFLi_Types.h:260 | m1 | RFL serialized faces: review byte swapping and four-byte wire offsets before native use; record: RFLiCharData | MacioGalaxy has a fix: native/resource/rfl/rfl_native.cpp |
| libs/RVLFaceLib/include/RFLi_Types.h:345 | m1 | RFL serialized faces: review byte swapping and four-byte wire offsets before native use; record: RFLiStoreData | MacioGalaxy has a fix: native/resource/rfl/rfl_native.cpp |
| libs/RVLFaceLib/include/RFLi_Types.h:350 | m1 | RFL serialized faces: review byte swapping and four-byte wire offsets before native use; record: RFLiHiddenCharData | MacioGalaxy has a fix: native/resource/rfl/rfl_native.cpp |
| libs/libms/include/libms.h:18 | m1 | LMS/MSBT/MSBF: review byte swapping and four-byte wire offsets before native use; record: MsbHeader | review needed |
| libs/libms/include/libms.h:32 | m1 | LMS/MSBT/MSBF: review byte swapping and four-byte wire offsets before native use; record: MsbBlockHeader | review needed |
| libs/libms/include/libms.h:56 | m1 | LMS/MSBT/MSBF: review byte swapping and four-byte wire offsets before native use; record: MsbLabelBucket | review needed |
| libs/libms/include/libms.h:78 | m1 | LMS/MSBT/MSBF: review byte swapping and four-byte wire offsets before native use; record: MsbfNode | review needed |
| libs/libms/include/libms.h:84 | m1 | LMS/MSBT/MSBF: review byte swapping and four-byte wire offsets before native use; record: MsbtAttribute | review needed |
| libs/nw4r/include/nw4r/lyt/resources.h:209 | m1 | BRLYT/BRLAN: review byte swapping and four-byte wire offsets before native use; record: ExtUserData | review needed |

### endian-raw-access — high (5)

| File:line | Lane | Evidence | Reference |
| --- | --- | --- | --- |
| src/libms/commonlib.c:30 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ((char*)pInfo->header + pos) | review needed |
| src/libms/commonlib.c:34 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ((char*)pInfo->header + pos + 4) | review needed |
| src/libms/flowfile.c:19 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pInfo->base.blocks[pInfo->flw2Index].data | review needed |
| src/libms/flowfile.c:49 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ((char*)pBlocks[blockIndex].data + (offset + entryLen + 1)) | review needed |
| src/libms/msgfile.c:41 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ((char*)pBlocks[blockIndex].data + (offset + entryLen + 1)) | review needed |

### pointer-truncation — high (298)

| File:line | Lane | Evidence | Reference |
| --- | --- | --- | --- |
| libs/MetroTRK/include/ppc/Generic/ppc_mem.h:7 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: ptr | review needed |
| libs/MetroTRK/include/ppc/Generic/ppc_mem.h:8 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: alignedPtr | review needed |
| libs/MetroTRK/include/ppc/Generic/ppc_mem.h:12 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: ptr | review needed |
| libs/MetroTRK/include/ppc/Generic/ppc_mem.h:14 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: alignedPtr | review needed |
| libs/MetroTRK/include/ppc/Generic/ppc_mem.h:15 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: alignedPtr | review needed |
| libs/RVL_SDK/include/revolution/gx/GXTypes.h:13 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: a | review needed |
| libs/RVL_SDK/include/revolution/mem/heapCommon.h:41 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: (ptr) | MacioGalaxy has a fix: libs/RVL_SDK/include/revolution/mem/heapCommon.h |
| libs/nw4r/include/nw4r/ut/CharStrmReader.h:36 | m1 | Clang-confirmed in audit snapshot: cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | review needed |
| src/Game/Enemy/TeresaAtomic.cpp:69 | enemies | Clang-confirmed in audit snapshot: cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | review needed |
| src/Game/Scene/StageDataHolder.cpp:300 | core | Clang-confirmed in audit snapshot: cast from pointer to smaller type 's32' (aka 'int') loses information | review needed |
| src/Game/Scene/StageDataHolder.cpp:533 | core | Clang-confirmed in audit snapshot: cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | review needed |
| src/Game/Scene/StageDataHolder.cpp:534 | core | 32-bit cast of a declared pointer/array/address expression; operand: it->mData | review needed |
| src/Game/Scene/StageDataHolder.cpp:537 | core | 32-bit cast of a declared pointer/array/address expression; operand: it->mData | review needed |
| src/Game/Screen/CustomTagProcessor.cpp:154 | enemies | 32-bit cast of a declared pointer/array/address expression; operand: this | review needed |
| src/Game/Screen/CustomTagProcessor.cpp:155 | enemies | 32-bit cast of a declared pointer/array/address expression; operand: this | review needed |
| src/Game/Screen/CustomTagProcessor.cpp:156 | enemies | 32-bit cast of a declared pointer/array/address expression; operand: this | review needed |
| src/Game/Screen/CustomTagProcessor.cpp:157 | enemies | 32-bit cast of a declared pointer/array/address expression; operand: this | review needed |
| src/Game/Screen/CustomTagProcessor.cpp:158 | enemies | 32-bit cast of a declared pointer/array/address expression; operand: this | review needed |
| src/Game/Screen/CustomTagProcessor.cpp:159 | enemies | 32-bit cast of a declared pointer/array/address expression; operand: this | review needed |
| src/Game/Screen/CustomTagProcessor.cpp:160 | enemies | 32-bit cast of a declared pointer/array/address expression; operand: this | review needed |
| src/Game/System/DrawSyncManager.cpp:81 | core | Clang-confirmed in audit snapshot: cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | MacioGalaxy has a fix: src/Game/System/DrawSyncManager.cpp |
| src/Game/System/DrawSyncManager.cpp:88 | core | Clang-confirmed in audit snapshot: cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | MacioGalaxy has a fix: src/Game/System/DrawSyncManager.cpp |
| src/Game/System/FileRipper.cpp:62 | core | Clang-confirmed in audit snapshot: cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | review needed |
| src/Game/System/MarioFaceShipAttentionConversationDirector.cpp:112 | core | Clang-confirmed in audit snapshot: cast from pointer to smaller type 's32' (aka 'int') loses information | review needed |
| src/Game/System/Overwrite.cpp:157 | core | Clang-confirmed in audit snapshot: cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | MacioGalaxy has a fix: src/Game/System/Overwrite.cpp |
| src/Game/System/OverwriteCharWriter.cpp:19 | core | 32-bit cast of a declared pointer/array/address expression; operand: this | review needed |
| src/Game/System/OverwriteCharWriter.cpp:20 | core | 32-bit cast of a declared pointer/array/address expression; operand: this | review needed |
| src/Game/System/OverwriteCharWriter.cpp:21 | core | 32-bit cast of a declared pointer/array/address expression; operand: this | review needed |
| src/Game/System/OverwriteCharWriter.cpp:22 | core | 32-bit cast of a declared pointer/array/address expression; operand: this | review needed |
| src/Game/System/OverwriteCharWriter.cpp:23 | core | 32-bit cast of a declared pointer/array/address expression; operand: this | review needed |
| src/Game/System/OverwriteCharWriter.cpp:24 | core | 32-bit cast of a declared pointer/array/address expression; operand: this | review needed |
| src/Game/System/OverwriteCharWriter.cpp:25 | core | 32-bit cast of a declared pointer/array/address expression; operand: this | review needed |
| src/Game/System/OverwriteCharWriter.cpp:29 | core | 32-bit cast of a declared pointer/array/address expression; operand: this | review needed |
| src/Game/System/OverwriteCharWriter.cpp:30 | core | 32-bit cast of a declared pointer/array/address expression; operand: this | review needed |
| src/Game/System/OverwriteCharWriter.cpp:31 | core | 32-bit cast of a declared pointer/array/address expression; operand: this | review needed |
| src/Game/System/OverwriteCharWriter.cpp:32 | core | 32-bit cast of a declared pointer/array/address expression; operand: this | review needed |
| src/Game/System/OverwriteCharWriter.cpp:33 | core | 32-bit cast of a declared pointer/array/address expression; operand: this | review needed |
| src/Game/System/OverwriteCharWriter.cpp:34 | core | 32-bit cast of a declared pointer/array/address expression; operand: this | review needed |
| src/Game/System/OverwriteCharWriter.cpp:35 | core | 32-bit cast of a declared pointer/array/address expression; operand: this | review needed |
| src/Game/System/RSO.cpp:60 | core | 32-bit cast of a declared pointer/array/address expression; operand: RSOFindExportSymbolAddr(rsoPtr, pTbl->symbol_name) | review needed |
| src/JSystem/J3DGraphLoader/J3DAnmLoader.cpp:276 | m1 | Clang-confirmed in audit snapshot: cast from pointer to smaller type 's32' (aka 'int') loses information | review needed |
| src/JSystem/J3DGraphLoader/J3DAnmLoader.cpp:278 | m1 | Clang-confirmed in audit snapshot: cast from pointer to smaller type 's32' (aka 'int') loses information | review needed |
| src/JSystem/J3DGraphLoader/J3DAnmLoader.cpp:474 | m1 | Clang-confirmed in audit snapshot: cast from pointer to smaller type 's32' (aka 'int') loses information | review needed |
| src/JSystem/J3DGraphLoader/J3DAnmLoader.cpp:476 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: dst->mAnmVtxColorIndexData[1][i].mpData | review needed |
| src/JSystem/JAudio2/JAIStream.cpp:58 | m1 | Clang-confirmed in audit snapshot: cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | review needed |
| src/JSystem/JAudio2/JASAramStream.cpp:625 | m1 | Clang-confirmed in audit snapshot: cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | review needed |
| src/JSystem/JAudio2/JASAramStream.cpp:640 | m1 | Clang-confirmed in audit snapshot: cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | review needed |
| src/JSystem/JAudio2/JASAramStream.cpp:645 | m1 | Clang-confirmed in audit snapshot: cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | review needed |
| src/JSystem/JKernel/JKRAramArchive.cpp:266 | m1 | Clang-confirmed in audit snapshot: cast from pointer to smaller type 's32' (aka 'int') loses information | review needed |
| src/JSystem/JKernel/JKRAramArchive.cpp:302 | m1 | Clang-confirmed in audit snapshot: cast from pointer to smaller type 's32' (aka 'int') loses information | review needed |
| src/JSystem/JKernel/JKRAramPiece.cpp:89 | m1 | Clang-confirmed in audit snapshot: cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | review needed |
| src/JSystem/JKernel/JKRDecomp.cpp:204 | m1 | Clang-confirmed in audit snapshot: cast from pointer to smaller type 's32' (aka 'int') loses information | MacioGalaxy has a fix: src/JSystem/JKernel/JKRDecomp.cpp |
| src/JSystem/JKernel/JKRExpHeap.cpp:849 | m1 | Clang-confirmed in audit snapshot: cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | review needed |
| src/JSystem/JKernel/JKRExpHeap.cpp:964 | m1 | Clang-confirmed in audit snapshot: cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | review needed |
| src/JSystem/JKernel/JKRExpHeap.cpp:967 | m1 | Clang-confirmed in audit snapshot: cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | review needed |
| src/JSystem/JKernel/JKRHeap.cpp:78 | m1 | Clang-confirmed in audit snapshot: cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | review needed |
| src/JSystem/JKernel/JKRHeap.cpp:79 | m1 | Clang-confirmed in audit snapshot: cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | review needed |
| src/JSystem/JKernel/JKRUnitHeap.cpp:51 | m1 | Clang-confirmed in audit snapshot: cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | review needed |
| src/JSystem/JSupport/JSUMemoryStream.cpp:17 | m1 | Clang-confirmed in audit snapshot: cast from pointer to smaller type 'int' loses information | review needed |
| src/JSystem/JSupport/JSUMemoryStream.cpp:56 | m1 | Clang-confirmed in audit snapshot: cast from pointer to smaller type 'int' loses information | review needed |
| src/MSL_C/mem.c:10 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: pSource | review needed |
| src/MSL_C/string.c:15 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: destb | review needed |
| src/MSL_C/string.c:117 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: right | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Os/dolphin/dolphin_trk.c:288 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: buff | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Os/dolphin/dolphin_trk.c:306 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: buff | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:1171 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: ConvertAddress(*(u32*)(thread + 0x2FC)) | review needed |
| src/MetroTRK/gamedev/cust_connection/cc/exi2/GCN/EXI2_GDEV_GCN/main.c:82 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: bytes | review needed |
| src/RVLFaceLib/RFL_MiddleDatabase.c:132 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: db | review needed |
| src/RVLFaceLib/RFL_MiddleDatabase.c:239 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: db | review needed |
| src/RVLFaceLib/RFL_MiddleDatabase.c:307 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: db | review needed |
| src/RVLFaceLib/RFL_MiddleDatabase.c:414 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: db | review needed |
| src/RVL_SDK/aralt/aralt.c:69 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: memLo | review needed |
| src/RVL_SDK/aralt/aralt.c:71 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: memLo | review needed |
| src/RVL_SDK/aralt/aralt.c:72 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: memLo | review needed |
| src/RVL_SDK/aralt/aralt.c:138 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __ARQRequestPendingHi | review needed |
| src/RVL_SDK/aralt/aralt.c:142 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __ARQRequestPendingLo | review needed |
| src/RVL_SDK/bte/bte_main.c:96 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __BTUInterruptHandlerStack | review needed |
| src/RVL_SDK/bte/uusb_ppc.c:238 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: (__uusb_ppc_stack1 + 4096) | review needed |
| src/RVL_SDK/bte/uusb_ppc.c:287 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: (__uusb_ppc_stack2 + 4096) | review needed |
| src/RVL_SDK/dsp/dsp_task.c:14 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: pCurrent->dram_mem_addr | review needed |
| src/RVL_SDK/dsp/dsp_task.c:30 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: pNext->iram_mmem_addr | review needed |
| src/RVL_SDK/dsp/dsp_task.c:50 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: pNext->dram_mem_addr | review needed |
| src/RVL_SDK/dsp/dsp_task.c:68 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: (pTask->iram_mmem_addr) | review needed |
| src/RVL_SDK/dsp/dsp_task.c:88 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: (pTask->iram_mmem_addr) | review needed |
| src/RVL_SDK/dvd/dvd.c:723 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: BootGameInfo->gamePartition | review needed |
| src/RVL_SDK/dvd/dvd.c:726 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: BootGameInfo->gamePartition | review needed |
| src/RVL_SDK/dvd/dvd.c:729 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: BootGameInfo->gamePartition | review needed |
| src/RVL_SDK/dvd/dvd.c:781 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: BootGameInfo->gamePartition | review needed |
| src/RVL_SDK/dvd/dvd.c:784 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: BootGameInfo->gamePartition | review needed |
| src/RVL_SDK/dvd/dvd.c:787 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: BootGameInfo->gamePartition | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:57 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: low | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:58 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: low | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:61 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: low | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:65 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: low | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:317 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: eTicket | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:322 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: certificates | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:327 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: tmd | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:378 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: (eTicket) | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:382 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: (certificates) | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:388 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: (tmd) | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:395 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: (eTicket) | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:437 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: certificates | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:444 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: tmd | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:452 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: eTicketView | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:614 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: (certificates) | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:615 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: (h3HashPtr) | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:1196 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: destAddr | review needed |
| src/RVL_SDK/dvd/dvdfs.c:31 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: &((type *)0)->memb | review needed |
| src/RVL_SDK/esp/esp.c:46 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: pTicketView | review needed |
| src/RVL_SDK/esp/esp.c:74 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: ticketViewList | review needed |
| src/RVL_SDK/esp/esp.c:121 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: ticketView | review needed |
| src/RVL_SDK/esp/esp.c:154 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: tmd | review needed |
| src/RVL_SDK/esp/esp.c:198 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: tmdView | review needed |
| src/RVL_SDK/esp/esp.c:247 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: dataDir | review needed |
| src/RVL_SDK/esp/esp.c:298 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: entries | review needed |
| src/RVL_SDK/exi/EXIBios.c:221 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: buffer | review needed |
| src/RVL_SDK/fs/fs.c:56 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __devfs | review needed |
| src/RVL_SDK/fs/fs.c:70 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __devfs | review needed |
| src/RVL_SDK/fs/fs.c:285 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: nameList | review needed |
| src/RVL_SDK/fs/fs.c:301 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: dnPtr | review needed |
| src/RVL_SDK/fs/fs.c:344 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: nameList | review needed |
| src/RVL_SDK/fs/fs.c:364 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: dnPtr | review needed |
| src/RVL_SDK/fs/fs.c:623 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: dnPtr | review needed |
| src/RVL_SDK/fs/fs.c:775 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: stats | review needed |
| src/RVL_SDK/fs/fs.c:808 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: stats | review needed |
| src/RVL_SDK/fs/fs.c:854 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: pBuffer | review needed |
| src/RVL_SDK/fs/fs.c:865 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: buf | review needed |
| src/RVL_SDK/fs/fs.c:888 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: buf | review needed |
| src/RVL_SDK/fs/fs.c:902 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: buf | review needed |
| src/RVL_SDK/gd/GDBase.c:18 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __GDCurrentDL->ptr | review needed |
| src/RVL_SDK/gx/GXDisplayList.c:79 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: list | review needed |
| src/RVL_SDK/gx/GXFifo.c:113 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: base | review needed |
| src/RVL_SDK/gx/GXFifo.c:166 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: CPUFifo.top | review needed |
| src/RVL_SDK/gx/GXFifo.c:167 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: GPFifo.top | review needed |
| src/RVL_SDK/gx/GXFifo.c:320 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: CPUFifo.rdPtr | review needed |
| src/RVL_SDK/gx/GXMisc.c:213 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: OSPhysicalToUncached(0x8000000) | review needed |
| src/RVL_SDK/gx/GXMisc.c:221 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: OSPhysicalToUncached(0x8000000) | review needed |
| src/RVL_SDK/ipc/ipcclt.c:111 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: req->args.read.outPtr | review needed |
| src/RVL_SDK/ipc/ipcclt.c:120 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: req->args.ioctl.outPtr | review needed |
| src/RVL_SDK/ipc/ipcclt.c:129 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: req->args.ioctlv.vector | review needed |
| src/RVL_SDK/ipc/memory.c:71 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p | review needed |
| src/RVL_SDK/ipc/memory.c:111 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p | review needed |
| src/RVL_SDK/ipc/memory.c:128 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p | review needed |
| src/RVL_SDK/ipc/memory.c:192 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: h->base | review needed |
| src/RVL_SDK/nand/nand.c:303 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: (b->absPath) | review needed |
| src/RVL_SDK/nand/nand.c:319 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: (block->absPath) | review needed |
| src/RVL_SDK/nwc24/NWC24Manage.c:63 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: pWork | review needed |
| src/RVL_SDK/os/OS.c:279 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: base | review needed |
| src/RVL_SDK/os/OS.c:286 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: OSGetArenaLo() | review needed |
| src/RVL_SDK/os/OS.c:290 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: OSGetArenaLo() | review needed |
| src/RVL_SDK/os/OS.c:293 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __OSRebootParams.regionStart | review needed |
| src/RVL_SDK/os/OS.c:294 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __OSRebootParams.regionStart | review needed |
| src/RVL_SDK/os/OS.c:295 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: OSGetArenaLo() | review needed |
| src/RVL_SDK/os/OS.c:298 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: OSGetArenaLo() | review needed |
| src/RVL_SDK/os/OS.c:300 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __OSRebootParams.regionEnd | review needed |
| src/RVL_SDK/os/OS.c:301 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __OSRebootParams.regionEnd | review needed |
| src/RVL_SDK/os/OS.c:311 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: OSGetMEM2ArenaLo() | review needed |
| src/RVL_SDK/os/OS.c:315 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: OSGetMEM2ArenaLo() | review needed |
| src/RVL_SDK/os/OS.c:318 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __OSRebootParams.regionStart | review needed |
| src/RVL_SDK/os/OS.c:319 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __OSRebootParams.regionStart | review needed |
| src/RVL_SDK/os/OS.c:320 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: OSGetMEM2ArenaLo() | review needed |
| src/RVL_SDK/os/OS.c:323 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: OSGetMEM2ArenaLo() | review needed |
| src/RVL_SDK/os/OS.c:325 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __OSRebootParams.regionEnd | review needed |
| src/RVL_SDK/os/OS.c:326 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __OSRebootParams.regionEnd | review needed |
| src/RVL_SDK/os/OS.c:494 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: bi2StartAddr | review needed |
| src/RVL_SDK/os/OS.c:495 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: bi2StartAddr | review needed |
| src/RVL_SDK/os/OS.c:548 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: arenaAddr | review needed |
| src/RVL_SDK/os/OSCache.c:281 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: pDest | review needed |
| src/RVL_SDK/os/OSCache.c:282 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: pSrc | review needed |
| src/RVL_SDK/os/OSContext.c:490 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p | review needed |
| src/RVL_SDK/os/OSError.c:47 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: stackPtr | review needed |
| src/RVL_SDK/os/OSExec.c:182 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __OSExecParamsAddr | review needed |
| src/RVL_SDK/os/OSExec.c:298 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: gameToc->partitionInfos | review needed |
| src/RVL_SDK/os/OSExec.c:348 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: bootGameInfo->gamePartition | review needed |
| src/RVL_SDK/os/OSExec.c:352 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: bootGameInfo->gamePartition | review needed |
| src/RVL_SDK/os/OSExec.c:354 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: bootGameInfo->gamePartition | review needed |
| src/RVL_SDK/os/OSExec.c:480 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: bootGameInfo->gamePartition | review needed |
| src/RVL_SDK/os/OSExec.c:482 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: bootGameInfo->gamePartition | review needed |
| src/RVL_SDK/os/OSExec.c:582 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: tgcHeader | review needed |
| src/RVL_SDK/os/OSExec.c:720 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: argv[1] | review needed |
| src/RVL_SDK/os/OSFatal.c:226 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: OSGetArenaHi() | review needed |
| src/RVL_SDK/os/OSLaunch.c:292 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: bi2 | review needed |
| src/RVL_SDK/os/OSLaunch.c:422 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: bi2 | review needed |
| src/RVL_SDK/os/OSPlayTime.c:71 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: buffer | review needed |
| src/RVL_SDK/os/OSPlayTime.c:186 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: arenaHi | review needed |
| src/RVL_SDK/os/OSPlayTime.c:189 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: arenaHi | review needed |
| src/RVL_SDK/os/OSPlayTime.c:192 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: stack | review needed |
| src/RVL_SDK/os/OSPlayTime.c:233 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: (ticket) | review needed |
| src/RVL_SDK/os/OSThread.c:400 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: stack | review needed |
| src/RVL_SDK/os/OSThread.c:409 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: param | review needed |
| src/RVL_SDK/os/OSThread.c:411 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: stack | review needed |
| src/RVL_SDK/rso/RSOLink.c:60 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: newModule | review needed |
| src/RVL_SDK/rso/RSOLink.c:61 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: newModule | review needed |
| src/RVL_SDK/rso/RSOLink.c:62 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: newModule | review needed |
| src/RVL_SDK/rso/RSOLink.c:63 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: newModule | review needed |
| src/RVL_SDK/rso/RSOLink.c:64 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: newModule | review needed |
| src/RVL_SDK/rso/RSOLink.c:65 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: newModule | review needed |
| src/RVL_SDK/rso/RSOLink.c:66 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: newModule | review needed |
| src/RVL_SDK/rso/RSOLink.c:67 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: newModule | review needed |
| src/RVL_SDK/rso/RSOLink.c:72 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: newModule | review needed |
| src/RVL_SDK/rso/RSOLink.c:75 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: bss | review needed |
| src/RVL_SDK/rso/RSOLink.c:92 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: newModule | review needed |
| src/RVL_SDK/rso/RSOLink.c:102 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: newModule | review needed |
| src/RVL_SDK/rso/RSOLink.c:123 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: moduleHeader | review needed |
| src/RVL_SDK/rso/RSOLink.c:124 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: moduleHeader | review needed |
| src/RVL_SDK/rso/RSOLink.c:125 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: moduleHeader | review needed |
| src/RVL_SDK/rso/RSOLink.c:126 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: moduleHeader | review needed |
| src/RVL_SDK/rso/RSOLink.c:127 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: moduleHeader | review needed |
| src/RVL_SDK/rso/RSOLink.c:128 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: moduleHeader | review needed |
| src/RVL_SDK/rso/RSOLink.c:129 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: moduleHeader | review needed |
| src/RVL_SDK/rso/RSOLink.c:130 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: moduleHeader | review needed |
| src/RVL_SDK/rso/RSOLink.c:137 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: _f_init | review needed |
| src/RVL_SDK/rso/RSOLink.c:140 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: _f_text | review needed |
| src/RVL_SDK/rso/RSOLink.c:155 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: _f_rodata | review needed |
| src/RVL_SDK/rso/RSOLink.c:158 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: _f_data | review needed |
| src/RVL_SDK/rso/RSOLink.c:161 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: _f_bss | review needed |
| src/RVL_SDK/rso/RSOLink.c:164 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: _f_sdata | review needed |
| src/RVL_SDK/rso/RSOLink.c:167 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: _f_sbss | review needed |
| src/RVL_SDK/rso/RSOLink.c:170 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: _f_sdata2 | review needed |
| src/RVL_SDK/rso/RSOLink.c:173 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: _f_sbss2 | review needed |
| src/RVL_SDK/rso/RSOLink.c:192 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: moduleHeader | review needed |
| src/RVL_SDK/rso/RSOLink.c:447 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p | review needed |
| src/RVL_SDK/rso/RSOLink.c:450 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p | review needed |
| src/RVL_SDK/rso/RSOLink.c:458 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p | review needed |
| src/RVL_SDK/rso/RSOLink.c:499 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p | review needed |
| src/RVL_SDK/rso/RSOLink.c:640 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p | review needed |
| src/RVL_SDK/rso/RSOLink.c:643 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p | review needed |
| src/RVL_SDK/rso/RSOLink.c:646 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p | review needed |
| src/RVL_SDK/rso/RSOLink.c:687 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: RSOGetExportSymbolAddr(i_rsoExp, i) | review needed |
| src/RVL_SDK/rso/RSOLink.c:720 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: RSOGetExportSymbolAddr(i_rsoExp, a_idx) | review needed |
| src/RVL_SDK/thp/THPDec.c:303 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __THPWorkArea | review needed |
| src/RVL_SDK/thp/THPDec.c:434 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __THPInfo->c | review needed |
| src/RVL_SDK/thp/THPDec.c:435 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __THPInfo->c | review needed |
| src/RVL_SDK/thp/THPDec.c:917 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: &(h->maxCode) | review needed |
| src/RVL_SDK/thp/THPDec.c:1011 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: &(h->maxCode) | review needed |
| src/RVL_SDK/thp/THPDec.c:1085 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: &(h->maxCode) | review needed |
| src/RVL_SDK/thp/THPDec.c:1672 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: &(h->maxCode) | review needed |
| src/RVL_SDK/thp/THPDec.c:1796 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: &(h->maxCode) | review needed |
| src/RVL_SDK/thp/THPDec.c:1831 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: &(h->maxCode) | review needed |
| src/RVL_SDK/tpl/TPL.c:18 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: (ptr->descriptorArray[i].textureHeader->data) | review needed |
| src/RVL_SDK/tpl/TPL.c:27 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: (ptr->descriptorArray[i].CLUTHeader->data) | review needed |
| src/RVL_SDK/usb/usb.c:106 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: lo | review needed |
| src/RVL_SDK/usb/usb.c:112 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: lo | review needed |
| src/RVL_SDK/usb/usb.c:290 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: deviceList | review needed |
| src/RVL_SDK/usb/usb.c:531 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: data | review needed |
| src/RVL_SDK/vf/nand_drv.c:458 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_disk | review needed |
| src/RVL_SDK/vf/pdm_disk.c:62 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_disk | review needed |
| src/RVL_SDK/vf/pdm_disk.c:121 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_disk | review needed |
| src/RVL_SDK/vf/pdm_disk.c:123 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_disk | review needed |
| src/RVL_SDK/vf/pdm_disk.c:127 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_disk | review needed |
| src/RVL_SDK/vf/pdm_disk.c:188 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_disk | review needed |
| src/RVL_SDK/vf/pdm_disk.c:237 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_disk | review needed |
| src/RVL_SDK/vf/pdm_disk.c:256 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_disk | review needed |
| src/RVL_SDK/vf/pdm_disk.c:296 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_disk | review needed |
| src/RVL_SDK/vf/pdm_disk.c:323 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_disk | review needed |
| src/RVL_SDK/vf/pdm_disk.c:350 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_disk | review needed |
| src/RVL_SDK/vf/pdm_disk.c:376 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_disk | review needed |
| src/RVL_SDK/vf/pdm_disk.c:402 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_disk | review needed |
| src/RVL_SDK/vf/pdm_disk.c:421 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_disk | review needed |
| src/RVL_SDK/vf/pdm_disk.c:440 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_disk | review needed |
| src/RVL_SDK/vf/pdm_disk.c:464 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_disk | review needed |
| src/RVL_SDK/vf/pdm_disk.c:488 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_disk | review needed |
| src/RVL_SDK/vf/pdm_disk.c:501 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_disk | review needed |
| src/RVL_SDK/vf/pdm_mbr.c:139 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_disk | review needed |
| src/RVL_SDK/vf/pdm_partition.c:12 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_part | review needed |
| src/RVL_SDK/vf/pdm_partition.c:33 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_part | review needed |
| src/RVL_SDK/vf/pdm_partition.c:34 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_part | review needed |
| src/RVL_SDK/vf/pdm_partition.c:35 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_part | review needed |
| src/RVL_SDK/vf/pdm_partition.c:41 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_part | review needed |
| src/RVL_SDK/vf/pdm_partition.c:296 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_part | review needed |
| src/RVL_SDK/vf/pdm_partition.c:335 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_part | review needed |
| src/RVL_SDK/vf/pdm_partition.c:389 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_part | review needed |
| src/RVL_SDK/vf/pdm_partition.c:426 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_part | review needed |
| src/RVL_SDK/vf/pdm_partition.c:457 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_part | review needed |
| src/RVL_SDK/vf/pdm_partition.c:501 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_part | review needed |
| src/RVL_SDK/vf/pdm_partition.c:544 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_part | review needed |
| src/RVL_SDK/vf/pdm_partition.c:577 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_part | review needed |
| src/RVL_SDK/vf/pdm_partition.c:599 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_part | review needed |
| src/RVL_SDK/vf/pdm_partition.c:623 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_part | review needed |
| src/RVL_SDK/vf/pdm_partition.c:647 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_part | review needed |
| src/RVL_SDK/vf/pdm_partition.c:671 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_part | review needed |
| src/RVL_SDK/vf/pdm_partition.c:699 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_part | review needed |
| src/RVL_SDK/vf/pdm_partition.c:705 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_part | review needed |
| src/RVL_SDK/vf/pf_cache.c:135 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_buf | review needed |
| src/RVL_SDK/vf/pf_clib.c:18 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: src | review needed |
| src/RVL_SDK/vf/pf_clib.c:38 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: d | review needed |
| src/RVL_SDK/vf/pf_volume.c:496 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: p_drv->cache->pages | review needed |
| src/RVL_SDK/vf/pf_w_clib.c:11 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: s | review needed |
| src/RVL_SDK/vi/vi.c:44 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: (fb) | review needed |
| src/RVL_SDK/vi/vi.c:1229 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: fb | review needed |
| src/RVL_SDK/wpad/WPAD.c:806 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: (__WPADiManageHandlerStack + sizeof(__WPADiManageHandlerStack)) | review needed |
| src/RVL_SDK/wud/WUD.c:918 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __WUDHandlerStack | review needed |
| src/RVL_SDK/wud/WUD.c:1055 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __WUDHandlerStack | review needed |
| src/RVL_SDK/wud/WUD.c:1135 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __WUDHandlerStack | review needed |
| src/RVL_SDK/wud/WUD.c:1349 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __WUDHandlerStack | review needed |
| src/RVL_SDK/wud/WUD.c:1393 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: __WUDHandlerStack | review needed |
| src/Runtime/__va_arg.c:17 | m1 | 32-bit cast of a declared pointer/array/address expression; operand: (addr) | review needed |
| src/nw4r/db/db_assert.cpp:55 | m1 | Clang-confirmed in audit snapshot: cast from pointer to smaller type 'u32' (aka 'unsigned int') loses information | review needed |

### union-pointer-alias — high (4)

| File:line | Lane | Evidence | Reference |
| --- | --- | --- | --- |
| include/Game/LiveActor/ActorParam.hpp:16 | core | Anonymous union contains native pointers and 32-bit value aliases; clear/write the full native slot | review needed |
| include/Game/NPC/TalkMessageCtrl.hpp:71 | enemies | Anonymous union contains native pointers and 32-bit value aliases; clear/write the full native slot | review needed |
| libs/RVLFaceLib/include/RFLi_NANDAccess.h:19 | m1 | Anonymous union contains native pointers and 32-bit value aliases; clear/write the full native slot | review needed |
| libs/RVL_SDK/include/revolution/nwc24/NWC24Types.h:138 | m1 | Anonymous union contains native pointers and 32-bit value aliases; clear/write the full native slot | review needed |

### bitfield-layout — review (21)

| File:line | Lane | Evidence | Reference |
| --- | --- | --- | --- |
| include/Game/LiveActor/Binder.hpp:68 | core | runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer; record: <anonymous>; fields: _0:1, _1:1, _2:1, _3:1, _4:1, _5:1 | review needed |
| include/Game/MapObj/StarPiece.hpp:138 | objects | runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer; record: <anonymous>; fields: isGoToPlayer:1, _1:1, isDynamicShadow:1, isShadowDisabled:1, isGroup:1, isInWater:1, isLightEffect:1, isLaunched:1 | review needed |
| include/Game/Player/MarioActor.hpp:807 | astra | runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer; record: <anonymous>; fields: mSmoke:1, mSnow:1, _2:1, mSlopeSlip:1, mIsStationary:1, mBeeWind:1, mSpecialSmoke:1 | MacioGalaxy has a fix: include/Game/Player/MarioActor.hpp |
| include/Game/Player/RushEndInfo.hpp:21 | astra | runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer; record: <anonymous>; fields: _8:24, mDamageType:4, _0:4, _0:4, mDamageType:4, _8:24 | review needed |
| libs/JSystem/include/JSystem/JAudio2/JAISound.hpp:246 | m1 | runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer; record: <anonymous>; fields: mute:1, paused:1, flag3:1, flag4:1, flag5:1, flag6:1, flag7:1, flag8:1 | review needed |
| libs/JSystem/include/JSystem/JAudio2/JAISound.hpp:259 | m1 | runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer; record: <anonymous>; fields: comesBack:1, hasLifeTime:1, pauseWhenOut:1, flag4:1, flag5:1, flag6:1, flag7:1, flag8:1 | review needed |
| libs/JSystem/include/JSystem/JAudio2/JAISound.hpp:272 | m1 | runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer; record: <anonymous>; fields: flag1:1, calcedOnce:1, animationState:2, flag5:1, flag6:1, flag7:1, flag8:1 | review needed |
| libs/JSystem/include/JSystem/JAudio2/JAISound.hpp:378 | m1 | runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer; record: <anonymous>; fields: muted:1, paused:1, flag3:1, flag4:1, flag5:1, flag6:1, flag7:1, flag8:1 | review needed |
| libs/JSystem/include/JSystem/JAudio2/JASChannel.hpp:78 | m1 | runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer; record: <anonymous>; fields: lower1:4, lower0:4, upper:8 | review needed |
| libs/JSystem/include/JSystem/JAudio2/JASTrack.hpp:397 | m1 | runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer; record: <anonymous>; fields: mPauseFlag:1, mIsMute:1, mIsDirectlyPlayed:1, mIsOwnedByParent:1, mReadyToPlay:1, mInvalidateSeq:1, mIsStopped:1, flag7:1 | review needed |
| libs/JSystem/include/JSystem/JAudio2/JAUAudibleParam.hpp:16 | m1 | runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer; record: <anonymous>; fields: b0_0:4, b0_4:1, b0_5:1, b0_6:1, b0_7:1, b1_0:1, b1_1:1, b1_2_7:6 | review needed |
| libs/MSL_C/include/file_struct.h:15 | m1 | runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer; record: <anonymous>; fields: open:2, io:3, buffer:2, file:3, file_orientation:2, binary:1 | review needed |
| libs/MSL_C/include/file_struct.h:26 | m1 | runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer; record: <anonymous>; fields: io_state:3, free_buffer:1 | review needed |
| libs/RVL_SDK/include/revolution/mem/expHeap.h:19 | m1 | runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer; record: <anonymous>; fields: allocDir:1, alignment:7, groupID:8 | review needed |
| libs/RVL_SDK/include/revolution/mem/expHeap.h:48 | m1 | runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer; record: <anonymous>; fields: _reserved:14, useMarginOfAlign:1, allocMode:1 | review needed |
| libs/RVL_SDK/include/revolution/mem/heapCommon.h:26 | m1 | runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer; record: <anonymous>; fields: reserved:24, optFlag:8 | review needed |
| libs/nw4r/include/nw4r/lyt/drawInfo.h:33 | m1 | runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer; record: <anonymous>; fields: mulViewDraw:1, influencedAlpha:1, locationAdjust:1, invisiblePaneCalculateMtx:1 | review needed |
| libs/nw4r/include/nw4r/lyt/material.h:16 | m1 | runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer; record: BitGXNums; fields: texMap:4, texSRT:4, texCoordGen:4, indSRT:2, indStage:3, tevSwap:1, tevStage:5, chanCtrl:1, matCol:1, alpComp:1, blendMode:1 | review needed |
| libs/nw4r/include/nw4r/lyt/texMap.h:179 | m1 | runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer; record: <anonymous>; fields: textureFormat:4, mipmap:1, wrapS:2, wrapT:2, minFilter:3, magFilter:1, biasClampEnable:1, edgeLODEnable:1, anisotropy:2, paletteFormat:2 | review needed |
| libs/nw4r/include/nw4r/lyt/textBox.h:97 | m1 | runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer; record: <anonymous>; fields: bAllocFont:1, textAlignment:2 | review needed |
| src/RVLFaceLib/RFL_MiddleDatabase.c:17 | m1 | runtime bitfield: only hazardous if persisted, overlaid, or shared with a packed integer; record: HiddenNewOldParam; fields: sex:2, srcIdx:15, dstIdx:15 | review needed |

### cast-unresolved — review (1053)

| File:line | Lane | Evidence | Reference |
| --- | --- | --- | --- |
| include/Game/Animation/MaterialAnmBuffer.hpp:17 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Animation/XanimeResource.hpp:110 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Animation/XanimeResource.hpp:112 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/AudioLib/AudBgm.hpp:17 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: = 0 | review needed |
| include/Game/AudioLib/AudMultiBgmInfo.hpp:22 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/AudioLib/AudSoundInfo.hpp:11 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const NO_INLINE | review needed |
| include/Game/Boss/BossKameckMoveRail.hpp:12 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Boss/BossKameckMoveRail.hpp:13 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Boss/BossStinkBug.hpp:49 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Boss/BossStinkBugActionBase.hpp:37 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Boss/BossStinkBugActionBase.hpp:40 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Boss/DinoPackun.hpp:45 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Boss/DinoPackunStateDamage.hpp:15 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Boss/DinoPackunTail.hpp:28 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Boss/KoopaBattle.hpp:32 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Boss/KoopaBattleAccessor.hpp:30 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Boss/KoopaBattleAccessor.hpp:32 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Boss/KoopaJrRobotTU14.hpp:39 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Boss/Sandy.hpp:359 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Boss/SkeletalFishBossInfo.hpp:36 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Boss/SkeletalFishBossRail.hpp:13 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Boss/SkeletalFishBossRail.hpp:14 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Boss/SkeletalFishBossRailHolder.hpp:12 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Boss/SkeletalFishBossRailHolder.hpp:13 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Boss/TripodBoss.hpp:77 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Boss/TripodBossAccesser.hpp:54 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/Camera/CameraHolder.hpp:15 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Camera/CameraHolder.hpp:16 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Camera/CameraHolder.hpp:17 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Camera/CameraHolder.hpp:26 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | MacioGalaxy has a fix: include/Game/Camera/CameraHolder.hpp |
| include/Game/Camera/CameraTargetObj.hpp:81 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Camera/CameraTargetObj.hpp:170 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Demo/DemoCastGroupHolder.hpp:16 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Demo/DemoDirector.hpp:21 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Effect/MultiEmitter.hpp:43 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Enemy/ActionSoundData.hpp:9 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Enemy/AnimScaleController.hpp:41 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Enemy/AttackRock.hpp:120 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Enemy/AttackRock.hpp:153 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Enemy/AttackRock.hpp:186 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Enemy/BattanKing.hpp:73 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Enemy/BegomanLauncher.hpp:24 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Enemy/HomingKiller.hpp:36 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/GameAudio/ActionSound.hpp:24 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/GameAudio/ActionSound.hpp:25 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/GameAudio/AudTamakoroBgmCtrl.hpp:25 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const NO_INLINE | review needed |
| include/Game/LiveActor/Binder.hpp:25 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/LiveActor/DisplayListMaker.hpp:31 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/LiveActor/EffectKeeper.hpp:37 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/LiveActor/HitSensor.hpp:15 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/LiveActor/HitSensorKeeper.hpp:24 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/LiveActor/LiveActorGroup.hpp:15 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/LiveActor/RailRider.hpp:22 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/LiveActor/RailRider.hpp:34 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/LiveActor/ShadowController.hpp:20 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/LiveActor/VolumeDrawer.hpp:18 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Map/BezierRail.hpp:40 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Map/BezierRail.hpp:45 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Map/CollisionHitInfoList.hpp:11 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Map/HitInfo.hpp:20 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Map/HitInfo.hpp:22 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Map/HitInfo.hpp:23 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Map/KCollision.hpp:105 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Map/KCollision.hpp:108 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Map/KCollision.hpp:110 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Map/LightZoneDataHolder.hpp:32 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Map/OceanRing.hpp:35 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Map/RailGraph.hpp:16 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Map/RailGraph.hpp:17 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Map/RailGraph.hpp:18 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Map/RailGraphEdge.hpp:12 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Map/RailGraphEdge.hpp:13 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Map/StageSwitch.hpp:16 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Map/StageSwitch.hpp:79 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Map/StageSwitch.hpp:80 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/BallRail.hpp:29 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/BigBubbleHolder.hpp:19 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/ChipHolder.hpp:18 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/DiceBase.hpp:29 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: = 0 | review needed |
| include/Game/MapObj/DiceBase.hpp:30 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: = 0 | review needed |
| include/Game/MapObj/DiceBase.hpp:31 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/ItemDice.hpp:21 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/ItemDice.hpp:39 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/ItemDice.hpp:51 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/ItemDice.hpp:68 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/MapPartsRailRotator.hpp:22 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/MapPartsRailRotator.hpp:24 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/MapPartsSeesaw1AxisRotator.hpp:14 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: override | review needed |
| include/Game/MapObj/MapPartsSeesaw2AxisRotator.hpp:14 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: override | review needed |
| include/Game/MapObj/MarioLauncher.hpp:48 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/PowerStarHolder.hpp:21 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/PowerStarHolder.hpp:22 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/RepeatBlock.hpp:143 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/RepeatBlock.hpp:144 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/RepeatBlock.hpp:177 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/RepeatBlock.hpp:178 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/RepeatBlock.hpp:250 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/SnowFloor.hpp:26 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/SnowFloorTile.hpp:26 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/SpinCloudBlock.hpp:30 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/MapObj/SpinDriver.hpp:56 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/SuperSpinDriver.hpp:87 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/MapObj/WarpPod.hpp:50 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/NPC/CometEventExecutor.hpp:5 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: = 0 | review needed |
| include/Game/NPC/CometEventExecutor.hpp:12 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: = 0 | review needed |
| include/Game/NPC/CometEventExecutor.hpp:14 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: = 0 | review needed |
| include/Game/NPC/Meister.hpp:40 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/NPC/PostmanEvent.hpp:31 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/NPC/PostmanEvent.hpp:32 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/NPC/PostmanEvent.hpp:33 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/NPC/PostmanEvent.hpp:35 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/NPC/PostmanEvent.hpp:66 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/NPC/PostmanEvent.hpp:67 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/NPC/PostmanEvent.hpp:68 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/NPC/PostmanEvent.hpp:69 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/NPC/PostmanEvent.hpp:73 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/NPC/PostmanEvent.hpp:74 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/NPC/PostmanEvent.hpp:75 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/NPC/PostmanEvent.hpp:76 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/NPC/TalkMessageFunc.hpp:8 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/NPC/TalkMessageFunc.hpp:39 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: > TalkMessageFunc(T* a1, bool (T::*a2)(u32)) NO_INLINE | review needed |
| include/Game/NPC/TalkMessageFunc.hpp:40 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: >(a1, a2) | review needed |
| include/Game/NPC/TalkMessageFunc.hpp:44 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: > TalkMessageFunc_Inline(T* a1, bool (T::*a2)(u32)) | review needed |
| include/Game/NPC/TalkMessageFunc.hpp:45 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: >(a1, a2) | review needed |
| include/Game/NPC/TicoRail.hpp:31 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/NameObj/NameObjArchiveListCollector.hpp:12 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/NameObj/NameObjCategoryList.hpp:55 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Player/GhostPacketData.hpp:18 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Player/Mario.hpp:393 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Player/Mario.hpp:572 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Player/Mario.hpp:660 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Player/MarioActor.hpp:439 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Player/MarioActorHelpers.hpp:88 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: = 0 | review needed |
| include/Game/Player/MarioAnimator.hpp:21 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Player/MarioAnimator.hpp:28 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Player/MarioAnimator.hpp:30 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Player/MarioModule.hpp:35 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Player/MarioModule.hpp:127 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Player/MarioShadow.hpp:35 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Player/MarioSwim.hpp:104 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/Player/MarioSwim.hpp:107 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/Player/MarioSwim.hpp:218 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/Player/MarioYoshiBalloon.hpp:20 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/Player/Yoshi.hpp:271 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Player/YoshiHeadCtrl.hpp:8 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/RhythmLib/AudMeSeqParser.hpp:9 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: AudMeSeqParser::*CommandFunc | review needed |
| include/Game/RhythmLib/AudMeTrackCallback.hpp:5 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *AudCallbackFunc | review needed |
| include/Game/Ride/SwingRope.hpp:32 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Scene/SceneObjHolder.hpp:169 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Scene/SceneObjHolder.hpp:170 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Screen/AllStarList.hpp:25 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/Screen/AllStarList.hpp:26 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/Screen/CoinCounter.hpp:23 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/Screen/GalaxyMap.hpp:58 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Screen/GalaxyMapIcon.hpp:31 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Screen/MessageTagSkipTagProcessor.hpp:58 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Screen/MessageTagSkipTagProcessor.hpp:59 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Screen/MessageTagSkipTagProcessor.hpp:60 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Screen/MessageTagSkipTagProcessor.hpp:61 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Screen/MoviePlayingSequence.hpp:89 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Screen/PostManLetter.hpp:28 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Screen/PostManLetter.hpp:29 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Screen/PostManLetter.hpp:30 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Screen/StarPointerInput.hpp:14 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Screen/StarPointerInput.hpp:15 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Screen/StarPointerLayout.hpp:153 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/Screen/StarPointerMovePositionHolder.hpp:34 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Screen/StarPointerMovePositionHolder.hpp:35 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Screen/StarPointerMovePositionHolder.hpp:61 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Screen/THPSimplePlayerWrapper.hpp:42 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const NO_INLINE | review needed |
| include/Game/Screen/WipeLayoutBase.hpp:12 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: = 0 | review needed |
| include/Game/Speaker/SpkMixingBuffer.hpp:10 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Speaker/SpkWave.hpp:29 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Speaker/SpkWave.hpp:30 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Speaker/SpkWave.hpp:31 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Speaker/SpkWave.hpp:32 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Speaker/SpkWave.hpp:33 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/DrawBufferHolder.hpp:30 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/DrawBufferHolder.hpp:31 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/FindingLuigiEventScheduler.hpp:31 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/GalaxyStatusAccessor.hpp:12 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/GalaxyStatusAccessor.hpp:23 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/GalaxyStatusAccessor.hpp:24 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/GalaxyStatusAccessor.hpp:25 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/GalaxyStatusAccessor.hpp:27 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/GalaxyStatusAccessor.hpp:28 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/GalaxyStatusAccessor.hpp:31 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/GalaxyStatusAccessor.hpp:32 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/GalaxyStatusAccessor.hpp:33 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/GalaxyStatusAccessor.hpp:34 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/GalaxyStatusAccessor.hpp:35 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/GalaxyStatusAccessor.hpp:36 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/GalaxyWorldMapInfo.hpp:66 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| include/Game/System/GameDataGalaxyStorage.hpp:16 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/GameDataGalaxyStorage.hpp:31 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/GameDataHolder.hpp:27 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/GameDataTicoFatStorage.hpp:27 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/MarioFaceShipAttentionConversationDirector.hpp:41 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const NO_INLINE | review needed |
| include/Game/System/MarioFaceShipAttentionConversationDirector.hpp:76 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const NO_INLINE | review needed |
| include/Game/System/MarioFaceShipEventDataHolder.hpp:59 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/MarioFaceShipEventDataHolder.hpp:62 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/MarioFaceShipEventDataHolder.hpp:63 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/MarioFaceShipEventDataHolder.hpp:65 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/MarioFaceShipEventDataHolder.hpp:66 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/MessageHolder.hpp:78 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/MessageHolder.hpp:79 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/MessageHolder.hpp:82 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/MessageHolder.hpp:83 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/MessageHolder.hpp:84 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/MessageHolder.hpp:85 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/ResourceHolder.hpp:19 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/ResourceInfo.hpp:32 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | MacioGalaxy has a fix: include/Game/System/ResourceInfo.hpp |
| include/Game/System/ResourceInfo.hpp:38 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | MacioGalaxy has a fix: include/Game/System/ResourceInfo.hpp |
| include/Game/System/ResourceInfo.hpp:41 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/ScenarioDataParser.hpp:20 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/ScenarioDataParser.hpp:21 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/ScenarioDataParser.hpp:23 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/ScenarioDataParser.hpp:26 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/ScenarioDataParser.hpp:55 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/UserFile.hpp:42 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/WPad.hpp:37 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/WPad.hpp:39 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/WPadHolder.hpp:12 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/WorldMapResultCommand.hpp:41 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const NO_INLINE | review needed |
| include/Game/System/WorldMapResultLocal.hpp:103 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const NO_INLINE | review needed |
| include/Game/System/WorldMapResultLocal.hpp:104 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const NO_INLINE | review needed |
| include/Game/System/WorldMapResultLocal.hpp:127 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const NO_INLINE | review needed |
| include/Game/System/WorldMapResultLocal.hpp:128 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const NO_INLINE | review needed |
| include/Game/System/WorldMapResultLocal.hpp:152 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/WorldMapResultLocal.hpp:153 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const NO_INLINE | review needed |
| include/Game/System/WorldMapResultLocal.hpp:154 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/WorldMapResultLocal.hpp:155 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/WorldMapResultLocal.hpp:159 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/WorldMapResultLocal.hpp:162 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/WorldMapResultLocal.hpp:164 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/WorldMapResultLocal.hpp:169 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/WorldMapResultLocal.hpp:170 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const NO_INLINE | review needed |
| include/Game/System/WorldMapResultLocal.hpp:171 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const NO_INLINE | review needed |
| include/Game/System/WorldMapResultLocal.hpp:173 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/WorldMapResultLocal.hpp:174 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/WorldMapResultLocal.hpp:175 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/System/WorldMapResultLocal.hpp:176 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Util/BaseMatrixFollowTargetHolder.hpp:12 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Util/BaseMatrixFollowTargetHolder.hpp:18 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Util/BaseMatrixFollowTargetHolder.hpp:37 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Util/BaseMatrixFollowTargetHolder.hpp:88 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Util/FootPrint.hpp:40 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Util/FootPrint.hpp:42 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Util/FurDrawer.hpp:28 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Util/JointRumbler.hpp:28 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mFlags & flag | review needed |
| include/Game/Util/MathUtil.hpp:51 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: max | review needed |
| include/Game/Util/MathUtil.hpp:656 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: max | review needed |
| include/Game/Util/MultiEventCamera.hpp:29 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Util/OctahedronBezierSurface.hpp:15 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Util/ParabolicPath.hpp:15 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| include/Game/Util/ValueControl.hpp:78 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (-direction & ~direction) | review needed |
| libs/JSystem/include/JSystem/J3DGraphAnimator/J3DJoint.hpp:13 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *J3DJointCallBack | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DDrawBuffer.hpp:60 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: J3DDrawBuffer::*sortFunc | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DMatBlock.hpp:422 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: = 0 | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DMatBlock.hpp:662 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DMatBlock.hpp:1710 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: = 0 | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DTevs.hpp:63 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DTevs.hpp:649 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (mColorChanID & 0x2) | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DTevs.hpp:653 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (mColorChanID & (1 << 6)) | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DTevs.hpp:661 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (mColorChanID & (3 << 7)) | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DTevs.hpp:666 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (mColorChanID & (3 << 9)) | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DTexture.hpp:59 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DTexture.hpp:63 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DTexture.hpp:64 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory.hpp:81 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory.hpp:82 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory.hpp:93 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory.hpp:96 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory.hpp:99 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory.hpp:104 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory.hpp:107 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory.hpp:112 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory.hpp:113 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory.hpp:114 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory.hpp:115 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory.hpp:116 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory.hpp:117 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory.hpp:118 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory_v21.hpp:45 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory_v21.hpp:46 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory_v21.hpp:49 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory_v21.hpp:51 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory_v21.hpp:54 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory_v21.hpp:59 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory_v21.hpp:62 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory_v21.hpp:63 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory_v21.hpp:64 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory_v21.hpp:65 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory_v21.hpp:66 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory_v21.hpp:67 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/J3DGraphLoader/J3DMaterialFactory_v21.hpp:68 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/JAudio2/JAIAudible.hpp:10 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: = 0 | review needed |
| libs/JSystem/include/JSystem/JAudio2/JAISound.hpp:448 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: = 0 | review needed |
| libs/JSystem/include/JSystem/JAudio2/JAISound.hpp:449 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: = 0 | review needed |
| libs/JSystem/include/JSystem/JAudio2/JAISound.hpp:454 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: = 0 | review needed |
| libs/JSystem/include/JSystem/JAudio2/JASBankList.hpp:11 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/JAudio2/JASBasicBank.hpp:21 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/JAudio2/JASBasicInst.hpp:84 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/JAudio2/JASBasicInst.hpp:100 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/JAudio2/JASBasicWaveBank.hpp:69 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/JAudio2/JASDSPChannel.hpp:6 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *Callback | review needed |
| libs/JSystem/include/JSystem/JAudio2/JASDriverIF.hpp:5 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *DriverCallback | review needed |
| libs/JSystem/include/JSystem/JAudio2/JASDrumSet.hpp:33 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/JAudio2/JASDrumSet.hpp:56 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/JAudio2/JASSeqParser.hpp:9 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: JASSeqParser::*CommandFunc | review needed |
| libs/JSystem/include/JSystem/JAudio2/JASSimpleWaveBank.hpp:23 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/JAudio2/JASTrack.hpp:103 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/JAudio2/JASTrackPort.hpp:14 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/JAudio2/JASTrackPort.hpp:15 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/JAudio2/JASWaveInfo.hpp:56 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/JAudio2/JASWaveInfo.hpp:57 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: = 0 | review needed |
| libs/JSystem/include/JSystem/JAudio2/JAUAudibleParam.hpp:7 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((*(u8*)&field_0x0.raw >> 4) & 0xf) | review needed |
| libs/JSystem/include/JSystem/JAudio2/JAUAudioArcInterpreter.hpp:19 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: = 0 | review needed |
| libs/JSystem/include/JSystem/JAudio2/JAUAudioArcInterpreter.hpp:21 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: = 0 | review needed |
| libs/JSystem/include/JSystem/JAudio2/JAUStreamFileTable.hpp:16 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/JGeometry/TUtil.hpp:66 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (1023.5f * -val) | review needed |
| libs/JSystem/include/JSystem/JGeometry/TUtil.hpp:69 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (1023.5f * val) | review needed |
| libs/JSystem/include/JSystem/JGeometry/TUtil.hpp:81 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (1023.5f * -val) | review needed |
| libs/JSystem/include/JSystem/JGeometry/TUtil.hpp:83 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (1023.5f * val) | review needed |
| libs/JSystem/include/JSystem/JKernel/JKRArchive.hpp:142 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/JKernel/JKRArchive.hpp:144 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/JKernel/JKRArchive.hpp:148 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/JKernel/JKRUnitHeap.hpp:35 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/JSystem/include/JSystem/JMath/JMATrigonometric.hpp:155 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (-x * 1023.5f) | review needed |
| libs/JSystem/include/JSystem/JMath/JMATrigonometric.hpp:157 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (x * 1023.5f) | review needed |
| libs/JSystem/include/JSystem/JMath/JMATrigonometric.hpp:167 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (-x * 1023.5f) | review needed |
| libs/JSystem/include/JSystem/JMath/JMATrigonometric.hpp:169 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (x * 1023.5f) | review needed |
| libs/JSystem/include/JSystem/JSupport/JSUList.hpp:167 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| libs/JSystem/include/JSystem/JUtility/JUTConsole.hpp:126 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: offsetof(JUTConsole, mListNode) | review needed |
| libs/MSL_C/include/file_struct.h:34 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *__pos_proc | review needed |
| libs/MSL_C/include/file_struct.h:35 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *__io_proc | review needed |
| libs/MSL_C/include/file_struct.h:36 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *__close_proc | review needed |
| libs/MSL_C/include/locale.h:8 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: * __decode_mbyte | review needed |
| libs/MSL_C/include/locale.h:9 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: * __encode_mbyte | review needed |
| libs/MetroTRK/include/Os/dolphin/dolphin_trk_glue.h:11 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *DBCommFunc | review needed |
| libs/MetroTRK/include/Os/dolphin/dolphin_trk_glue.h:12 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *DBCommInitFunc | review needed |
| libs/MetroTRK/include/Os/dolphin/dolphin_trk_glue.h:13 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *DBCommReadFunc | review needed |
| libs/MetroTRK/include/Os/dolphin/dolphin_trk_glue.h:14 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *DBCommWriteFunc | review needed |
| libs/RVL_SDK/include/revolution/gd/GDBase.h:10 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((0xfe << 24) \| (mask)) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDGeometry.h:173 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (nInds) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDIndirect.h:23 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDIndirect.h:26 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDIndirect.h:29 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (map3) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDIndirect.h:30 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDIndirect.h:32 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDPixel.h:93 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDPixel.h:95 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDPixel.h:97 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDPixel.h:99 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDPixel.h:101 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDPixel.h:105 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (enable_alpha_update) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDPixel.h:106 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDPixel.h:109 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDPixel.h:111 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDPixel.h:113 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTev.h:141 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (clamp) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTev.h:142 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTev.h:145 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (bias) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTev.h:146 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTev.h:148 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTev.h:150 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTev.h:153 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTev.h:156 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTev.h:158 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTev.h:160 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTev.h:163 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (coord1) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTev.h:164 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTexture.h:50 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (wrap_t) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTexture.h:51 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (min_filter) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTexture.h:52 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (lodbias) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTexture.h:53 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (lodclamp) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTexture.h:54 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (rid) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTexture.h:60 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (rid) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTexture.h:67 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (height) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTexture.h:68 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (rid) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTexture.h:72 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (rid) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTexture.h:75 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTexture.h:77 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTexture.h:80 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (lod_bias) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTexture.h:81 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTexture.h:83 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTexture.h:85 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTexture.h:87 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTexture.h:89 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTexture.h:91 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTexture.h:93 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTexture.h:96 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gd/GDTexture.h:98 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (id) | review needed |
| libs/RVL_SDK/include/revolution/gx/GXFDLShortcut.h:8 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (newFlag) | review needed |
| libs/RVL_SDK/include/revolution/gx/GXRegs.h:48 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (ui) | review needed |
| libs/RVL_SDK/include/revolution/gx/GXRegs.h:52 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (d) | review needed |
| libs/RVL_SDK/include/revolution/gx/GXRegs.h:60 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (d) | review needed |
| libs/RVL_SDK/include/revolution/gx/GXRegs.h:74 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (d) | review needed |
| libs/RVL_SDK/include/revolution/gx/GXRegs.h:82 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (d) | review needed |
| libs/RVL_SDK/include/revolution/gx/GXRegs.h:148 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (newFlag) | review needed |
| libs/RVL_SDK/include/revolution/hbm/HBMBase.h:77 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *HBMSoundCallback | review needed |
| libs/RVL_SDK/include/revolution/os.h:36 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: OS_BASE_CACHED | review needed |
| libs/RVL_SDK/include/revolution/os.h:37 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: OS_BASE_CACHED | review needed |
| libs/RVL_SDK/include/revolution/os.h:38 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: OS_BASE_UNCACHED | review needed |
| libs/RVL_SDK/include/revolution/os.h:39 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: OS_BASE_UNCACHED | review needed |
| libs/RVL_SDK/include/revolution/os.h:46 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (tick0) | MacioGalaxy has a fix: libs/RVL_SDK/include/revolution/os.h |
| libs/RVL_SDK/include/revolution/os.h:48 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: y | MacioGalaxy has a fix: libs/RVL_SDK/include/revolution/os.h |
| libs/RVL_SDK/include/revolution/types.h:138 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((c0 & 0xFF) << 24 \| (c1 & 0xFF) << 16 \| (c2 & 0xFF) << 8 \| (c3 & 0xFF)) | review needed |
| libs/RVL_SDK/include/revolution/usb.h:11 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (val) | review needed |
| libs/RVL_SDK/include/revolution/usb.h:12 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (val) | review needed |
| libs/RVL_SDK/include/revolution/vf/vf_struct.h:331 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *p_callback | review needed |
| libs/RVL_SDK/include/revolution/vf/vf_struct.h:348 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *oem2unicode | review needed |
| libs/RVL_SDK/include/revolution/vf/vf_struct.h:349 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *unicode2oem | review needed |
| libs/RVL_SDK/include/revolution/vf/vf_struct.h:350 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *oem_char_width | review needed |
| libs/RVL_SDK/include/revolution/vf/vf_struct.h:351 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *is_oem_mb_char | review needed |
| libs/RVL_SDK/include/revolution/vf/vf_struct.h:352 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *unicode_char_width | review needed |
| libs/RVL_SDK/include/revolution/vf/vf_struct.h:353 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *is_unicode_mb_char | review needed |
| libs/nw4r/include/nw4r/db/assert.h:20 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: reinterpret_cast< const void* >(exp) | review needed |
| libs/nw4r/include/nw4r/db/assert.h:21 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: reinterpret_cast< const void* >(exp) | review needed |
| libs/nw4r/include/nw4r/db/assert.h:22 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: reinterpret_cast< const void* >(exp) | review needed |
| libs/nw4r/include/nw4r/db/assert.h:23 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: reinterpret_cast< const void* >(exp) | review needed |
| libs/nw4r/include/nw4r/db/assert.h:24 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: reinterpret_cast< const void* >(exp) | review needed |
| libs/nw4r/include/nw4r/db/assert.h:25 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: reinterpret_cast< const void* >(exp) | review needed |
| libs/nw4r/include/nw4r/db/assert.h:26 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: reinterpret_cast< const void* >(exp) | review needed |
| libs/nw4r/include/nw4r/db/assert.h:31 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: exp | review needed |
| libs/nw4r/include/nw4r/db/assert.h:32 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: min | review needed |
| libs/nw4r/include/nw4r/db/assert.h:34 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: reinterpret_cast< const void* >(exp) | review needed |
| libs/nw4r/include/nw4r/lyt/common.h:100 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: static_cast< u32 >(bytes[0]) << 24 \| static_cast< u32 >(bytes[1]) << 16 \| static_cast< u32 >(bytes[2]) << 8 \|                                           bytes[3] | review needed |
| libs/nw4r/include/nw4r/lyt/pane.h:54 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/nw4r/include/nw4r/lyt/pane.h:56 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/nw4r/include/nw4r/lyt/pane.h:58 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/nw4r/include/nw4r/lyt/pane.h:72 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/nw4r/include/nw4r/lyt/textBox.h:30 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/nw4r/include/nw4r/lyt/textBox.h:32 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/nw4r/include/nw4r/lyt/window.h:34 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/nw4r/include/nw4r/lyt/window.h:36 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/nw4r/include/nw4r/lyt/window.h:41 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| libs/nw4r/include/nw4r/lyt/window.h:44 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| src/Game/AudioLib/AudSoundObject.cpp:242 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mHashDatas[idx].mID | review needed |
| src/Game/Boss/KoopaJrCastleBody.cpp:543 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (100.0f * mRate) | review needed |
| src/Game/Boss/KoopaJrCastleFunction.cpp:284 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (MR::calcDistanceToPlayer(out) / 30.0f) | review needed |
| src/Game/Boss/KoopaJrCastleHead.cpp:258 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 100.0f * ease / 25.0f | review needed |
| src/Game/Boss/KoopaJrCastleTU06.cpp:136 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (1000.0f * (1.0f - nearest / 1400.0f)) | review needed |
| src/Game/Boss/SandyHand.cpp:600 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((f32)frame / sThrowRate) | review needed |
| src/Game/Boss/SandyHand.cpp:651 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((f32)frame / sThrowRate) | review needed |
| src/Game/Boss/SandyMini.cpp:301 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((f32)shotFrame / sMiniShotRate) | review needed |
| src/Game/Boss/SandyMini.cpp:307 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((f32)shotFrame / sMiniShotRate) | review needed |
| src/Game/Boss/SandyMini.cpp:311 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((f32)endFrame / sMiniShotRate) | review needed |
| src/Game/Boss/SandyStateDamage.cpp:65 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((f32)interval * 0.5f) | review needed |
| src/Game/Boss/TwoLegsBullet.cpp:210 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 45.511112f * angle | review needed |
| src/Game/Boss/Yukkina.cpp:385 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (3.0f * (speed * speed)) | review needed |
| src/Game/Camera/CameraParallel.cpp:123 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (((roundInterval / 2) + MR::abs(mRoundAngle[mRoundIndex])) / roundInterval) | review needed |
| src/Game/Camera/CameraParallel.cpp:137 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (((roundInterval / 2) + MR::abs(mRoundAngle[mRoundIndex])) / roundInterval) | review needed |
| src/Game/Camera/CameraShaker.cpp:159 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: JUTVideo | review needed |
| src/Game/Camera/CameraTower.cpp:179 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (((::sRoundInterval / 2) + MR::abs(roundAngle)) / ::sRoundInterval) | review needed |
| src/Game/Demo/OpeningDemoObj.cpp:203 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: step / sShootingStarInterval | review needed |
| src/Game/Demo/PrepareKoopaFinalDemoObj.cpp:146 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR::getBckFrameMax(this) | review needed |
| src/Game/Demo/ScenarioStarter.cpp:51 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| src/Game/Demo/ScenarioStarter.cpp:391 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: duration / TWO_PI | review needed |
| src/Game/Demo/StaffRollDemoObj.cpp:127 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR::getBvaFrameMax(sky) | review needed |
| src/Game/Demo/StaffRollDemoObj.cpp:157 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR::getBvaFrameMax(sky) | review needed |
| src/Game/Enemy/AttackRockFinal.cpp:247 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| src/Game/Enemy/AttackRockRide.cpp:75 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (angle / 3.0f) | review needed |
| src/Game/Enemy/Jugem.cpp:1380 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 100.0f * PSVECMag(&mHost->mVelocity) | review needed |
| src/Game/Enemy/Kameck.cpp:657 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mRailNextPointCoord / 20.0f | review needed |
| src/Game/Enemy/KickStone.cpp:95 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR | review needed |
| src/Game/Enemy/KirairaChain.cpp:137 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (MR::getRailCoord(mHost) - offset) / 40.0f | review needed |
| src/Game/Enemy/KirairaChain.cpp:138 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (offset + MR::getRailCoord(mHost)) / 40.0f | review needed |
| src/Game/Enemy/MechanicKoopaMini.cpp:208 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR | review needed |
| src/Game/Enemy/Menbo.cpp:220 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR::getBckFrameMax(this) | review needed |
| src/Game/Enemy/Menbo.cpp:261 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR::getBckFrameMax(this) | review needed |
| src/Game/Enemy/Menbo.cpp:342 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR::getBckFrameMax(this) | review needed |
| src/Game/Enemy/Nyoropon.cpp:316 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (100.0f * PSVECMag(&mVel)) | review needed |
| src/Game/Enemy/Nyoropon.cpp:512 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (cHalf * period) | review needed |
| src/Game/Enemy/Nyoropon.cpp:749 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (100.0f * PSVECMag(&mVel)) | review needed |
| src/Game/Enemy/Nyoropon.cpp:751 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (100.0f * PSVECMag(&mVel)) | review needed |
| src/Game/Enemy/RingBeam.cpp:383 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR | review needed |
| src/Game/Enemy/TakoSho.cpp:226 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: PSVECMag(&delta) / mMoveRange | review needed |
| src/Game/Enemy/Takobo.cpp:222 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (PSVECMag(&diff) / mMoveRange) | review needed |
| src/Game/Enemy/Terebo.cpp:149 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR | review needed |
| src/Game/Enemy/Terebo.cpp:166 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR | review needed |
| src/Game/Enemy/TeresaAtomic.cpp:69 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR | review needed |
| src/Game/Enemy/Togezo.cpp:267 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (100.0f * speed) | review needed |
| src/Game/Enemy/WanwanCreator.cpp:40 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (MR::getRailTotalLength(this) / mSpeed) | review needed |
| src/Game/Enemy/WanwanSearch.cpp:110 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (100.0f * PSVECMag(&mVelocity)) | review needed |
| src/Game/Enemy/WanwanSearch.cpp:216 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (mPosition - mLastPos).length() | review needed |
| src/Game/Enemy/WanwanSearch.cpp:223 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (100.0f * PSVECMag(&mVelocity)) | review needed |
| src/Game/LiveActor/VolumeDrawer.cpp:95 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((f32)mColor.a * mScaleRate) | review needed |
| src/Game/Map/CollisionParts.cpp:292 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (1.0f / 35.0f) * localMovement.length() | review needed |
| src/Game/Map/KCollision.cpp:709 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (cell.x & mask) | review needed |
| src/Game/Map/KCollision.cpp:710 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (cell.y & mask) | review needed |
| src/Game/Map/KCollision.cpp:711 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (cell.z & mask) | review needed |
| src/Game/Map/KCollisionPlus.cpp:11 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: pPointA->x | review needed |
| src/Game/Map/KCollisionPlus.cpp:15 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: pPointA->y | review needed |
| src/Game/Map/KCollisionPlus.cpp:19 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: pPointA->z | review needed |
| src/Game/Map/KCollisionPlus.cpp:25 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: pPointB->x | review needed |
| src/Game/Map/KCollisionPlus.cpp:31 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: pPointB->y | review needed |
| src/Game/Map/KCollisionPlus.cpp:37 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: pPointB->z | review needed |
| src/Game/Map/KCollisionPlus.cpp:41 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: pPointA->y | review needed |
| src/Game/Map/KCollisionPlus.cpp:42 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: pPointA->z | review needed |
| src/Game/Map/KCollisionPlus.cpp:50 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: rPos.x - mFile->mMin.x | review needed |
| src/Game/Map/KCollisionPlus.cpp:51 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: rPos.y - mFile->mMin.y | review needed |
| src/Game/Map/KCollisionPlus.cpp:52 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: rPos.z - mFile->mMin.z | review needed |
| src/Game/Map/SeaGull.cpp:242 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR::getRailTotalLength(this) / 500.0f | review needed |
| src/Game/Map/WaterPlant.cpp:143 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: distPlayerToPlant * 0.1f | review needed |
| src/Game/MapObj/BallRail.cpp:82 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (MR::getRailTotalLength(this) / 100.0f) | review needed |
| src/Game/MapObj/BallRail.cpp:202 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| src/Game/MapObj/BeeJumpBall.cpp:64 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (300.0f + 1000.0f / scale) | review needed |
| src/Game/MapObj/BreakableCage.cpp:315 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: CAGE_FIX | review needed |
| src/Game/MapObj/CaveInCube.cpp:55 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mScale.x | review needed |
| src/Game/MapObj/CaveInCube.cpp:56 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: -mScale.z | review needed |
| src/Game/MapObj/ChimneyEntryObj.cpp:60 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR | review needed |
| src/Game/MapObj/CrescentSeesawPlanet.cpp:38 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 1000.0f * speed | review needed |
| src/Game/MapObj/DrumTrampoline.cpp:50 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR | review needed |
| src/Game/MapObj/DrumTrampoline.cpp:51 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR | review needed |
| src/Game/MapObj/DrumTrampoline.cpp:82 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (100.0f * (1.0f / mScale.x)) | review needed |
| src/Game/MapObj/DrumTrampoline.cpp:84 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (100.0f * (1.0f / mScale.x)) | review needed |
| src/Game/MapObj/DynamicCollisionObj.cpp:93 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: extent.x | review needed |
| src/Game/MapObj/DynamicCollisionObj.cpp:94 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: extent.y | review needed |
| src/Game/MapObj/DynamicCollisionObj.cpp:95 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: extent.z | review needed |
| src/Game/MapObj/ElectricRail.cpp:434 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR::getRailTotalLength(this) / 200.0f | review needed |
| src/Game/MapObj/ElectricRailMoving.cpp:180 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR::getRailTotalLength(this) / 100.0f | review needed |
| src/Game/MapObj/FireRing.cpp:126 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (degreeToTableIndex * absAngle) | review needed |
| src/Game/MapObj/FireRing.cpp:129 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (-degreeToTableIndex * angle) | review needed |
| src/Game/MapObj/FireRing.cpp:131 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (degreeToTableIndex * angle) | review needed |
| src/Game/MapObj/FireRingBall.cpp:63 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((1.0f - dist / 1100.0f) * 1000.0f) | review needed |
| src/Game/MapObj/ItemDice.cpp:289 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (200.0f + 200.0f * (d * d)) | review needed |
| src/Game/MapObj/ItemDrill.cpp:547 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: PSVECDistance(&from, &to) / 30.0f | review needed |
| src/Game/MapObj/ItemDrill.cpp:685 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 45.511112f * angle | review needed |
| src/Game/MapObj/ItemDrill.cpp:1101 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 45.511112f * angle | review needed |
| src/Game/MapObj/JumpHole.cpp:44 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| src/Game/MapObj/LeafBoard.cpp:140 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (1000.0f * vol) | review needed |
| src/Game/MapObj/LeafBoard.cpp:143 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (1000.0f * MR::getLinerValueFromMinMax(speed, 8.0f, 16.0f, 0.1f, 1.0f)) | review needed |
| src/Game/MapObj/LeafBoard.cpp:146 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (1000.0f * MR::getLinerValueFromMinMax(speed, 13.0f, 16.0f, 0.1f, 1.0f)) | review needed |
| src/Game/MapObj/LeafBoard.cpp:149 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (1000.0f * MR::getLinerValueFromMinMax(speed, 16.0f, 27.0f, 0.1f, 1.0f)) | review needed |
| src/Game/MapObj/LeafBoard.cpp:152 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (1000.0f * MR::getLinerValueFromMinMax(speed, 22.0f, 50.0f, 0.1f, 1.0f)) | review needed |
| src/Game/MapObj/LeafBoard.cpp:164 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (100.0f * speed) | review needed |
| src/Game/MapObj/LightFruitClipArea.cpp:89 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 120.0f + 240.0f * (1.0f - ratio) | review needed |
| src/Game/MapObj/MagmaMeteor.cpp:56 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR | review needed |
| src/Game/MapObj/MapPartsFloatingForce.cpp:199 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 1000.0f * __fabsf(mVerticalSpeed) | review needed |
| src/Game/MapObj/MapPartsFloatingForce.cpp:201 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 1000.0f * __fabsf(mVerticalSpeed) | review needed |
| src/Game/MapObj/MapPartsFloatingForce.cpp:225 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 1000.0f * __fabsf(mVerticalSpeed) | review needed |
| src/Game/MapObj/MapPartsSeesaw1AxisRotator.cpp:167 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (1000.0f * __fabsf(mAngularVelocity)) | review needed |
| src/Game/MapObj/MapPartsSeesaw1AxisRotator.cpp:182 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (1000.0f * __fabsf(mAngularVelocity)) | review needed |
| src/Game/MapObj/MapPartsSeesaw2AxisRollerRotator.cpp:93 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (1000.0f*mSpeed) | review needed |
| src/Game/MapObj/MapPartsSeesaw2AxisRotator.cpp:129 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (1000.0f * intensity + (f32)(s32)150) | review needed |
| src/Game/MapObj/MarioLauncher.cpp:704 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| src/Game/MapObj/MeteorStrike.cpp:115 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: pos0.distance(nextPos) / mStepSize | review needed |
| src/Game/MapObj/MeteorStrikeLauncher.cpp:36 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR::getRailTotalLength(this) / (speed * mIntervalStep) | review needed |
| src/Game/MapObj/PlantRailInfo.cpp:12 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mRailRider->getTotalLength() / mRailSpeed | review needed |
| src/Game/MapObj/PullTargetDrawer.cpp:216 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (mMaxLength / 200.0f) | review needed |
| src/Game/MapObj/RepeatBlockGrid.cpp:104 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mCount | review needed |
| src/Game/MapObj/RepeatBlockGrid.cpp:113 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mHead | review needed |
| src/Game/MapObj/RepeatBlockGrid.cpp:116 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mCount | review needed |
| src/Game/MapObj/RepeatBlockGrid.cpp:408 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mHistory->mCount | review needed |
| src/Game/MapObj/RepeatBlockGrid.cpp:428 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mHistory->mCount | review needed |
| src/Game/MapObj/RepeatTimerSwitchingBlock.cpp:104 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR | review needed |
| src/Game/MapObj/RockCreator.cpp:34 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR::getRailTotalLength(this) / mRockSpeed | review needed |
| src/Game/MapObj/RollingWater.cpp:81 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 2607.594482421875f * phase | review needed |
| src/Game/MapObj/RollingWater.cpp:194 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 1000.0f * MR::calcGravitySpeedAbs(this) | review needed |
| src/Game/MapObj/SnowBall.cpp:420 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (3.0f * (velLen * velLen)) | review needed |
| src/Game/MapObj/SpinDriver.cpp:806 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| src/Game/MapObj/SpinDriverPathDrawer.cpp:18 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 255.0f * MR::abs(rColor.x) | review needed |
| src/Game/MapObj/SpinDriverPathDrawer.cpp:19 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 255.0f * MR::abs(rColor.y) | review needed |
| src/Game/MapObj/SpinDriverPathDrawer.cpp:20 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 255.0f * MR::abs(rColor.z) | review needed |
| src/Game/MapObj/SpinDriverPathDrawer.cpp:21 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 255.0f * MR::abs(rAlpha) | review needed |
| src/Game/MapObj/StarPiece.cpp:1135 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mColor.r | review needed |
| src/Game/MapObj/StarPiece.cpp:1136 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mColor.g | review needed |
| src/Game/MapObj/StarPiece.cpp:1137 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mColor.b | review needed |
| src/Game/MapObj/StarPiece.cpp:1138 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mColor.r | review needed |
| src/Game/MapObj/StarPiece.cpp:1139 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mColor.g | review needed |
| src/Game/MapObj/StarPiece.cpp:1140 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mColor.b | review needed |
| src/Game/MapObj/StarPieceDirector.cpp:410 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: PetariNative::Mods::Mod::CollectStarBits | review needed |
| src/Game/MapObj/StarPieceDirector.cpp:419 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: PetariNative::Mods::Mod::ShootEnemy | review needed |
| src/Game/MapObj/SuperSpinDriver.cpp:1049 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| src/Game/MapObj/WarpPod.cpp:510 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| src/Game/MapObj/WoodLogBridge.cpp:59 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (200.0f * (__fabsf(mVelocity.y) - 2.5f)) | review needed |
| src/Game/NPC/Dreamer.cpp:191 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: >(this, &Dreamer::eventFunc) | review needed |
| src/Game/NPC/KinopioPostman.cpp:142 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: > callback | review needed |
| src/Game/NPC/KinopioPostman.cpp:146 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: > callback | review needed |
| src/Game/NPC/KinopioPostman.cpp:150 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: > callback | review needed |
| src/Game/NPC/KinopioPostman.cpp:154 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: > callback | review needed |
| src/Game/NPC/KinopioPostman.cpp:158 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: > callback | review needed |
| src/Game/NPC/KinopioPostman.cpp:162 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: > callback | review needed |
| src/Game/NPC/KinopioPostman.cpp:166 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: > callback | review needed |
| src/Game/NPC/KinopioPostman.cpp:170 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: > callback | review needed |
| src/Game/NPC/KinopioPostman.cpp:174 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: > callback | review needed |
| src/Game/NPC/MeisterTU05.cpp:72 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: > callback | review needed |
| src/Game/NPC/MeisterTU05.cpp:105 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: > callback | review needed |
| src/Game/NPC/PichanRacer.cpp:124 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 100.0f * weights[1] | review needed |
| src/Game/NPC/PichanRacer.cpp:125 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 100.0f * PSVECMag(&mVelocity) | review needed |
| src/Game/NPC/SuperDreamer.cpp:278 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: > event | review needed |
| src/Game/NPC/SuperDreamer.cpp:280 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: > kill | review needed |
| src/Game/NPC/TalkMessageCtrl.cpp:187 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: branch->mIndex | review needed |
| src/Game/NPC/TalkMessageCtrl.cpp:188 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: condition | review needed |
| src/Game/Player/GhostPlayer.cpp:33 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: y | review needed |
| src/Game/Player/GhostPlayer.cpp:34 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: z | review needed |
| src/Game/Player/GhostPlayer.cpp:38 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: z | review needed |
| src/Game/Player/GhostPlayer.cpp:45 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: z | review needed |
| src/Game/Player/GhostPlayer.cpp:49 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: z | review needed |
| src/Game/Player/MarineSnow.cpp:47 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (rPosition.x + mCellSize / 2.0f) / mCellSize | review needed |
| src/Game/Player/MarineSnow.cpp:49 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (rPosition.y + mCellSize / 2.0f) / mCellSize | review needed |
| src/Game/Player/MarineSnow.cpp:51 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (rPosition.z + mCellSize / 2.0f) / mCellSize | review needed |
| src/Game/Player/Mario.cpp:1557 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: _72C < 200.0f | review needed |
| src/Game/Player/Mario.cpp:1558 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: _72C < 160.0f | review needed |
| src/Game/Player/Mario.cpp:8020 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((mMovementStates.mLowWord >> 23) & 1U) | review needed |
| src/Game/Player/Mario.cpp:13138 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mWait->_16 | review needed |
| src/Game/Player/Mario.cpp:15324 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mSinkTimer | review needed |
| src/Game/Player/MarioActor.cpp:3483 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 0.49f + partsControl.x / 10.0f | review needed |
| src/Game/Player/MarioActor.cpp:3687 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: i + mBlurHistoryIndex | review needed |
| src/Game/Player/MarioActor.cpp:6476 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MarioStatus_Skate | review needed |
| src/Game/Player/MarioActor.cpp:6843 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: pInfo->_20 >> 22 | review needed |
| src/Game/Player/MarioActor.cpp:7969 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: frameMin.x | review needed |
| src/Game/Player/MarioActor.cpp:7970 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: frameMin.y | review needed |
| src/Game/Player/MarioActor.cpp:7971 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: frameMax.x | review needed |
| src/Game/Player/MarioActor.cpp:7972 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: frameMax.y | review needed |
| src/Game/Player/MarioActor.cpp:8059 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: maximum.x | review needed |
| src/Game/Player/MarioActor.cpp:8618 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (screenX - center).length() | review needed |
| src/Game/Player/MarioActor.cpp:8619 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (screenY - center).length() | review needed |
| src/Game/Player/MarioActor.cpp:8624 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: frameOrigin.x | review needed |
| src/Game/Player/MarioActor.cpp:8625 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: frameOrigin.y | review needed |
| src/Game/Player/MarioActorSMG2_A.inc:190 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: _468 | review needed |
| src/Game/Player/MarioActorSMG2_A.inc:198 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: _468 | review needed |
| src/Game/Player/MarioActorSMG2_A.inc:208 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: _468 | review needed |
| src/Game/Player/MarioActorSMG2_A.inc:1265 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: _468 | review needed |
| src/Game/Player/MarioActorSMG2_A.inc:1506 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (t & 7) | review needed |
| src/Game/Player/MarioActorSMG2_A.inc:1849 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mCounter6C8 | review needed |
| src/Game/Player/MarioActorSMG2_B.inc:235 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: _468 | review needed |
| src/Game/Player/MarioActorSMG2_B.inc:431 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: _468 | review needed |
| src/Game/Player/MarioActorSMG2_B.inc:666 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: _468 | review needed |
| src/Game/Player/MarioActorSMG2_B.inc:707 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: _468 | review needed |
| src/Game/Player/MarioActorSMG2_B.inc:739 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: _468 | review needed |
| src/Game/Player/MarioActorSMG2_B.inc:922 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: _468 | review needed |
| src/Game/Player/MarioActorSMG2_B.inc:1258 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: _468 | review needed |
| src/Game/Player/MarioActorSMG2_B.inc:1311 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((f32)0x40U * (1.0f - v)) | review needed |
| src/Game/Player/MarioActorSMG2_B.inc:1550 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (f * (f32)mConst->getTable()->mSearchLightColor[3]) | review needed |
| src/Game/Player/MarioActorSMG2_B.inc:1580 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: tbl->mSearchLightColor[0] | review needed |
| src/Game/Player/MarioActorSMG2_B.inc:1581 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: tbl->mSearchLightColor[1] | review needed |
| src/Game/Player/MarioActorSMG2_B.inc:1582 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: tbl->mSearchLightColor[2] | review needed |
| src/Game/Player/MarioActorSMG2_B.inc:1594 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (f * (f32)(mConst->getTable())->mSearchLightAlphaLevel) | review needed |
| src/Game/Player/MarioActorSMG2_B.inc:1609 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (f * (f32)mConst->getTable()->mSearchLightBlurAlpha[i]) | review needed |
| src/Game/Player/MarioAnimator.cpp:1817 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: type-0x12 | review needed |
| src/Game/Player/MarioBump.cpp:78 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 16.0f * _14 | review needed |
| src/Game/Player/MarioSMG2_A.inc:194 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: this->mFlags14.mLowWord | review needed |
| src/Game/Player/MarioSMG2_A.inc:197 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (this->getCurrentStatus() - 8) | review needed |
| src/Game/Player/MarioSMG2_A.inc:1404 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (500.0f * MarioAccess::fn_803B57A0()) | review needed |
| src/Game/Player/MarioSMG2_A.inc:1405 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (400.0f * MarioAccess::fn_803B57A0()) | review needed |
| src/Game/Player/MarioSlider.cpp:188 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 1000.0f*_3C | review needed |
| src/Game/Player/MarioStep.cpp:23 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 6.0f * _18 | review needed |
| src/Game/Player/MarioSwim.cpp:2560 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mSwim->mActor->getConst().getTable()->mOxygenMax | review needed |
| src/Game/Player/MarioYoshiBalloon.cpp:340 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (45.511112f * magnitude) | review needed |
| src/Game/Player/YoshiLockOnManager.cpp:10 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: NO_INLINE | review needed |
| src/Game/Player/YoshiLockOnTarget.cpp:20 | astra | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: type - YoshiLockOnTargetType::Capture | review needed |
| src/Game/Ride/Creeper.cpp:355 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR::getRailCoord(this) / ::sInterval | review needed |
| src/Game/Ride/Plant.cpp:374 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((MR::getRailTotalLength(this) - ::sLeafPosOffsetStart - ::sLeafPosOffsetEnd) / (::sLeafIntervalMax - ::sLeafIntervalMin)) | review needed |
| src/Game/Ride/PlantSlider.cpp:430 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (railCoord / 30.0f) | review needed |
| src/Game/Ride/RollingRockStateRoll.cpp:358 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (1000.0f * MR::abs(PSVECMag(proj))) | review needed |
| src/Game/Ride/RollingRockStateRoll.cpp:608 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR | review needed |
| src/Game/Ride/RollingRockStateRoll.cpp:611 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR | review needed |
| src/Game/Ride/RollingRockStateRoll.cpp:612 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR | review needed |
| src/Game/Ride/RollingRockStateRoll.cpp:615 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (pitch * 0.9f) | review needed |
| src/Game/Ride/RollingRockStateRoll.cpp:652 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR | review needed |
| src/Game/Ride/RollingRockStateRoll.cpp:653 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR | review needed |
| src/Game/Ride/TubeSlider.cpp:187 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((f32)pitchBase + __fabsf(spd) * (f32)scale) | review needed |
| src/Game/Ride/TubeSlider.cpp:188 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (spd * (f32)scale) | review needed |
| src/Game/Ride/TubeSlider.cpp:590 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (__fabsf(MR::getSubPadStickX(0)) * (f32)curveScale) | review needed |
| src/Game/Ride/TubeSlider.cpp:591 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (mForwardSpeed * (f32)pitchScale) | review needed |
| src/Game/Ride/TubeSlider.cpp:592 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (mForwardSpeed * (f32)volScale) | review needed |
| src/Game/Ride/TubeSlider.cpp:594 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (0.4f * (f32)curveVol) | review needed |
| src/Game/Ride/TubeSlider.cpp:597 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (0.4f * (f32)slipPitch) | review needed |
| src/Game/Ride/TubeSlider.cpp:598 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (0.4f * (f32)slipPitch) | review needed |
| src/Game/Ride/TubeSlider.cpp:718 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((f32)pitchBase + __fabsf(spd) * (f32)scale) | review needed |
| src/Game/Ride/TubeSlider.cpp:719 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (spd * (f32)scale) | review needed |
| src/Game/Ride/TubeSliderDamageObj.cpp:119 | objects | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (basePitch + 1000.0f * (50.0f / (50.0f + (micDistance - _A4)))) | review needed |
| src/Game/Scene/IntermissionScene.cpp:29 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: JUTVideo::getManager()->getRenderMode()->efbHeight | review needed |
| src/Game/Screen/CustomTagProcessor.cpp:109 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (1.0f + mCharCount * mCharInterval) / mFadeSpeed | review needed |
| src/Game/Screen/CustomTagProcessor.cpp:116 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (1.0f + mCharCount * mCharInterval) / mFadeSpeed | review needed |
| src/Game/Screen/LayoutManager.cpp:44 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: = 0 | review needed |
| src/Game/Screen/LayoutManager.cpp:45 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: = 0 | review needed |
| src/Game/Screen/LayoutManager.cpp:46 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| src/Game/Screen/LayoutManager.cpp:60 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| src/Game/Screen/LensFlare.cpp:287 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: JUTGetVideoManager()->getEfbHeight() | review needed |
| src/Game/Screen/PostFog.cpp:120 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mAlpha * getFadeRate() | review needed |
| src/Game/Screen/PostOutline.cpp:116 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 255.0f * getFadeRate() | review needed |
| src/Game/Screen/ReplaceTagProcessor.cpp:15 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *mFunction | review needed |
| src/Game/Screen/ReplaceTagProcessor.cpp:34 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *mFunction | review needed |
| src/Game/Screen/ScreenBlurEffect.cpp:88 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: getFadeRate() * 220.0f * mIntensity | review needed |
| src/Game/Screen/StarPointerLayout.cpp:615 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: getAnimType() - AnimType_HandGuu | review needed |
| src/Game/Screen/StarPointerMovePositionHolder.cpp:211 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| src/Game/Screen/StarPointerMovePositionHolder.cpp:212 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: const | review needed |
| src/Game/Screen/SystemWipeHolder.cpp:75 | enemies | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: JUTVideo | review needed |
| src/Game/Speaker/SpkMixingBuffer.cpp:26 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (wav * volume) | MacioGalaxy has a fix: src/Game/Speaker/SpkMixingBuffer.cpp |
| src/Game/System/FileRipper.cpp:62 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (buf + 0x3B) | review needed |
| src/Game/System/GameDataFunction.cpp:381 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: GameSequenceFunction::getClearedStarPieceNum() % 10 | review needed |
| src/Game/System/GameDataFunction.cpp:434 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->mMissNum | review needed |
| src/Game/System/GameDataFunction.cpp:440 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->mMissNum | review needed |
| src/Game/System/GameSequenceNameEntryTable.cpp:22 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (mValue & 0x7FFF) | review needed |
| src/Game/System/GameSystemResetAndPowerProcess.cpp:83 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: JUTVideo::getManager()->getRenderMode()->efbHeight | review needed |
| src/Game/System/GameSystemResetAndPowerProcess.cpp:313 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: OSTicksToMilliseconds(sCheckDiskAnswered - sCheckDiskIssued) | review needed |
| src/Game/System/HeapMemoryWatcher.cpp:30 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: static_cast< u8* >(pHeap->getEndAddr()) - static_cast< u8* >(pHeap->getStartAddr()) | review needed |
| src/Game/System/HeapMemoryWatcher.cpp:57 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: static_cast< u8* >(pHeap->getEndAddr()) - static_cast< u8* >(pHeap->getStartAddr()) | review needed |
| src/Game/System/HeapMemoryWatcher.cpp:303 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: static_cast< u8* >(mFileCacheHeap->getEndAddr()) - static_cast< u8* >(mFileCacheHeap->getStartAddr()) | review needed |
| src/Game/System/HeapMemoryWatcher.cpp:304 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: cFileCacheNativeAllowance | review needed |
| src/Game/System/HeapMemoryWatcher.cpp:305 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: cFileCacheNativeAllowance | review needed |
| src/Game/System/HeapMemoryWatcher.cpp:336 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: static_cast< u8* >(allocator->getEndAddr()) - static_cast< u8* >(allocator->getStartAddr()) | review needed |
| src/Game/System/HeapMemoryWatcher.cpp:352 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: static_cast< u8* >(mFileCacheHeap->getEndAddr()) - static_cast< u8* >(mFileCacheHeap->getStartAddr()) | review needed |
| src/Game/System/HeapMemoryWatcher.cpp:355 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mFileCacheArchiveBytes | review needed |
| src/Game/System/MainLoopFramework.cpp:448 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((OS_BUS_CLOCK / 4) * frame / 59.94) | review needed |
| src/Game/System/OverwriteCharWriter.cpp:61 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mColorMapping.max.r | review needed |
| src/Game/System/OverwriteCharWriter.cpp:62 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mColorMapping.max.g | review needed |
| src/Game/System/OverwriteCharWriter.cpp:63 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: mColorMapping.max.b | review needed |
| src/Game/System/WPadPointer.cpp:133 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: JUTVideo::getManager()->getRenderMode()->efbHeight | review needed |
| src/Game/System/WorldMapResultTU07.cpp:555 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (0.8f * (f32)maxStep) | review needed |
| src/Game/System/WorldMapResultTU11.cpp:796 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: >(pMeister, &MiniMeister::startTalkAction) | review needed |
| src/Game/System/WorldMapResultTU23.cpp:19 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: > : public TalkMessageFuncBase | review needed |
| src/Game/System/WorldMapResultTU23.cpp:30 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: > WorldMapResultNewsTalkFunc | review needed |
| src/Game/Util/ActorAnimUtil.cpp:179 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: end * getRandom() | review needed |
| src/Game/Util/ActorAnimUtil.cpp:183 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: end * getRandom() | review needed |
| src/Game/Util/ActorAnimUtil.cpp:187 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: end * getRandom() | review needed |
| src/Game/Util/ActorAnimUtil.cpp:191 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: end * getRandom() | review needed |
| src/Game/Util/ActorAnimUtil.cpp:195 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: end * getRandom() | review needed |
| src/Game/Util/ActorAnimUtil.cpp:199 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: end * getRandom() | review needed |
| src/Game/Util/ActorSoundUtil.cpp:13 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 1000.0f * ((distance - calcDistanceToPlayer(pActor)) / distance) | review needed |
| src/Game/Util/ActorSoundUtil.cpp:18 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 1000.0f * ((distance - PSVECDistance(&pActor->mPosition, &getMicPos())) / distance) | review needed |
| src/Game/Util/BossJugem.cpp:893 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 100.0f * PSVECMag(&delta) | review needed |
| src/Game/Util/BossJugemTU02.cpp:60 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR | review needed |
| src/Game/Util/BossJugemTU02.cpp:62 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: MR | review needed |
| src/Game/Util/MathUtil.cpp:32 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (-x - 0.98f) * 50.0f * 255.0f | review needed |
| src/Game/Util/MathUtil.cpp:37 | core | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (x - 0.98f) * 50.0f * 255.0f | review needed |
| src/JSystem/J2DGraph/J2DPicture.cpp:70 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (field_0x109 & 1 << i) | review needed |
| src/JSystem/J3DGraphAnimator/J3DAnimation.cpp:433 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((f32)rot1 + rate * (f32)delta) | review needed |
| src/JSystem/J3DGraphAnimator/J3DAnimation.cpp:466 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((f32)rot1 + rate * (f32)delta) | review needed |
| src/JSystem/J3DGraphAnimator/J3DAnimation.cpp:499 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((f32)rot1 + rate * (f32)delta) | review needed |
| src/JSystem/J3DGraphBase/J3DGD.cpp:371 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (mtx_id - GX_ITM_0) | review needed |
| src/JSystem/J3DGraphBase/J3DGD.cpp:376 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (mtx_id - GX_ITM_S0) | review needed |
| src/JSystem/J3DGraphBase/J3DGD.cpp:381 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (mtx_id - GX_ITM_T0) | review needed |
| src/JSystem/J3DGraphBase/J3DGD.cpp:388 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (offset[0][0] * 0x400) | review needed |
| src/JSystem/J3DGraphBase/J3DGD.cpp:389 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (offset[1][0] * 0x400) | review needed |
| src/JSystem/J3DGraphBase/J3DGD.cpp:391 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (offset[0][1] * 0x400) | review needed |
| src/JSystem/J3DGraphBase/J3DGD.cpp:392 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (offset[1][1] * 0x400) | review needed |
| src/JSystem/J3DGraphBase/J3DGD.cpp:394 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (offset[0][2] * 0x400) | review needed |
| src/JSystem/J3DGraphBase/J3DGD.cpp:395 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (offset[1][2] * 0x400) | review needed |
| src/JSystem/J3DGraphBase/J3DGD.cpp:512 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (8388638.0f * B_mant) | review needed |
| src/JSystem/J3DGraphBase/J3DGD.cpp:524 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (arg0) | review needed |
| src/JSystem/J3DGraphBase/J3DGD.cpp:526 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (arg1) | review needed |
| src/JSystem/J3DGraphBase/J3DMatBlock.cpp:13 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: color.b | review needed |
| src/JSystem/J3DGraphBase/J3DShapeMtx.cpp:109 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (tex_mtx_info_2->mInfo & 0x80) | review needed |
| src/JSystem/J3DGraphBase/J3DShapeMtx.cpp:124 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (tex_mtx_info_2->mInfo & 0x80) | review needed |
| src/JSystem/J3DGraphBase/J3DShapeMtx.cpp:141 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (tex_mtx_info_2->mInfo & 0x80) | review needed |
| src/JSystem/J3DGraphBase/J3DShapeMtx.cpp:196 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (tex_mtx_info_2->mInfo & 0x80) | review needed |
| src/JSystem/J3DGraphBase/J3DShapeMtx.cpp:213 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (tex_mtx_info_2->mInfo & 0x80) | review needed |
| src/JSystem/J3DGraphBase/J3DShapeMtx.cpp:231 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (tex_mtx_info_2->mInfo & 0x80) | review needed |
| src/JSystem/JAudio2/JAISeq.cpp:267 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: id | review needed |
| src/JSystem/JAudio2/JASAramStream.cpp:248 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: header->loop_end | review needed |
| src/JSystem/JAudio2/JASAramStream.cpp:340 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: bhead->_4 | review needed |
| src/JSystem/JAudio2/JASSeqParser.cpp:879 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: readType & 3 | review needed |
| src/JSystem/JAudio2/JAUSeqDataBlockMgr.cpp:16 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: param_0 | review needed |
| src/JSystem/JAudio2/JAUStdSoundInfo.cpp:129 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: data->mStreamFileNameOffset | review needed |
| src/JSystem/JKernel/JKRAramArchive.cpp:207 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ALIGN_PREV(bufferSize, 0x20) | review needed |
| src/JSystem/JKernel/JKRDecomp.cpp:110 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ptr[offset + 3] | review needed |
| src/JSystem/JKernel/JKRHeap.cpp:78 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: arenaStart | review needed |
| src/JSystem/JKernel/JKRUnitHeap.cpp:51 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: bat + ALIGN_NEXT(unitCount / 8, 4) | review needed |
| src/JSystem/JMath/JMATrigonometric.cpp:40 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: 0.5f + Len * y / x | review needed |
| src/JSystem/JParticle/JPABaseShape.cpp:184 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (param_1->mTime * maxFrm) | review needed |
| src/JSystem/JParticle/JPABaseShape.cpp:359 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (maxFrm * param_1->mTime) | review needed |
| src/JSystem/JParticle/JPAKeyBlock.cpp:9 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: field_0x4[(mDataStart[9] - 1) * 4] | review needed |
| src/JSystem/JParticle/JPAParticle.cpp:298 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((mLifeTime - 1) * csp->getTiming()) | review needed |
| src/JSystem/JUtility/JUTConsole.cpp:96 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (mFontSizeX * field_0x20) + 4.0f | review needed |
| src/JSystem/JUtility/JUTConsole.cpp:97 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: font_yOffset * mHeight | review needed |
| src/JSystem/JUtility/JUTConsole.cpp:113 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: font_yOffset * mHeight | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_fmod.c:27 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: sx | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_fmod.c:102 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: sx | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_fmod.c:114 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: sx | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_fmod.c:124 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: sx | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_fmod.c:137 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: sx | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_fmod.c:154 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: hx | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_log10.c:41 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: k | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_rem_pio2.c:63 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (t * 6.36619772367581382433e-01 + 5.00000000000000000000e-01) | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/k_rem_pio2.c:50 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (5.96046447753906250000e-08 * z) | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/k_rem_pio2.c:51 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (z - 1.67772160000000000000e+07 * fw) | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/k_rem_pio2.c:147 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (5.96046447753906250000e-08 * z) | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/k_rem_pio2.c:148 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (z - 1.67772160000000000000e+07 * fw) | review needed |
| src/MSL_C/ansi_fp.c:617 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (e - i) | review needed |
| src/MSL_C/mem_funcs.c:121 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (srcCharPtr + 1) | review needed |
| src/MSL_C/mem_funcs.c:174 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (srcCharPtr) | review needed |
| src/MSL_C/scanf.c:33 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *ReadProc | review needed |
| src/MSL_C/scanf.c:34 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *ReadProc | review needed |
| src/MSL_C/scanf.c:36 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *ReadProc | review needed |
| src/MSL_C/scanf.c:272 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *ReadProc | review needed |
| src/MSL_C/string.c:15 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: fromb | review needed |
| src/MSL_C/strtold.c:28 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *ReadProc | review needed |
| src/MSL_C/strtoul.c:10 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *ReadProc | review needed |
| src/MSL_C/strtoul.c:147 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *ReadProc | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:972 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: INSTR_STW(4, 0, 3) | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:974 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: INSTR_LWZ(4, 0, 3) | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:1097 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: asm_access | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:1099 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: access_func | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:1115 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: access_func | review needed |
| src/MetroTRK/gamedev/cust_connection/cc/exi2/GCN/EXI2_GDEV_GCN/main.c:120 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: DBRead(buff, poll) | review needed |
| src/RVL_SDK/arc/arc.c:178 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: parentDir(FSTEntries, dirLookAt) | review needed |
| src/RVL_SDK/axfx/AXFXReverbHiExp.c:35 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (reverb->preDelayTimeMax * (f32)32000) | review needed |
| src/RVL_SDK/axfx/AXFXReverbHiExp.c:193 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (output[0] * reverb->outGain) | review needed |
| src/RVL_SDK/axfx/AXFXReverbHiExp.c:194 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (output[1] * reverb->outGain) | review needed |
| src/RVL_SDK/axfx/AXFXReverbHiExp.c:195 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (output[2] * reverb->outGain) | review needed |
| src/RVL_SDK/axfx/AXFXReverbHiExp.c:198 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (output[0] * reverb->sendGain) | review needed |
| src/RVL_SDK/axfx/AXFXReverbHiExp.c:199 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (output[1] * reverb->sendGain) | review needed |
| src/RVL_SDK/axfx/AXFXReverbHiExp.c:200 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (output[2] * reverb->sendGain) | review needed |
| src/RVL_SDK/axfx/AXFXReverbHiExp.c:373 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (reverb->preDelayTime * (f32)32000) | review needed |
| src/RVL_SDK/bte/bt_types.h:58 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: >>  0 | review needed |
| src/RVL_SDK/bte/bt_types.h:59 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: >>  8 | review needed |
| src/RVL_SDK/bte/bt_types.h:60 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: >> 16 | review needed |
| src/RVL_SDK/bte/bt_types.h:61 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: >> 24 | review needed |
| src/RVL_SDK/bte/bt_types.h:139 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: = 0												\ | review needed |
| src/RVL_SDK/bte/bt_types.h:204 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: >> 24 | review needed |
| src/RVL_SDK/bte/bt_types.h:205 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: >> 16 | review needed |
| src/RVL_SDK/bte/bt_types.h:206 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: >>  8 | review needed |
| src/RVL_SDK/bte/bt_types.h:207 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: >>  0 | review needed |
| src/RVL_SDK/bte/bt_types.h:242 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: = 0												\ | review needed |
| src/RVL_SDK/bte/bt_types.h:270 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: >> 24 | review needed |
| src/RVL_SDK/bte/bt_types.h:271 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: >> 16 | review needed |
| src/RVL_SDK/bte/bt_types.h:272 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: >>  8 | review needed |
| src/RVL_SDK/bte/bt_types.h:273 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: >>  0 | review needed |
| src/RVL_SDK/bte/hcicmds.c:2104 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ARRAY_LENGTH(channels) | review needed |
| src/RVL_SDK/bte/hidd_api.c:325 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ARRAY_LENGTH(hd_cb.pm_params) | review needed |
| src/RVL_SDK/bte/hidd_pm.c:52 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ARRAY_LENGTH(hd_cb.pm_params) | review needed |
| src/RVL_SDK/bte/sdp_utils.c:137 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: p_attr->len | review needed |
| src/RVL_SDK/bte/sdp_utils.c:169 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: p_attr->len | review needed |
| src/RVL_SDK/bte/sdp_utils.c:200 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: strlen(p_error_text) | review needed |
| src/RVL_SDK/dsp/dsp_task.c:72 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (pTask->iram_addr & 0xFFFF) | review needed |
| src/RVL_SDK/dsp/dsp_task.c:84 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (0xFFFF & pTask->dsp_vector) | review needed |
| src/RVL_SDK/dvd/dvd.c:793 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (0x420 >> 2) | review needed |
| src/RVL_SDK/dvd/dvd.c:794 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (0x420 >> 2) | review needed |
| src/RVL_SDK/dvd/dvd.c:1530 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: TRUE | review needed |
| src/RVL_SDK/dvd/dvd.c:1585 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (DVDLowGetImmBufferReg() << 2) | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:61 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: high | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:502 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: numTmdBytes | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:507 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: numCertBytes | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:539 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (addr) | review needed |
| src/RVL_SDK/dvd/dvd_broadway.c:615 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (dataWordOffset) | review needed |
| src/RVL_SDK/dvd/dvderror.c:201 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: OSTicksToSeconds(OSGetTime()) | review needed |
| src/RVL_SDK/dvd/dvdfs.c:74 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: dirLookAt | review needed |
| src/RVL_SDK/dvd/dvdfs.c:89 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: parentDir(dirLookAt) | review needed |
| src/RVL_SDK/dvd/dvdfs.c:97 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: dirLookAt | review needed |
| src/RVL_SDK/dvd/dvdfs.c:281 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (fileInfo->startAddr + (offset >> 2)) | review needed |
| src/RVL_SDK/dvd/dvdfs.c:309 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (fileInfo->startAddr + (offset >> 2)) | review needed |
| src/RVL_SDK/euart/euart.c:134 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((*sendData & 0xFF) << 24) | review needed |
| src/RVL_SDK/exi/EXIBios.c:309 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (OSTicksToMilliseconds(OSGetTime()) / 100) | review needed |
| src/RVL_SDK/exi/EXICommon.c:11 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((*((u8*)reg) & 0xFF) << 24) | review needed |
| src/RVL_SDK/exi/EXICommon.c:14 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (((*((u16*)reg) & 0xFF) << 24) \| ((*((u16*)reg) & 0xFF00) << 8)) | review needed |
| src/RVL_SDK/fs/fs.c:13 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (sz) | review needed |
| src/RVL_SDK/fs/fs.c:56 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: hi | review needed |
| src/RVL_SDK/fs/fs.c:72 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: hi | review needed |
| src/RVL_SDK/fs/fs.c:79 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: __fsCtxt | review needed |
| src/RVL_SDK/fs/fs.c:624 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: blkPtr | review needed |
| src/RVL_SDK/gd/GDGeometry.c:244 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (func - GX_TG_BUMP0) | review needed |
| src/RVL_SDK/gd/GDLight.c:6 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: color.a | review needed |
| src/RVL_SDK/gd/GDLight.c:10 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: color.a | review needed |
| src/RVL_SDK/gd/GDPixel.c:10 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (type & 0x07) | review needed |
| src/RVL_SDK/gd/GDPixel.c:11 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((type >> 3) & 0x01) | review needed |
| src/RVL_SDK/gd/GDPixel.c:48 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (B_mant * 8388638) | review needed |
| src/RVL_SDK/gx/GXBump.c:33 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (mtx_id - GX_ITM_0) | review needed |
| src/RVL_SDK/gx/GXBump.c:38 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (mtx_id - GX_ITM_S0) | review needed |
| src/RVL_SDK/gx/GXBump.c:43 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (mtx_id - GX_ITM_T0) | review needed |
| src/RVL_SDK/gx/GXBump.c:50 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (offset[0][0] * (f32)(1 << 10)) | review needed |
| src/RVL_SDK/gx/GXBump.c:51 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (offset[1][0] * (f32)(1 << 10)) | review needed |
| src/RVL_SDK/gx/GXBump.c:61 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (offset[0][1] * (f32)(1 << 10)) | review needed |
| src/RVL_SDK/gx/GXBump.c:62 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (offset[1][1] * (f32)(1 << 10)) | review needed |
| src/RVL_SDK/gx/GXBump.c:70 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (offset[0][2] * (f32)(1 << 10)) | review needed |
| src/RVL_SDK/gx/GXBump.c:71 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (offset[1][2] * (f32)(1 << 10)) | review needed |
| src/RVL_SDK/gx/GXFifo.c:11 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: a | review needed |
| src/RVL_SDK/gx/GXFifo.c:30 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: GX_CP_REG_READ_U16(reg##H) | review needed |
| src/RVL_SDK/gx/GXFifo.c:31 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: GX_CP_REG_READ_U16(reg##L) | review needed |
| src/RVL_SDK/gx/GXFifo.c:38 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: GX_CP_REG_READ_U16(reg##H) | review needed |
| src/RVL_SDK/gx/GXFifo.c:39 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: GX_CP_REG_READ_U16(reg##L) | review needed |
| src/RVL_SDK/gx/GXFrameBuf.c:47 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (fmt & 0xf) | review needed |
| src/RVL_SDK/gx/GXFrameBuf.c:123 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (256.0f / yScale) | review needed |
| src/RVL_SDK/gx/GXFrameBuf.c:133 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (256.0f / yScale) | review needed |
| src/RVL_SDK/gx/GXFrameBuf.c:139 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (256.0f / yScale) | review needed |
| src/RVL_SDK/gx/GXFrameBuf.c:148 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (256.0f / yScale) | review needed |
| src/RVL_SDK/gx/GXFrameBuf.c:161 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (256.0f / vscale) | review needed |
| src/RVL_SDK/gx/GXGeometry.c:158 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (gx->vNum * gx->vLim) | review needed |
| src/RVL_SDK/gx/GXInit.c:68 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: OSUncachedToPhysical((void*)0xCC008000) | review needed |
| src/RVL_SDK/gx/GXInit.c:170 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: i | review needed |
| src/RVL_SDK/gx/GXLight.c:318 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (chan & 0x0003) | review needed |
| src/RVL_SDK/gx/GXMisc.c:156 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (func << 8 \| threshold) | review needed |
| src/RVL_SDK/gx/GXPixel.c:13 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (type & 0x07) | review needed |
| src/RVL_SDK/gx/GXPixel.c:14 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((type >> 3) & 0x01) | review needed |
| src/RVL_SDK/gx/GXPixel.c:48 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (B_mant * 8388638) | review needed |
| src/RVL_SDK/gx/GXPixel.c:113 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (type & 0x01) | review needed |
| src/RVL_SDK/gx/GXTev.c:513 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (map & ~GX_TEX_DISABLE) | review needed |
| src/RVL_SDK/gx/GXTexture.c:13 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: d | review needed |
| src/RVL_SDK/gx/GXTexture.c:133 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((width + (1 << tileShiftX) - 1) >> tileShiftX) | review needed |
| src/RVL_SDK/gx/GXTexture.c:134 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((height + (1 << tileShiftY) - 1) >> tileShiftY) | review needed |
| src/RVL_SDK/gx/GXTexture.c:145 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((width + (1 << tileShiftX) - 1) >> tileShiftX) | review needed |
| src/RVL_SDK/gx/GXTexture.c:146 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((height + (1 << tileShiftY) - 1) >> tileShiftY) | review needed |
| src/RVL_SDK/gx/GXTexture.c:163 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((wd + (1 << texRowShift) - 1) >> texRowShift) | review needed |
| src/RVL_SDK/gx/GXTexture.c:164 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((ht + (1 << texColShift) - 1) >> texColShift) | review needed |
| src/RVL_SDK/gx/GXTexture.c:195 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (31 - NumLeadingZeros(width)) | review needed |
| src/RVL_SDK/gx/GXTexture.c:197 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (31 - NumLeadingZeros(height)) | review needed |
| src/RVL_SDK/gx/GXTexture.c:257 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((width + (1 << rowT) - 1) >> rowT) | review needed |
| src/RVL_SDK/gx/GXTexture.c:258 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((height + (1 << colT) - 1) >> colT) | review needed |
| src/RVL_SDK/gx/GXTransform.c:332 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (x_off + (u32)342.0f) | review needed |
| src/RVL_SDK/gx/GXTransform.c:333 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (y_off + (u32)342.0f) | review needed |
| src/RVL_SDK/ipc/ipcclt.c:32 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (sz) | review needed |
| src/RVL_SDK/ipc/ipcclt.c:133 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: v->vector[i].base | review needed |
| src/RVL_SDK/ipc/memory.c:28 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ptr | review needed |
| src/RVL_SDK/ipc/memory.c:192 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ptr | review needed |
| src/RVL_SDK/kpad/KMPLS.c:283 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (1500.0f * window) | review needed |
| src/RVL_SDK/kpad/KMPLS.c:675 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (device - 5) | review needed |
| src/RVL_SDK/kpad/KPAD.c:85 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (delay_sec * 200.0f + 0.5f) | review needed |
| src/RVL_SDK/kpad/KPAD.c:86 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (pulse_sec * 200.0f + 0.5f) | review needed |
| src/RVL_SDK/kpad/KPAD.c:416 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: -uwp->u.core.accX | review needed |
| src/RVL_SDK/kpad/KPAD.c:417 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: -uwp->u.core.accZ | review needed |
| src/RVL_SDK/kpad/KPAD.c:440 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: -uwp->u.fs.fsAccX | review needed |
| src/RVL_SDK/kpad/KPAD.c:441 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: -uwp->u.fs.fsAccZ | review needed |
| src/RVL_SDK/kpad/KPAD.c:444 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: -uwp->u.fs.fsAccX | review needed |
| src/RVL_SDK/kpad/KPAD.c:445 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: -uwp->u.fs.fsAccZ | review needed |
| src/RVL_SDK/kpad/KPAD.c:916 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: wobj_p->x | review needed |
| src/RVL_SDK/kpad/KPAD.c:917 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: wobj_p->y | review needed |
| src/RVL_SDK/kpad/KPAD.c:1934 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (kp->deviceType - WPAD_DEV_MPLS) | review needed |
| src/RVL_SDK/kpad/KZMplsTestSub.c:10 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (value - 0.5f) | review needed |
| src/RVL_SDK/kpad/KZMplsTestSub.c:12 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (0.5f + value) | review needed |
| src/RVL_SDK/nand/NANDCore.c:38 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (strlen(oldpath)) | review needed |
| src/RVL_SDK/nand/NANDCore.c:88 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (strlen(path) - 1) | review needed |
| src/RVL_SDK/nand/NANDCore.c:280 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: strlen(absPath) | review needed |
| src/RVL_SDK/nwc24/NWC24FileAPI.c:89 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (NAND_RESULT_ALLOC_FAILED - NAND_RESULT_BUSY) | review needed |
| src/RVL_SDK/nwc24/NWC24FileAPI.c:179 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (NAND_RESULT_ALLOC_FAILED - NAND_RESULT_BUSY) | review needed |
| src/RVL_SDK/nwc24/NWC24FileAPI.c:225 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (NAND_RESULT_ALLOC_FAILED - NAND_RESULT_BUSY) | review needed |
| src/RVL_SDK/nwc24/NWC24FileAPI.c:267 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (NAND_RESULT_ALLOC_FAILED - NAND_RESULT_BUSY) | review needed |
| src/RVL_SDK/nwc24/NWC24FileAPI.c:313 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (NAND_RESULT_ALLOC_FAILED - NAND_RESULT_BUSY) | review needed |
| src/RVL_SDK/nwc24/NWC24FileAPI.c:461 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (NAND_RESULT_ALLOC_FAILED - NAND_RESULT_BUSY) | review needed |
| src/RVL_SDK/nwc24/NWC24FileAPI.c:512 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (NAND_RESULT_ALLOC_FAILED - NAND_RESULT_BUSY) | review needed |
| src/RVL_SDK/nwc24/NWC24FileAPI.c:579 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (NAND_RESULT_ALLOC_FAILED - NAND_RESULT_BUSY) | review needed |
| src/RVL_SDK/nwc24/NWC24MsgCommit.c:779 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (myId >> 0) | review needed |
| src/RVL_SDK/nwc24/NWC24Time.c:185 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (OSTicksToSeconds(time) - bias) | review needed |
| src/RVL_SDK/os/OS.c:502 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *(u8*)OSPhysicalToCached(0x30E8) | review needed |
| src/RVL_SDK/os/OS.c:504 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *(u8*)OSPhysicalToCached(0x30E9) | review needed |
| src/RVL_SDK/os/OSArena.c:9 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (n) | review needed |
| src/RVL_SDK/os/OSExec.c:581 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (__OSCurrentTGCOffset >> 2) | review needed |
| src/RVL_SDK/os/OSExec.c:583 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((__OSCurrentTGCOffset + apploaderOffsetInTGC) >> 2) | review needed |
| src/RVL_SDK/os/OSExec.c:596 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: OSRoundUp32B(header->size) | review needed |
| src/RVL_SDK/os/OSExec.c:635 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (offset >> __DVDLayoutFormat) | review needed |
| src/RVL_SDK/os/OSExec.c:742 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (GetApploaderPosition() + ((sizeof(ImgHeader) + header->size) >> 2)) | review needed |
| src/RVL_SDK/os/OSFatal.c:121 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (__VIRegs[0x36]) | review needed |
| src/RVL_SDK/os/OSFatal.c:210 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (__EXIRegs[3]) | review needed |
| src/RVL_SDK/os/OSFatal.c:226 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: Halt | review needed |
| src/RVL_SDK/os/OSFatal.c:242 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (640 * 480 * 2) | review needed |
| src/RVL_SDK/os/OSFont.c:232 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (i + j + 0x2BE) | review needed |
| src/RVL_SDK/os/OSLaunch.c:18 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (((u64)t)&0xffffffff) | review needed |
| src/RVL_SDK/os/OSLaunch.c:19 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((((u64)t)>>32)&0xffffffff) | review needed |
| src/RVL_SDK/os/OSLaunch.c:79 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: tmd->head.reserved | review needed |
| src/RVL_SDK/os/OSMemory.c:15 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (n) | review needed |
| src/RVL_SDK/os/OSMemory.c:16 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (n) | review needed |
| src/RVL_SDK/os/OSMemory.c:46 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (__MEMRegs[0x12] & 0x3FF) | review needed |
| src/RVL_SDK/os/OSMemory.c:67 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: addr | review needed |
| src/RVL_SDK/os/OSMemory.c:506 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ConfigMEM1_24MB | review needed |
| src/RVL_SDK/os/OSMemory.c:514 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ConfigMEM2_52MB | review needed |
| src/RVL_SDK/os/OSMemory.c:517 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ConfigMEM2_56MB | review needed |
| src/RVL_SDK/os/OSMemory.c:520 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ConfigMEM2_64MB | review needed |
| src/RVL_SDK/os/OSMemory.c:525 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ConfigMEM2_112MB | review needed |
| src/RVL_SDK/os/OSMemory.c:528 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ConfigMEM2_128MB | review needed |
| src/RVL_SDK/os/OSMemory.c:536 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ConfigMEM1_24MB | review needed |
| src/RVL_SDK/os/OSRtc.c:167 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (offset << 6) | review needed |
| src/RVL_SDK/os/OSSync.c:27 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: __OSSystemCallVectorStart | review needed |
| src/RVL_SDK/os/OSThread.c:9 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (n) | review needed |
| src/RVL_SDK/os/OSThread.c:10 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (n) | review needed |
| src/RVL_SDK/os/OSThread.c:11 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (n) | review needed |
| src/RVL_SDK/os/OSThread.c:408 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: OSExitThread | review needed |
| src/RVL_SDK/os/OSThread.c:651 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: val | review needed |
| src/RVL_SDK/os/OSTime.c:72 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (OSTicksToMicroseconds(ticksAfter) % 1000) | review needed |
| src/RVL_SDK/os/OSTime.c:73 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (OSTicksToMilliseconds(ticksAfter) % 1000) | review needed |
| src/RVL_SDK/os/OSTime.c:77 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (OSTicksToSeconds(ticks) / 86400 + 0xB2575) | review needed |
| src/RVL_SDK/os/OSTime.c:78 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (OSTicksToSeconds(ticks) % 86400) | review needed |
| src/RVL_SDK/rso/RSOLink.c:224 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: addr | review needed |
| src/RVL_SDK/sc/scapi_prdinfo.c:60 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((seed >> 31) \| (seed << 1)) | review needed |
| src/RVL_SDK/sc/scapi_prdinfo.c:67 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((seed >> 31) \| (seed << 1)) | review needed |
| src/RVL_SDK/sc/scsystem.c:235 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: result | review needed |
| src/RVL_SDK/sc/scsystem.c:395 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (((*p) & 0x1F) + 1) | review needed |
| src/RVL_SDK/sc/scsystem.c:419 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((*bufp & 0x1F) + 1) | review needed |
| src/RVL_SDK/sc/scsystem.c:437 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (*(itemp->data) + 1) | review needed |
| src/RVL_SDK/sc/scsystem.c:442 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((((*(itemp->data)) << 8) \| (*(itemp->data + 1))) + 1) | review needed |
| src/RVL_SDK/si/SIBios.c:6 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (n) | review needed |
| src/RVL_SDK/si/SIBios.c:7 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (n) | review needed |
| src/RVL_SDK/thp/THPAudio.c:133 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((*(info->encodeData) & 0xF) << 28) | review needed |
| src/RVL_SDK/thp/THPAudio.c:137 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((*(info->encodeData) & 0xF0) << 24) | review needed |
| src/RVL_SDK/thp/THPDec.c:27 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (b) | review needed |
| src/RVL_SDK/thp/THPDec.c:453 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (j >> (5-k-1)) | review needed |
| src/RVL_SDK/thp/THPDec.c:541 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (cl_num * 16) | review needed |
| src/RVL_SDK/thp/THPDec.c:1084 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (cb & (~mask)) | review needed |
| src/RVL_SDK/thp/THPDec.c:1452 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (cl_num * 16) | review needed |
| src/RVL_SDK/thp/THPDec.c:1519 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (cl_num * 16) | review needed |
| src/RVL_SDK/thp/THPDec.c:1830 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (cb & (~mask)) | review needed |
| src/RVL_SDK/usb/usb.c:10 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (sz) | review needed |
| src/RVL_SDK/usb/usb.c:106 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: hi | review needed |
| src/RVL_SDK/usb/usb.c:712 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: buflen | review needed |
| src/RVL_SDK/vf/d_common.c:196 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *makeFS | review needed |
| src/RVL_SDK/vf/d_vf_sys.c:57 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *l_check_exist_file[4] | review needed |
| src/RVL_SDK/vf/d_vf_sys.c:616 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *check_proc | review needed |
| src/RVL_SDK/vf/nand_drv.c:11 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *create | review needed |
| src/RVL_SDK/vf/nand_drv.c:12 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *open | review needed |
| src/RVL_SDK/vf/nand_drv.c:13 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *createDir | review needed |
| src/RVL_SDK/vf/nand_drv.c:14 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *delete | review needed |
| src/RVL_SDK/vf/nand_drv.c:21 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: * | review needed |
| src/RVL_SDK/vf/nand_drv.c:23 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: * | review needed |
| src/RVL_SDK/vf/nand_drv.c:24 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: * | review needed |
| src/RVL_SDK/vf/nand_drv.c:202 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *tmpOpen | review needed |
| src/RVL_SDK/vf/nand_drv.c:207 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: * | review needed |
| src/RVL_SDK/vf/nand_drv.c:349 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *tmpOpen | review needed |
| src/RVL_SDK/vf/pdm_bpb.c:46 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: p_bpb->fat_type | review needed |
| src/RVL_SDK/vf/pdm_bpb.c:115 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: p_bpb->fat_type | review needed |
| src/RVL_SDK/vf/pdm_mbr.c:7 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: *VFipdm_disk_physical_read_5arg | review needed |
| src/RVL_SDK/vf/pf_cache.c:134 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (p_buf - p_page->buffer) | review needed |
| src/RVL_SDK/vf/pf_entry.c:22 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (val) | review needed |
| src/RVL_SDK/vf/pf_entry.c:167 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: GET_U16_LE(&buf[0x14]) | review needed |
| src/RVL_SDK/vf/pf_entry_iterator.c:691 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((u16*)p_iter->buf)[10] | review needed |
| src/RVL_SDK/vf/pf_entry_iterator.c:692 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((u16*)p_iter->buf)[13] | review needed |
| src/RVL_SDK/vf/pf_fat12.c:69 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (raw_entry >> 4) | review needed |
| src/RVL_SDK/vf/pf_fat12.c:71 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (raw_entry & 0xFFF) | review needed |
| src/RVL_SDK/vf/pf_fat32.c:45 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: * | review needed |
| src/RVL_SDK/vf/pf_fat32.c:108 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: * | review needed |
| src/RVL_SDK/vf/pf_fat32.c:216 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: * | review needed |
| src/RVL_SDK/vf/pf_volume.c:496 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: p_drv->cache->buffers | review needed |
| src/RVL_SDK/vf/pf_w_clib.c:32 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (*s1 - *s2) | review needed |
| src/RVL_SDK/vi/i2c.c:26 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (reg \| (value << 14)) | review needed |
| src/RVL_SDK/vi/i2c.c:36 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (reg \| (value << 15)) | review needed |
| src/RVL_SDK/vi/i2c.c:43 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((reg >> 15) & 1) | review needed |
| src/RVL_SDK/vi/i2c.c:48 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (reg & ~(1 << 15)) | review needed |
| src/RVL_SDK/vi/i2c.c:49 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (reg \| (1 << 14) \| (dir << 15)) | review needed |
| src/RVL_SDK/vi/vi.c:44 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (((u32)(fb)) & 0x3FFFFFFF) | review needed |
| src/RVL_SDK/vi/vi.c:212 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ((__VIRegs[0x16]) & 0x7FF) | review needed |
| src/RVL_SDK/vi/vi.c:216 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (__VIRegs[0x17] & 0x7FF) | review needed |
| src/RVL_SDK/vi/vi.c:217 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (__VIRegs[0x16] & 0x7FF) | review needed |
| src/RVL_SDK/vi/vi.c:386 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (1 << (31 - regIndex)) | review needed |
| src/RVL_SDK/vi/vi.c:544 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: wordPerLine | review needed |
| src/RVL_SDK/vi/vi.c:926 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (bit >> 32) | review needed |
| src/RVL_SDK/vi/vi.c:927 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: (bit & 0xFFFFFFFF) | review needed |
| src/RVL_SDK/vi/vi.c:1084 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: rm->viTVmode | review needed |
| src/RVL_SDK/vi/vi.c:1091 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: rm->viTVmode | review needed |
| src/RVL_SDK/vi/vi.c:1405 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: THD_TIME_TO_DIMMING | review needed |
| src/RVL_SDK/vi/vi.c:1435 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: THD_TIME_TO_DVD_STOP | review needed |
| src/RVL_SDK/wpad/WPAD.c:446 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: OSTicksToSeconds(__OSGetSystemTime() - p_wpd->lastUpdateTime) | review needed |
| src/RVL_SDK/wpad/WPAD.c:806 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: WPADiManageHandler | review needed |
| src/RVL_SDK/wpad/WPAD.c:2567 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: OSGetTime() | review needed |
| src/RVL_SDK/wpad/WPAD.c:2583 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: OSGetTime() | review needed |
| src/RVL_SDK/wpad/WPAD.c:2591 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: OSGetTime() | review needed |
| src/RVL_SDK/wpad/WPAD.c:2607 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: OSGetTime() | review needed |
| src/RVL_SDK/wud/WUD.c:918 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: SyncHandler | review needed |
| src/RVL_SDK/wud/WUD.c:1055 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: DeleteAllHandler | review needed |
| src/RVL_SDK/wud/WUD.c:1135 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: EnableStackHandler | review needed |
| src/RVL_SDK/wud/WUD.c:1349 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: InitHandler | review needed |
| src/RVL_SDK/wud/WUD.c:1393 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: ShutdownHandler | review needed |
| src/nw4r/lyt/lyt_common.cpp:12 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: c | review needed |
| src/nw4r/ut/ut_TagProcessorBase.cpp:82 | m1 | 32-bit cast operand type unresolved; may be an ordinary numeric conversion; operand: dx / tabPixel | review needed |

### endian-raw-access — review (334)

| File:line | Lane | Evidence | Reference |
| --- | --- | --- | --- |
| include/Game/MapObj/RepeatBlock.hpp:23 | objects | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &rOther | review needed |
| include/Game/MapObj/RepeatBlock.hpp:28 | objects | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &rOther | review needed |
| include/Game/MapObj/RepeatBlock.hpp:89 | objects | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &(a) | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DGD.hpp:50 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: data | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DMatBlock.hpp:8 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &j3dDefaultIndTexOrderNull | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DMatBlock.hpp:17 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &info | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DMatBlock.hpp:31 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &j3dDefaultIndTexCoordScaleInfo | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DMatBlock.hpp:35 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &info | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DStruct.hpp:79 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &other.mRotation | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DStruct.hpp:152 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &other | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DTevs.hpp:257 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &j3dDefaultTevOrderInfoNull | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DTevs.hpp:261 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &info | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DTevs.hpp:459 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &j3dDefaultBlendInfo | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DTevs.hpp:463 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &info | review needed |
| libs/JSystem/include/JSystem/J3DGraphBase/J3DTransform.hpp:21 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &b.mRotation | review needed |
| libs/JSystem/include/JSystem/JAudio2/JAISound.hpp:152 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &mState | review needed |
| libs/JSystem/include/JSystem/JAudio2/JASSeqReader.hpp:76 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: mSeqCursor | MacioGalaxy has a fix: libs/JSystem/include/JSystem/JAudio2/JASSeqReader.hpp |
| libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp:52 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: mData | review needed |
| libs/JSystem/include/JSystem/JAudio2/JAUSoundTable.hpp:55 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: mData | review needed |
| libs/JSystem/include/JSystem/JGeometry/TVec.hpp:396 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &b | review needed |
| libs/nw4r/include/nw4r/ut/Color.h:31 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &color | MacioGalaxy has a fix: libs/nw4r/include/nw4r/ut/Color.h |
| libs/nw4r/include/nw4r/ut/Color.h:42 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: this | MacioGalaxy has a fix: libs/nw4r/include/nw4r/ut/Color.h |
| libs/nw4r/include/nw4r/ut/Color.h:45 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: this | review needed |
| src/Game/AudioLib/AudSystem.cpp:828 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: reinterpret_cast< u8* >(msBasic) + 0x834 | review needed |
| src/Game/Map/KCollision.cpp:1477 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: octree + offset | MacioGalaxy has a fix: src/Game/Map/KCollision.cpp |
| src/Game/MapObj/DynamicCollisionObj.cpp:162 | objects | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: u16array | review needed |
| src/Game/MapObj/ItemDrill.cpp:686 | objects | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: reinterpret_cast< const u8* >(&JMath::sSinCosTable) + index * 8 + 4 | review needed |
| src/Game/MapObj/ItemDrill.cpp:1102 | objects | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: reinterpret_cast< const u8* >(&JMath::sSinCosTable) + ((index * 8) & 0x1FFF8) + 4 | review needed |
| src/Game/MapObj/RollingWater.cpp:82 | objects | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: reinterpret_cast< const u8* >(&JMath::sSinCosTable) + ((index * 8) & 0x1FFF8) + 4 | review needed |
| src/Game/MapObj/StarPiece.cpp:194 | objects | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &initColors[Color_Silver] | review needed |
| src/Game/Screen/MessageTagSkipTagProcessor.cpp:35 | enemies | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pParam | review needed |
| src/Game/Speaker/SpkWave.cpp:51 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: reinterpret_cast< uintptr_t >(mResource) + wave * 4 + 8 | review needed |
| src/Game/System/BinaryDataContentAccessor.cpp:46 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: mData + 0 | review needed |
| src/Game/System/BinaryDataContentAccessor.cpp:50 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: mData + 2 | review needed |
| src/Game/System/BinaryDataContentAccessor.cpp:55 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: mData + 0 | review needed |
| src/Game/System/GameDataAllGalaxyStorage.cpp:92 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pData | review needed |
| src/Game/System/GameDataAllGalaxyStorage.cpp:123 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pData | review needed |
| src/Game/System/GameDataPlayerStatus.cpp:38 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: accessor.getPointer(                      , pData) | review needed |
| src/Game/System/GameDataPlayerStatus.cpp:41 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: accessor.getPointer(                 , pData) | review needed |
| src/Game/System/GameDataPlayerStatus.cpp:44 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: accessor.getPointer(                 , pData) | review needed |
| src/Game/System/GameDataPlayerStatus.cpp:62 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pStarPiece | review needed |
| src/Game/System/GameDataPlayerStatus.cpp:65 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pCoin | review needed |
| src/Game/System/GameDataPlayerStatus.cpp:68 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pLast1up | review needed |
| src/Game/System/GameDataSomeScenarioAccessor.cpp:17 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: rAccessor.getPointer(           , pData) | review needed |
| src/Game/System/MessageSystem.cpp:102 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pData->getNodeBranchPtr(flowIndex) | review needed |
| src/Game/System/MessageSystem.cpp:112 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: reinterpret_cast< u8* >(pData->getNodeBranchPtr(flowIndex)) + 2 | review needed |
| src/Game/System/SysConfigFile.cpp:155 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: accessor.getPointer(            , data) | review needed |
| src/Game/System/SysConfigFile.cpp:159 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: accessor.getPointer(                   , data) | review needed |
| src/Game/System/SysConfigFile.cpp:163 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: accessor.getPointer(                   , data) | review needed |
| src/Game/System/SysConfigFile.cpp:171 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: accessor.getPointer(                     , data) | review needed |
| src/Game/System/WorldMapPointParts.cpp:287 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: reinterpret_cast<u8*>(find(pointIndex)->mActor) + 0x94 | review needed |
| src/Game/Util/BellyDragonTU04.cpp:97 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ((u8*)pObj + 0xD4) | review needed |
| src/Game/Util/CShader.cpp:118 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pOutput | review needed |
| src/Game/Util/CShader.cpp:119 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pPositions | review needed |
| src/Game/Util/CShader.cpp:120 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pNormals | review needed |
| src/Game/Util/CShader.cpp:122 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pOutput | review needed |
| src/Game/Util/CShader.cpp:123 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pPositions | review needed |
| src/Game/Util/CShader.cpp:124 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pNormals | review needed |
| src/Game/Util/CShader.cpp:216 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pRead + 1 | review needed |
| src/Game/Util/CShader.cpp:273 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pVertex + posOffset | review needed |
| src/Game/Util/CShader.cpp:274 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pVertex + nrmOffset | review needed |
| src/Game/Util/CShader.cpp:275 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pVertex + texOffset | review needed |
| src/Game/Util/CShader.cpp:330 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pRead + 1 | review needed |
| src/Game/Util/CShader.cpp:333 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (pRead + stride * i) + (posOffset + 3) | review needed |
| src/Game/Util/MemoryUtil.cpp:206 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pSrcBytes | review needed |
| src/Game/Util/MemoryUtil.cpp:262 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pDstBytes | review needed |
| src/Game/Util/PadRecorder.cpp:115 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: mPacketBuffer | review needed |
| src/Game/Util/PadRecorder.cpp:206 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: mPacketBuffer | review needed |
| src/Game/Util/TDDraw.cpp:556 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &pTexture[sourceOffset] | review needed |
| src/Game/Util/TDDraw.cpp:557 | core | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &pTexture[sourceOffset + 4] | review needed |
| src/JSystem/J3DGraphLoader/J3DModelLoader.cpp:80 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ((uintptr_t)i_data + 4) | review needed |
| src/JSystem/J3DGraphLoader/J3DModelLoader.cpp:83 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ((uintptr_t)i_data + 4) | review needed |
| src/JSystem/J3DGraphLoader/J3DModelLoader.cpp:87 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ((uintptr_t)i_data + 4) | review needed |
| src/JSystem/JAudio2/JAISound.cpp:197 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &mSoundID | review needed |
| src/JSystem/JKernel/JKRDecomp.cpp:204 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (pSrc + 4) | MacioGalaxy has a fix: src/JSystem/JKernel/JKRDecomp.cpp |
| src/JSystem/JKernel/JKRDecomp.cpp:213 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pSrc | review needed |
| src/JSystem/JParticle/JPAResourceLoader.cpp:25 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (data + 4) | MacioGalaxy has a fix: src/JSystem/JParticle/JPAResourceLoader.cpp |
| src/JSystem/JParticle/JPAResourceLoader.cpp:40 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (data + 8) | review needed |
| src/JSystem/JParticle/JPAResourceLoader.cpp:41 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (data + 0xA) | review needed |
| src/JSystem/JParticle/JPAResourceLoader.cpp:46 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (data + 8) | review needed |
| src/JSystem/JParticle/JPAResourceLoader.cpp:62 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (data + offset) | review needed |
| src/JSystem/JParticle/JPAResourceLoader.cpp:63 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (data + offset + 4) | review needed |
| src/JSystem/JParticle/JPAResourceLoader.cpp:99 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (data + 0xC) | review needed |
| src/JSystem/JParticle/JPAResourceLoader.cpp:100 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (data + 0xA) | review needed |
| src/JSystem/JParticle/JPAResourceLoader.cpp:101 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (data + offset + 4) | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_acos.c:9 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_asin.c:8 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: __float_nan | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_asin.c:12 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_asin.c:16 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_atan2.c:8 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_atan2.c:11 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &y | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_atan2.c:93 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &z | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_fmod.c:9 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_fmod.c:11 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &y | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_fmod.c:147 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_fmod.c:166 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_log.c:14 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_log.c:30 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_log.c:40 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_log10.c:16 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_log10.c:33 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_log10.c:44 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_pow.c:43 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_pow.c:45 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &y | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_pow.c:180 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &ax | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_pow.c:200 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &ax | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_pow.c:209 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &t_h | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_pow.c:246 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &z | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_pow.c:278 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &t | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_pow.c:298 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &z | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_pow.c:305 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &z | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_rem_pio2.c:22 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_rem_pio2.c:73 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &y[0] | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_rem_pio2.c:81 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &y[0] | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_rem_pio2.c:111 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &z | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_sqrt.c:13 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_sqrt.c:135 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &z | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/k_cos.c:3 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/k_cos.c:22 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &qx | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/k_sin.c:3 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/k_tan.c:11 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_atan.c:20 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_ceil.c:5 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_ceil.c:67 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_copysign.c:2 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &y | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_cos.c:7 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_floor.c:7 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_floor.c:69 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_frexp.c:3 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_frexp.c:14 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_frexp.c:21 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_ldexp.c:10 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_ldexp.c:20 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_ldexp.c:39 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_ldexp.c:53 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_sin.c:7 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_tan.c:7 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &x | review needed |
| src/MSL_C/math_api.c:26 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &dbl | review needed |
| src/MSL_C/math_api.c:30 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &dbl | review needed |
| src/MSL_C/math_api.c:32 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &dbl | review needed |
| src/MSL_C/math_api.c:41 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &dbl | review needed |
| src/MSL_C/printf.c:1168 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: buff_ptr | review needed |
| src/MSL_C/scanf.c:388 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: arg_ptr | review needed |
| src/MSL_C/scanf.c:534 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: arg_ptr | review needed |
| src/MSL_C/scanf.c:736 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: arg_ptr | review needed |
| src/MSL_C/string.c:37 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (fromb) | review needed |
| src/MSL_C/string.c:140 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: left | review needed |
| src/MSL_C/string.c:141 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: right | review needed |
| src/MSL_C/wprintf.c:1194 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: buff_ptr | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Os/dolphin/dolphin_trk.c:189 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: r3 | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Os/dolphin/dolphin_trk.c:220 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: p3 | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Os/dolphin/dolphin_trk.c:225 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: p3 | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Os/dolphin/dolphin_trk.c:264 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: p3 | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Os/dolphin/dolphin_trk.c:270 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: p3 | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Os/dolphin/dolphin_trk.c:301 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: p3 | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Portable/msg.c:19 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (msg->data + 6) | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Portable/msghndlr.c:82 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (buffer->data + 16) | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Portable/msghndlr.c:83 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (buffer->data + 12) | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Portable/msghndlr.c:150 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (&b->data[16]) | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Portable/msghndlr.c:151 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (&b->data[12]) | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Portable/msghndlr.c:215 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (b->data + 12) | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Portable/msghndlr.c:216 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (b->data + 16) | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Portable/msghndlr.c:286 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (b->data + 12) | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Portable/msghndlr.c:287 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (b->data + 16) | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Portable/msghndlr.c:387 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &b->data[16] | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Portable/msghndlr.c:388 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &b->data[20] | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Portable/nubinit.c:55 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: bendian | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Portable/nubinit.c:57 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: bendian | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Portable/support.c:61 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (replyBuffer->data + 0x10) | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Portable/support.c:62 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (replyBuffer->data + 0x14) | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Portable/support.c:175 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (tempBuffer->data + 0x10) | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Portable/support.c:176 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (tempBuffer->data + 0x8) | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Portable/support.c:210 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (buffer2->data + 0x10) | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Portable/support.c:249 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (buffer2->data + 0x10) | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Portable/support.c:250 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (buffer2->data + 0x18) | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:65 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: _p_ | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:716 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (reply._10 + 4) | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:717 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (reply._10 + 8) | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:721 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: reply._10 | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:778 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: reply._10 | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:907 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: gTRKCPUState.Default.GPR[5] | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:915 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: gTRKCPUState.Default.GPR[5] | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:1047 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (srcDestPtr) | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:1107 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: asm_access | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:1159 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: 0x800000DC | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:1166 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: 0x800000E4 | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:1171 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (thread + 0x2FC) | review needed |
| src/RVLFaceLib/RFL_MiddleDatabase.c:256 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &db->data[hparam->dstIdx] | review needed |
| src/RVLFaceLib/RFL_Model.c:803 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ptr8 | review needed |
| src/RVLFaceLib/RFL_Model.c:812 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ptr8 | review needed |
| src/RVLFaceLib/RFL_Model.c:867 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ptr8 | review needed |
| src/RVLFaceLib/RFL_Model.c:907 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ptr8 | review needed |
| src/RVLFaceLib/RFL_NANDLoader.c:126 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ((u8*)headerBuf1 + 2) | review needed |
| src/RVL_SDK/dvd/dvd.c:677 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3198) | review needed |
| src/RVL_SDK/dvd/dvd.c:679 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3194) | review needed |
| src/RVL_SDK/dvd/dvd.c:680 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3198) | review needed |
| src/RVL_SDK/exi/EXICommon.c:14 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: reg | review needed |
| src/RVL_SDK/exi/EXICommon.c:17 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: reg | review needed |
| src/RVL_SDK/fs/fs.c:114 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ptr | review needed |
| src/RVL_SDK/fs/fs.c:147 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ptr | review needed |
| src/RVL_SDK/fs/fs.c:149 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ptr | review needed |
| src/RVL_SDK/gd/GDPixel.c:51 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &A_f | review needed |
| src/RVL_SDK/gd/GDPixel.c:52 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &C | review needed |
| src/RVL_SDK/gx/GXDisplayList.c:74 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (&gx->vNumNot) | review needed |
| src/RVL_SDK/gx/GXGeometry.c:148 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (&gx->vNumNot) | review needed |
| src/RVL_SDK/gx/GXLight.c:166 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (&color) | review needed |
| src/RVL_SDK/gx/GXLight.c:227 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (&amb_color) | review needed |
| src/RVL_SDK/gx/GXLight.c:233 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (&amb_color) | review needed |
| src/RVL_SDK/gx/GXLight.c:248 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (&amb_color) | review needed |
| src/RVL_SDK/gx/GXLight.c:252 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (&amb_color) | review needed |
| src/RVL_SDK/gx/GXLight.c:269 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (&mat_color) | review needed |
| src/RVL_SDK/gx/GXLight.c:276 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (&mat_color) | review needed |
| src/RVL_SDK/gx/GXLight.c:294 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (&mat_color) | review needed |
| src/RVL_SDK/gx/GXLight.c:299 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (&mat_color) | review needed |
| src/RVL_SDK/gx/GXMisc.c:217 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: addr | review needed |
| src/RVL_SDK/gx/GXMisc.c:225 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: addr | review needed |
| src/RVL_SDK/gx/GXPixel.c:62 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &c | review needed |
| src/RVL_SDK/gx/GXPixel.c:76 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (&color) | review needed |
| src/RVL_SDK/gx/GXTev.c:366 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (&color) | review needed |
| src/RVL_SDK/gx/GXTev.c:384 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (&color) | review needed |
| src/RVL_SDK/gx/GXTev.c:385 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (&color) | review needed |
| src/RVL_SDK/gx/GXTev.c:403 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (&color) | review needed |
| src/RVL_SDK/nwc24/NWC24Config.c:192 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSGetAppGamename() | review needed |
| src/RVL_SDK/nwc24/NWC24Config.c:195 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: DVDGetCurrentDiskID()->gameName | review needed |
| src/RVL_SDK/nwc24/NWC24Config.c:215 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: DVDGetCurrentDiskID()->company | review needed |
| src/RVL_SDK/nwc24/NWC24Ipc.c:56 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: pArg | review needed |
| src/RVL_SDK/nwc24/NWC24System.c:66 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: shtBuffer | review needed |
| src/RVL_SDK/os/OS.c:145 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToUncached(0x3140) | review needed |
| src/RVL_SDK/os/OS.c:146 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToUncached(0x3144) | review needed |
| src/RVL_SDK/os/OS.c:160 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached((0x3138)) | review needed |
| src/RVL_SDK/os/OS.c:495 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ((u32)bi2StartAddr + 0x24) | review needed |
| src/RVL_SDK/os/OS.c:510 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x310C) | review needed |
| src/RVL_SDK/os/OS.c:528 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3110) | review needed |
| src/RVL_SDK/os/OS.c:537 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3124) | review needed |
| src/RVL_SDK/os/OS.c:557 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3128) | review needed |
| src/RVL_SDK/os/OS.c:619 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3188) | review needed |
| src/RVL_SDK/os/OS.c:692 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: destAddr | review needed |
| src/RVL_SDK/os/OSAudioSystem.c:119 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &__DSPRegs[0x10] | review needed |
| src/RVL_SDK/os/OSAudioSystem.c:120 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &__DSPRegs[0x12] | review needed |
| src/RVL_SDK/os/OSAudioSystem.c:121 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &__DSPRegs[0x14] | review needed |
| src/RVL_SDK/os/OSAudioSystem.c:135 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &__DSPRegs[0x10] | review needed |
| src/RVL_SDK/os/OSAudioSystem.c:136 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &__DSPRegs[0x12] | review needed |
| src/RVL_SDK/os/OSAudioSystem.c:137 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &__DSPRegs[0x14] | review needed |
| src/RVL_SDK/os/OSContext.c:490 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: *p | review needed |
| src/RVL_SDK/os/OSError.c:47 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: *stackPtr | review needed |
| src/RVL_SDK/os/OSExec.c:59 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &bootInfo2[8] | review needed |
| src/RVL_SDK/os/OSExec.c:76 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (list - 1) | review needed |
| src/RVL_SDK/os/OSExec.c:77 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &bootInfo2[8] | review needed |
| src/RVL_SDK/os/OSExec.c:130 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &bootInfo2[8] | review needed |
| src/RVL_SDK/os/OSExec.c:156 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (list - 1) | review needed |
| src/RVL_SDK/os/OSExec.c:158 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &bootInfo2[8] | review needed |
| src/RVL_SDK/os/OSExec.c:220 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3198) | review needed |
| src/RVL_SDK/os/OSExec.c:248 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3194) | review needed |
| src/RVL_SDK/os/OSExec.c:249 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3198) | review needed |
| src/RVL_SDK/os/OSExec.c:347 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3194) | review needed |
| src/RVL_SDK/os/OSExec.c:348 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3198) | review needed |
| src/RVL_SDK/os/OSExec.c:424 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x311C) | review needed |
| src/RVL_SDK/os/OSExec.c:425 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3120) | review needed |
| src/RVL_SDK/os/OSExec.c:438 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x311C) | review needed |
| src/RVL_SDK/os/OSExec.c:439 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3120) | review needed |
| src/RVL_SDK/os/OSExec.c:440 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3128) | review needed |
| src/RVL_SDK/os/OSExec.c:441 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3130) | review needed |
| src/RVL_SDK/os/OSExec.c:442 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3134) | review needed |
| src/RVL_SDK/os/OSExec.c:443 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x311C) | review needed |
| src/RVL_SDK/os/OSExec.c:446 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3120) | review needed |
| src/RVL_SDK/os/OSExec.c:582 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ((u32)tgcHeader + 56) | review needed |
| src/RVL_SDK/os/OSExec.c:734 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: diskId->gameName | review needed |
| src/RVL_SDK/os/OSFont.c:261 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &s[4] | review needed |
| src/RVL_SDK/os/OSFont.c:262 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &s[8] | review needed |
| src/RVL_SDK/os/OSFont.c:263 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &s[12] | review needed |
| src/RVL_SDK/os/OSFont.c:298 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &buf[4] | review needed |
| src/RVL_SDK/os/OSFont.c:308 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0xCC) | review needed |
| src/RVL_SDK/os/OSFont.c:392 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: src | review needed |
| src/RVL_SDK/os/OSFont.c:492 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: string | review needed |
| src/RVL_SDK/os/OSInterrupt.c:76 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: 0xCC003004 | review needed |
| src/RVL_SDK/os/OSInterrupt.c:77 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: 0xCD000034 | review needed |
| src/RVL_SDK/os/OSIpc.c:16 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3100 + 0x0030) | review needed |
| src/RVL_SDK/os/OSIpc.c:17 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3100 + 0x0034) | review needed |
| src/RVL_SDK/os/OSLaunch.c:37 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &bootInfo2[8] | review needed |
| src/RVL_SDK/os/OSLaunch.c:56 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ptr | review needed |
| src/RVL_SDK/os/OSLaunch.c:57 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &bootInfo2[8] | review needed |
| src/RVL_SDK/os/OSLaunch.c:65 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: tmd->head.reserved | review needed |
| src/RVL_SDK/os/OSLaunch.c:115 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: diskId->company | review needed |
| src/RVL_SDK/os/OSLaunch.c:122 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: diskId->company | review needed |
| src/RVL_SDK/os/OSLaunch.c:460 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &bi2[8] | review needed |
| src/RVL_SDK/os/OSLink.c:5 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: 0x800030CC | review needed |
| src/RVL_SDK/os/OSLink.c:6 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: 0x800030C8 | review needed |
| src/RVL_SDK/os/OSLink.c:7 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: 0x800030D0 | review needed |
| src/RVL_SDK/os/OSMemory.c:19 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3100) | review needed |
| src/RVL_SDK/os/OSMemory.c:23 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3118) | review needed |
| src/RVL_SDK/os/OSMemory.c:27 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3104) | review needed |
| src/RVL_SDK/os/OSMemory.c:31 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x311C) | review needed |
| src/RVL_SDK/os/OSMemory.c:510 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0x3120) | review needed |
| src/RVL_SDK/rso/RSOLink.c:429 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: p | review needed |
| src/RVL_SDK/rso/RSOLink.c:432 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: p | review needed |
| src/RVL_SDK/rso/RSOLink.c:435 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: p | review needed |
| src/RVL_SDK/rso/RSOLink.c:439 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: p | review needed |
| src/RVL_SDK/rso/RSOLink.c:548 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (p + 1) | review needed |
| src/RVL_SDK/sc/scsystem.c:100 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: bufp | review needed |
| src/RVL_SDK/sc/scsystem.c:470 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (conf + refp[-id]) | review needed |
| src/RVL_SDK/sc/scsystem.c:520 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (conf + 4) | review needed |
| src/RVL_SDK/sc/scsystem.c:629 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (conf + 4) | review needed |
| src/RVL_SDK/si/SIBios.c:72 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: input | review needed |
| src/RVL_SDK/vf/nand_drv.c:27 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: 0x800000F8 | review needed |
| src/RVL_SDK/vf/nand_drv.c:459 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ((u8*)realDisk_p + 0x1BC) | review needed |
| src/RVL_SDK/vf/pf_entry.c:14 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (addr) | review needed |
| src/RVL_SDK/vf/pf_entry.c:16 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (addr) | review needed |
| src/RVL_SDK/vf/pf_entry.c:18 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (addr) | review needed |
| src/RVL_SDK/vf/pf_entry.c:21 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (addr) | review needed |
| src/RVL_SDK/vf/pf_entry.c:278 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: &buf[26] | review needed |
| src/RVL_SDK/vf/pf_fat12.c:65 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: buf | review needed |
| src/RVL_SDK/vf/pf_fat12.c:211 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: buf | review needed |
| src/RVL_SDK/vf/pf_fat16.c:62 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: buf | review needed |
| src/RVL_SDK/vf/pf_fat16.c:130 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ((*pp_page)->p_buf + offset) | review needed |
| src/RVL_SDK/vf/pf_fat16.c:150 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: buf | review needed |
| src/RVL_SDK/vf/pf_fat32.c:62 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: buf | review needed |
| src/RVL_SDK/vf/pf_fat32.c:130 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ((*pp_page)->p_buf + offset) | review needed |
| src/RVL_SDK/vf/pf_fat32.c:161 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: buf | review needed |
| src/RVL_SDK/vf/pf_fat32.c:164 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: buf | review needed |
| src/RVL_SDK/vf/pf_fat32.c:236 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ((*pp_page)->p_buf + fat_offset) | review needed |
| src/RVL_SDK/vf/pf_fat32.c:239 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: ((*pp_page)->p_buf + fat_offset) | review needed |
| src/RVL_SDK/vf/pf_path.c:135 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (p_name->buf + p_name->index) | review needed |
| src/RVL_SDK/vf/pf_path.c:179 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: p_pattern->p_head | review needed |
| src/RVL_SDK/vf/pf_path.c:180 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: p_pattern->p_head | review needed |
| src/RVL_SDK/vf/pf_str.c:131 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (p1 - 2) | review needed |
| src/RVL_SDK/vf/pf_str.c:133 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (p1 - 2) | review needed |
| src/RVL_SDK/vf/pf_str.c:165 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (p1 - 2) | review needed |
| src/RVL_SDK/vf/pf_str.c:167 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (p1 - 2) | review needed |
| src/RVL_SDK/vi/vi.c:661 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0xCC) | review needed |
| src/RVL_SDK/vi/vi.c:788 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0xCC) | review needed |
| src/RVL_SDK/vi/vi.c:1092 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0xCC) | review needed |
| src/RVL_SDK/vi/vi3in1.c:518 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: OSPhysicalToCached(0xCC) | review needed |
| src/RVL_SDK/wpad/WPADHIDParser.c:441 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: (data + length) | review needed |
| src/nw4r/db/db_assert.cpp:61 | m1 | multi-byte pointer dereference over byte/resource memory; review byte order and alignment; operand: *p | review needed |

### integer-pointer-storage — review (71)

| File:line | Lane | Evidence | Reference |
| --- | --- | --- | --- |
| include/Game/System/PlayResultInStageHolder.hpp:79 | core | 32-bit field documented as holding an address/pointer | review needed |
| src/Game/NPC/Dreamer.cpp:191 | enemies | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: (u32) | review needed |
| src/Game/NPC/KinopioPostman.cpp:142 | enemies | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: (u32) | review needed |
| src/Game/NPC/KinopioPostman.cpp:146 | enemies | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: (u32) | review needed |
| src/Game/NPC/KinopioPostman.cpp:150 | enemies | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: (u32) | review needed |
| src/Game/NPC/KinopioPostman.cpp:154 | enemies | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: (u32) | review needed |
| src/Game/NPC/KinopioPostman.cpp:158 | enemies | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: (u32) | review needed |
| src/Game/NPC/KinopioPostman.cpp:162 | enemies | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: (u32) | review needed |
| src/Game/NPC/KinopioPostman.cpp:166 | enemies | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: (u32) | review needed |
| src/Game/NPC/KinopioPostman.cpp:170 | enemies | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: (u32) | review needed |
| src/Game/NPC/KinopioPostman.cpp:174 | enemies | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: (u32) | review needed |
| src/Game/NPC/MeisterTU05.cpp:72 | enemies | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: (u32) | review needed |
| src/Game/NPC/MeisterTU05.cpp:105 | enemies | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: (u32) | review needed |
| src/Game/NPC/SuperDreamer.cpp:278 | enemies | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: (u32) | review needed |
| src/Game/NPC/SuperDreamer.cpp:280 | enemies | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: (u32) | review needed |
| src/Game/System/WorldMapResultTU11.cpp:796 | core | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: (u32) | review needed |
| src/Game/System/WorldMapResultTU23.cpp:19 | core | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: (u32) | review needed |
| src/Game/System/WorldMapResultTU23.cpp:30 | core | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: (u32) | review needed |
| src/JSystem/JAudio2/JASHeapCtrl.cpp:302 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: sAramBase | review needed |
| src/JSystem/JKernel/JKRAramPiece.cpp:93 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: requestAddress | review needed |
| src/JSystem/JUtility/JUTException.cpp:246 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: module->sectionInfoOffset | review needed |
| src/JSystem/JUtility/JUTException.cpp:277 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: mStackPointer | review needed |
| src/JSystem/JUtility/JUTException.cpp:291 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: stackPointer[0] | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Os/dolphin/dolphin_trk.c:189 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: r3 | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Os/dolphin/dolphin_trk.c:274 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: counter | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Os/dolphin/dolphin_trk.c:297 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: c | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Os/dolphin/dolphin_trk.c:315 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: g | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:907 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: gTRKCPUState.Default.GPR[5] | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:915 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: gTRKCPUState.Default.GPR[5] | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:917 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: gTRKCPUState.Default.GPR[5] | review needed |
| src/MetroTRK/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c:919 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: gTRKCPUState.Default.GPR[6] | review needed |
| src/MetroTRK/gamedev/cust_connection/cc/exi2/GCN/EXI2_GDEV_GCN/main.c:90 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: hexCopy | review needed |
| src/RVLFaceLib/RFL_MiddleDatabase.c:107 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: arg | review needed |
| src/RVLFaceLib/RFL_MiddleDatabase.c:286 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: arg | review needed |
| src/RVL_SDK/aralt/aralt.c:27 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: ram_addr | review needed |
| src/RVL_SDK/aralt/aralt.c:28 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: ram_addr | review needed |
| src/RVL_SDK/aralt/aralt.c:29 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: aram_addr | review needed |
| src/RVL_SDK/aralt/aralt.c:32 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: ram_addr | review needed |
| src/RVL_SDK/aralt/aralt.c:33 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: ram_addr | review needed |
| src/RVL_SDK/aralt/aralt.c:34 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: aram_addr | review needed |
| src/RVL_SDK/gx/GXMisc.c:217 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: addr | review needed |
| src/RVL_SDK/gx/GXMisc.c:225 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: addr | review needed |
| src/RVL_SDK/os/OSContext.c:490 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: *p | review needed |
| src/RVL_SDK/os/OSError.c:47 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: *stackPtr | review needed |
| src/RVL_SDK/os/OSMemory.c:70 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: start | review needed |
| src/RVL_SDK/os/OSThread.c:404 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: sp | review needed |
| src/RVL_SDK/os/OSThread.c:405 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: sp | review needed |
| src/RVL_SDK/os/OSThread.c:654 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: sp | review needed |
| src/RVL_SDK/rso/RSOLink.c:70 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: moduleHeader->mInfo.mSectionInfoOffset | review needed |
| src/RVL_SDK/rso/RSOLink.c:80 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: moduleHeader->mInfo.mSectionInfoOffset | review needed |
| src/RVL_SDK/rso/RSOLink.c:83 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: moduleHeader->mInfo.mSectionInfoOffset | review needed |
| src/RVL_SDK/rso/RSOLink.c:86 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: moduleHeader->mInfo.mSectionInfoOffset | review needed |
| src/RVL_SDK/rso/RSOLink.c:91 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: moduleHeader->mInternalRelOffset | review needed |
| src/RVL_SDK/rso/RSOLink.c:93 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: moduleHeader->mInfo.mSectionInfoOffset | review needed |
| src/RVL_SDK/rso/RSOLink.c:102 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: moduleHeader->mExternalRelOffset | review needed |
| src/RVL_SDK/rso/RSOLink.c:133 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: moduleHeader->mInfo.mSectionInfoOffset | review needed |
| src/RVL_SDK/rso/RSOLink.c:192 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: moduleHeader->mExternalRelOffset | review needed |
| src/RVL_SDK/rso/RSOLink.c:199 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: imp->mTableOffset | review needed |
| src/RVL_SDK/rso/RSOLink.c:253 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: imp->mTableOffset | review needed |
| src/RVL_SDK/rso/RSOLink.c:318 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: rso->mExpHeader.mTableOffset | review needed |
| src/RVL_SDK/rso/RSOLink.c:319 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: rso->mInfo.mSectionInfoOffset | review needed |
| src/RVL_SDK/rso/RSOLink.c:324 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: exp->mTableOffset | review needed |
| src/RVL_SDK/rso/RSOLink.c:333 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: rso->mExpHeader.mTableOffset | review needed |
| src/RVL_SDK/rso/RSOLink.c:477 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: rso->mExpHeader.mTableOffset | review needed |
| src/RVL_SDK/rso/RSOLink.c:493 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: rsoImp->mExternalRelOffset | review needed |
| src/RVL_SDK/rso/RSOLink.c:547 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: rsoExp->mInfo.mSectionInfoOffset | review needed |
| src/RVL_SDK/rso/RSOLink.c:708 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: imp->mTableOffset | review needed |
| src/RVL_SDK/vi/vi.c:1236 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: NextBufAddr | review needed |
| src/RVL_SDK/vi/vi.c:1240 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: CurrBufAddr | review needed |
| src/nw4r/db/db_assert.cpp:52 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: sp | review needed |
| src/nw4r/db/db_assert.cpp:61 | m1 | 32-bit integer used as a pointer; distinguish addresses from deliberate wire offsets; operand: *p | review needed |

### packed-layout — review (11)

| File:line | Lane | Evidence | Reference |
| --- | --- | --- | --- |
| include/Game/Speaker/SpkSpeakerCtrl.hpp:7 | core | packed layout: audit multi-byte access, serialized offsets, and pointer-bearing members | review needed |
| include/Game/Speaker/SpkSpeakerCtrl.hpp:43 | core | packed layout: audit multi-byte access, serialized offsets, and pointer-bearing members | review needed |
| libs/RVL_SDK/include/revolution/esp.h:24 | m1 | packed layout: audit multi-byte access, serialized offsets, and pointer-bearing members | review needed |
| libs/RVL_SDK/include/revolution/nwc24/internal/NWC24iDownload.h:36 | m1 | packed layout: audit multi-byte access, serialized offsets, and pointer-bearing members | review needed |
| libs/RVL_SDK/include/revolution/nwc24/internal/NWC24iDownload.h:68 | m1 | packed layout: audit multi-byte access, serialized offsets, and pointer-bearing members | review needed |
| libs/RVL_SDK/include/revolution/types.h:103 | m1 | packed layout: audit multi-byte access, serialized offsets, and pointer-bearing members | MacioGalaxy has a fix: libs/RVL_SDK/include/revolution/types.h |
| libs/libms/include/libms.h:15 | m1 | packed layout: audit multi-byte access, serialized offsets, and pointer-bearing members | review needed |
| src/RVL_SDK/nwc24/NWC24Schedule.c:19 | m1 | packed layout: audit multi-byte access, serialized offsets, and pointer-bearing members | review needed |
| src/RVL_SDK/nwc24/NWC24Schedule.c:31 | m1 | packed layout: audit multi-byte access, serialized offsets, and pointer-bearing members | review needed |
| src/RVL_SDK/nwc24/NWC24Time.c:20 | m1 | packed layout: audit multi-byte access, serialized offsets, and pointer-bearing members | review needed |
| src/RVL_SDK/nwc24/NWC24Time.c:26 | m1 | packed layout: audit multi-byte access, serialized offsets, and pointer-bearing members | review needed |

### pointer-storage32 — review (2)

| File:line | Lane | Evidence | Reference |
| --- | --- | --- | --- |
| src/Game/Player/Mario.cpp:14187 | astra | pointer assigned to an apparent 32-bit slot; verify receiver type and local scope; operand: pActor | review needed |
| src/RVL_SDK/vf/d_vf_sys.c:392 | m1 | pointer assigned to an apparent 32-bit slot; verify receiver type and local scope; operand: i_drive_p->cache.pf_cache_page_p | review needed |

### union-pointer-alias — review (10)

| File:line | Lane | Evidence | Reference |
| --- | --- | --- | --- |
| include/Game/Enemy/BattanKing.hpp:167 | enemies | Anonymous union contains native pointers; verify consumers, growth alone is not proof of a bug | review needed |
| include/Game/NameObj/NameObjCategoryList.hpp:60 | core | Anonymous union contains native pointers; verify consumers, growth alone is not proof of a bug | review needed |
| include/Game/System/NANDManager.hpp:26 | core | Anonymous union contains native pointers; verify consumers, growth alone is not proof of a bug | review needed |
| include/Game/System/NANDManager.hpp:30 | core | Anonymous union contains native pointers; verify consumers, growth alone is not proof of a bug | review needed |
| libs/JSystem/include/JSystem/JAudio2/JASTaskThread.hpp:14 | m1 | Anonymous union contains native pointers; verify consumers, growth alone is not proof of a bug | review needed |
| src/RVL_SDK/bte/hidd_api.h:64 | m1 | Anonymous union contains native pointers; verify consumers, growth alone is not proof of a bug | review needed |
| src/RVL_SDK/bte/rfc_int.h:150 | m1 | Anonymous union contains native pointers; verify consumers, growth alone is not proof of a bug | review needed |
| src/RVL_SDK/bte/sdp_api.h:90 | m1 | Anonymous union contains native pointers; verify consumers, growth alone is not proof of a bug | review needed |
| src/RVL_SDK/fs/fs.c:36 | m1 | Anonymous union contains native pointers; verify consumers, growth alone is not proof of a bug | review needed |
| src/Runtime/ptmf.c:4 | m1 | Anonymous union contains native pointers; verify consumers, growth alone is not proof of a bug | review needed |


# Native text helper audit

Branch `game-utf16-text`, base `8abbfaa3`; 2026-10-20. Two native-only fixes,
with original Wii statements retained verbatim in `#else`. WorldMap08 is closed
as unsupported direct entry and was not changed. No push or landing performed.

## Fixes and original-code evidence

1. `Game/System/Language.cpp`, `MR::getCurrentRegionPrefix`: the native local
   array now holds two letters **plus** the terminator. The extraction length
   remains 2; changing only the array size while retaining `sizeof(prefix)` as
   the extraction count would copy three letters and still write past its end.
   DOL `804BD2D0..804BD390` passes destination `r1+8`, count 2, fourth argument
   3, within a 16-byte frame. `extractString` at `8005F530..8005F570` ignores the
   fourth argument, calls `strncpy`, then stores zero at destination+count.
   Thus bytes at stack offsets 8, 9, 10 are used. The reconstructed `char[2]`
   understates the required C object extent; the DOL cannot tell us the original
   declared array size. This is a decompilation object-extent error exposed by
   native sanitizers, independent of UTF-16 byte order. Fix the caller, preserving
   the shared helper's actual count semantics. No shared signature changed.
2. `Game/Util/StringUtil.cpp`, `MR::convertUTF16ToASCII`: native code reads a
   typed UTF-16 unit and accepts `0x01..0xFF`, stopping on zero or a nonzero high
   byte. DOL function `8005F570..8005F5C0`, especially `lbz +0` at `8005F584`,
   `lbz +1` at `8005F590` and stride 2 at `8005F5A4`, establishes that contract.
   The function name does not imply a 7-bit restriction: bytes 0x80..0xFF must
   remain accepted. The `u32 i` versus signed `num - 1` comparison, termination
   write, and return count are unchanged. No new zero/negative-capacity policy
   was introduced; callers must supply a valid positive destination capacity.

The pattern for host text is typed code-unit access, followed by numeric masks
or shifts when a serialized byte is needed. Existing tag accessors already use
that pattern. There is only one new confirmed host-order byte-test defect here;
adding a new abstraction for its single read would duplicate existing types.

## ABI and audit method

`native/tools/encode_legacy_sources.py` transforms legacy `wchar_t` identifiers
to `PetariChar16` (`char16_t`), `L` literals to `u` literals, and wide library
calls to the native UTF-16 functions. Host system headers retain four-byte
`wchar_t`. Native code must use the generated source/header overlay, not a raw
legacy TU against the host wide-character library. Consequently audited legacy
`sizeof(wchar_t)` expressions evaluate to **2**, including copy lengths in
CustomTagProcessor, ReplaceTagProcessor, TextBox, UserFile and RFL.

Searches covered all `src/` and `libs/`, including library headers and the actual
library implementations in `src/nw4r`, `src/RVL_SDK`, `src/MSL_C`, and
`src/RVLFaceLib`. Searches included byte pointer declarations/casts, UTF-16 and
wide-character names, tag fields, fixed-byte tests, numeric masks/shifts and
`sizeof(wchar_t)`. The keyword inventory contains 95 files; a separate whole-tree
byte-pointer search catches UTF-16 paths named only with `u16`, such as VF.
Results and inventories accompany the external REPORT in `evidence.tgz`.
This is a static classification of text accesses, not a claim that every helper
or serialized resource is otherwise correct or runtime-covered.

## Classification

Paths below are relative to the repository root. “Endian-safe” describes the
access under the stated representation contract. UNKNOWN rows remain unedited.

| Site / operation | Classification | Evidence / disposition |
| --- | --- | --- |
| `src/Game/System/Language.cpp:getCurrentRegionPrefix` | DEFECT, fixed natively | Two-byte C array versus count+terminator requiring three; DOL evidence above. |
| `src/Game/Util/StringUtil.cpp:convertUTF16ToASCII` | DEFECT, fixed natively | BE high/low byte tests on host UTF-16 replaced by typed unit test. |
| `src/Game/NPC/TalkNodeCtrl.cpp:233..250`, `getSubMessage` | UNKNOWN | Marker 0x1A, byte offset 2 into `mMessageText`, raw `((char*)mMessage)[1]`, tag test and parameter read describe a legacy packed-tag path. The byte test is endian-sensitive. No `getSubMessage` symbol in the SB4E01 symbol map and no identified DOL consumer. The source wrapper in TalkMessageCtrl is not proof of an original consumer. Cannot select a correct SMG2 replacement without establishing this path's format and use. |
| `src/libms/hostorder.c:fixText/fix16`, block/offset conversion | Endian-safe | Deliberate BE-to-host ingress; swaps each TXT2 two-byte unit once, using BOM to avoid repeated conversion. These raw bytes are disk data, not already host text. |
| `src/libms/msgfile.c:LMS_GetText`, label and attribute tables | Endian-safe | Byte arithmetic locates data; converted offsets and typed UTF-16 loads determine text. Label lengths and bytes are narrow text. |
| `src/Game/System/MarioFaceShipEventDataHolder.cpp:MessageData` and TalkMessageCtrl text getters | Endian-safe | Pointer transport through `u8*`; no character classification from individual UTF-16 bytes. |
| `src/Game/Screen/MessageTagSkipTagProcessor.cpp:getTagLength/getParamLength/getParam8/getParam32/getParamPtr` | Endian-safe native replacement already present | Raw BE implementations are under `#ifndef GALAXY_NATIVE`; native TU supplies the host-order accessors. |
| `src/Game/Screen/MessageEditorMessageTagNative.cpp`, tag header accessors in its header | Endian-safe | Group/tag/parameter length read as units; parameter 8-bit extraction uses numeric high/low shifts; 16/32-bit parameters use units; parameter pointers advance in units, rejecting odd byte offsets. String slots use handles. |
| `src/Game/Screen/ReplaceTagProcessor.cpp:Replace/ReplaceArgs`, `exe*Group` | Endian-safe for text representation | Marker 0xE and typed tag helpers; copy byte counts multiply unit counts by the transformed two-byte type. PictureTag consists of u16 fields. No WorldMap08 change. |
| `src/Game/Screen/CustomTagProcessor.cpp:setArgNumber/setArgString/exeNumberGroup/exeStringGroup` | Endian-safe native replacement already present | Raw s32/pointer casts remain Wii-only; native path uses parameter units and string handles. |
| `src/Game/Screen/CustomTagProcessor.cpp:exeSoundGroup` | Endian-safe after converter fix | `getParamPtr(2)` returns typed host text; converter supplies the narrow sound name. Parameter length arithmetic counts serialized bytes, then divides by two. |
| `src/Game/Screen/CustomTagProcessor.cpp:ruby/base copies and text reader loops` | Endian-safe | Typed context cursors and code units; `sizeof(wchar_t)` becomes 2. |
| `src/Game/Util/StringUtil.cpp:isMessageEditorNextTag/getStringLengthWithMessageTag/copyString` | Endian-safe | Full-unit 0xE/zero tests, typed tag skipping, and native UTF-16 string functions. Narrow string comparisons process narrow strings. |
| `src/Game/Util/StringUtil.cpp:addPictureFontTag/addNumberFontTag` | UNKNOWN legacy format / endian-safe packing | Existing native branches explicitly pack size/group with shifts into units. Helpers still use marker 0x1A; no in-tree call sites found. No evidence to turn them into active SMG2 0xE builders. |
| `src/Game/Util/MessageUtil.cpp:line/character/page loops`, NPC TalkTextFormer | Endian-safe | Typed marker/newline/null checks. Number/string argument casts have native unit/handle branches. |
| `src/nw4r/ut/ut_CharStrmReader.cpp:ReadNextCharUTF16` and `libs/nw4r/include/nw4r/ut/CharStrmReader.h` | Endian-safe | Loads and advances `u16`. UTF-8/SJIS/CP1252 branches intentionally read narrow bytes. |
| `src/nw4r/ut/ut_TextWriterBase.cpp`, `ut_TagProcessorBase.cpp`, WideTextWriter headers | Endian-safe | Wide stream cursors stay typed; code comparisons use u16. Narrow instantiations operate on narrow text. |
| `src/nw4r/lyt/lyt_nativeResource.cpp:txt1` and `lyt_layout.cpp:Build` | Endian-safe | NormalizeResource converts text string bytes with U16Array before TextBox reads them. |
| `src/nw4r/lyt/lyt_textBox.cpp` buffer lengths/copies and color byte casts | Endian-safe | Text uses generated two-byte types and host-normalized resource strings. `mTextColors` byte accesses are colors, not text. |
| StaffRoll/StaffRollLine/StaffRollUtil/StaffRollPart, LayoutCoreUtil, LayoutUtil, other screen message consumers | Endian-safe for text tests | Wide separators, `#`, `^`, digits, newline and zero use full units or reader.Next(). Region lookup defect fixed at its source. |
| UserFile, FileSelectInfo, FileSelector, FileSelectFunc, MiiSelect/MiiConfirmIcon; RFL name copies/validation | Endian-safe for text representation | Names are typed units or u16, with two-byte element sizes and unit comparisons. No separate player-name keyboard implementation was found. This does not certify every caller's buffer capacity. |
| `src/Game/NWC24/UTF16Util.cpp:strlenUTF16` and Messenger/ReceiverTagMail text handling | Endian-safe internally | Full u16 zero tests and typed copies/parameters. Export boundary is a separate row. |
| `src/Game/NWC24/NWC24SendThread.cpp:142` mail text export | UNKNOWN native export behavior | Cast exports raw host text but charset `NWC24_UTF_16` aliases `UTF_16BE`. Current native NWC24SetMsgText returns unavailable without reading bytes. A working mail provider needs an explicit boundary conversion and a consumer test before this path can be certified. |
| `src/MSL_C/wstring.c`, wmem, wide scan/print/ctype/conversion helpers | Endian-safe for unit accesses | Wide loops inspect units; wide/narrow adapters convert values. Decimal formatter `sig.text` is byte text, not UTF-16. Active native wide string/format calls use `native/text/utf16.cpp`, not host wchar_t. |
| `src/RVL_SDK/os/OSUtf.c` and OSFont UTF-16 branch | Endian-safe for host-unit input | Typed u16 loads/shifts for UTF-16 and surrogates; UTF-8/SJIS branches intentionally use bytes. |
| `src/RVL_SDK/vf/pf_cp932.c:Unicode2OEM`, `pf_w_clib.c`, typed VF comparisons | Endian-safe for unit accesses | Numeric `&0xFF`/`>>8` split values, not native byte positions. |
| `src/RVL_SDK/vf/pf_str.c:99`, `pf_path.c:184` zero tests | Endian-safe | Both bytes must be zero, so the result is independent of their order. |
| `src/RVL_SDK/vf/pf_path.c:382` and VF serialized name boundaries (`pf_str.c:186`, directory-entry macros) | UNKNOWN native VF path | GetNextTokenOfPath explicitly reconstructs BE from two bytes in code_mode 2, whereas other helpers load u16; serialization writes also mix fixed byte order and native loads. Not part of the native CMake provider sources. Needs a proved VF ingress/export contract and DOL binding before choosing a local correction; host-unit input would make the BE reconstruction wrong. No claim of native VF support. |
| SDK NWC24 narrow MIME/7-bit scans, VF OEM/SJIS names, MSL narrow strings, network/device names | Endian-safe for narrow text | Byte tests intentionally operate on byte encodings, not UTF-16 units. |

## Regression tests

`native/tests/text_helpers_tests.py` builds the actual implementation TUs and
actual generated headers with Clang C++17, `-O1 -fsanitize=address,undefined
-fno-sanitize-recover=all`, and dead strips unused game routines. The region
fixture uses production class declarations and minimal test constructors to
provide language state without DVD/NAND startup. It links the real NerveExecutor
and random constructors. The conversion fixture statically checks two-byte
PetariChar16 and four-byte host wchar_t. No game assets or network services are
used.

Under the shared native lock, with `nice -n 19`, all four commands passed:

```sh
python3 native/tests/text_helpers_tests.py --case region
python3 native/tests/text_helpers_tests.py --case region --baseline 8abbfaa3
python3 native/tests/text_helpers_tests.py --case conversion
python3 native/tests/text_helpers_tests.py --case conversion --baseline 8abbfaa3
```

The baseline mode requires the expected failure, not merely a nonzero exit.
Region: all 13 language IDs and the unknown-language fallback pass; old source
reports the write beyond its two-byte array in extractString. Conversion: 12
cases cover ordinary text, empty text, termination, truncation, capacity 1,
non-byte Unicode values, zero low bytes, bytes 0x80/0xFF, surrogate units and an
exact-length unterminated input; old source incorrectly returns length zero for
ordinary host-order text. The converter's valid-capacity precondition is retained.

Fresh before/after MWCC `Wii/1.0` builds used the official Ninja recipes and the
same unchanged headers, in isolated directories. `tools/team/objsame.py` reports
both **Language.o** and **StringUtil.o** equivalent (code, allocated data,
relocations and symbol identities; debug differences excluded). A separately
compiled additional Wii data symbol in each control object is detected as a
difference, showing the comparison actually inspects rebuilt objects. Recipes,
objects, logs and the proof runner are in the evidence archive. No full-game
build, DOL hash validation or StaffRoll stage replay is claimed for this branch.

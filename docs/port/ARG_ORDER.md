# Argument evaluation order sweep

Generated candidate inventory; no source fixes or native behavior claims.

Source root: `/Users/roberthaisfield/MacioGalaxy2-team/scratch-port/ht2/overlay`. All relative source locations below refer to this snapshot. Generated 2026-10-19T20:48:39.415825+00:00; per-file and tool hashes are retained in JSON.

```sh
python3 tools/port/argorder_sweep.py --root /Users/roberthaisfield/MacioGalaxy2-team/scratch-port/ht2/overlay src/Game src/JSystem src/nw4r src/RVLFaceLib src/RVL_SDK --model-root /Users/roberthaisfield/MacioGalaxy --output docs/port/ARG_ORDER.md --json-output /tmp/argorder.json
```

## Method and review rules

The native syntax baseline fails most translation units (docs/port/COMPILE_STATUS.md); this tool uses a C/C++ tokenizer, balanced (), [] and {}, template-id heuristics, top-level argument splitting and binary-operator precedence. It does not require a native build or dependencies.

Class A: two or more different arguments contain a classified effect. Parenthesized constructors, explicit operator calls, indirect calls and unexpanded macro invocations are included. C++17 function arguments are indeterminately sequenced, but their order remains unspecified. Class B: two operands of a potentially unsequenced binary operator contain effects. A containing B can appear in both classes; totals are sites, not unique bugs.

HIGH sources: RNG (including JPA get_r_* / get_rndm_*), read/pop/next/consume/extract, ++/-- and assignment. Any HIGH source plus an effect in a second operand makes a HIGH candidate. Unknown calls are MEDIUM, including getters not on the explicit reviewed-pure allowlist in the tool. Distinct objects may be independent: verify shared state/aliasing before fixing. Known-pure value queries and scalar/vector casts do not count as effects themselves; effects in their arguments still count. Random-parameter getters such as getScaleRndm are values, not RNG draws.

Brace initializer clauses are ordered and do not create A sites. Their expressions can still contain an A/B site, and an entire braced argument can reorder with another argument. Logical &&/||, comma, conditional branch selection, assignment RHS/LHS and shifts are not B sites (C++17). Deferred lambda bodies do not contribute to their caller; bodies are scanned separately. sizeof/decltype/noexcept operands and function declarations are skipped.

Language references: [function argument sequencing](https://eel.is/c++draft/expr.call#8), [ordered brace initializer clauses](https://eel.is/c++draft/dcl.init.list#4), [execution sequencing](https://eel.is/c++draft/intro.execution). The native audit uses C++17. For .c files the scanner conservatively also reports assignment and shift operand pairs as B; check whether each native build compiles these as C or C++. Initializer-list sequencing in C and MWCC-era language modes requires separate review.

Limits: no type/alias/overload resolution, macro expansion or includes. Conditional branches are combined conservatively; where that unbalances delimiters, separate GALAXY_NATIVE=0/1 views are scanned and merged. Unknown guards remain combined, not exhaustive build configurations. Macro replacement definitions are skipped; invocation arguments are candidates, whose actual expansion needs review. Template vs relational < ambiguity, complex declarators, user-defined casts, implicit operator calls/conversions/destructors and type-dependent unevaluated contexts need AST/manual follow-up. Read-vs-write pairs with only one effectful operand are outside this requested two-effect sweep. Const does not alone prove purity; do not broadly exclude get*/is*/calc*.

Reviewed read exclusions: PetariNative::readU16BE/readU24BE/readU32BE only assemble bytes (native/include/petari/endian.hpp:5). readReg is excluded only inside src/JSystem/JAudio2/JASSeqParser.cpp:213; it delegates to value queries (src/JSystem/JAudio2/JASRegisterParam.cpp:70; libs/JSystem/include/JSystem/JAudio2/JASTrack.hpp:144; JASTrackPort.hpp:17). Other read* calls remain HIGH. getNerveStep/getEnd and JPA parameter getters are known value queries; unlisted getters stay MEDIUM.

Fix only after reading original target asm: `python3 tools/team/sg.py diff <unit> <symbol>` (this helper builds; use `nice -n 10`, at most six jobs, and check disk space first). Introduce named temporaries in the observed DOL order, then pass those values to the original expression. Do not assume every function evaluates right-to-left or reorder FP arithmetic. Preserve the Wii match; coordinate any native-only guarded fix with the owning lane. This sweep edits no src/ or libs/ files.

## Totals

Scanned **2364 files**; **1643 candidate sites**.

| Class | HIGH | MEDIUM | Total |
| --- | ---: | ---: | ---: |
| A | 3 | 815 | 818 |
| B | 27 | 798 | 825 |

Primary scope: **1508 sites**, including **3 HIGH**. Optional SDK scope: **135 sites**, including **27 HIGH**. SDK C assignment/shift and unexpanded-macro candidates are conservative; HIGH does not establish shared-state interference or require a fix.

## Directory ownership

| Directory | Lane | Files | A HIGH | A MEDIUM | B HIGH | B MEDIUM |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| src/Game/Animation | core | 11 | 0 | 0 | 0 | 1 |
| src/Game/AreaObj | objects | 39 | 0 | 4 | 0 | 1 |
| src/Game/AudioLib | core | 43 | 0 | 3 | 0 | 6 |
| src/Game/Boss | objects | 166 | 0 | 67 | 0 | 5 |
| src/Game/Camera | core | 126 | 0 | 12 | 0 | 57 |
| src/Game/Demo | core | 39 | 0 | 28 | 0 | 1 |
| src/Game/Effect | core | 29 | 0 | 15 | 0 | 1 |
| src/Game/Enemy | enemies | 158 | 0 | 89 | 0 | 32 |
| src/Game/GameAudio | core | 7 | 0 | 2 | 0 | 0 |
| src/Game/Gravity | core | 16 | 0 | 0 | 0 | 0 |
| src/Game/LiveActor | core | 80 | 0 | 18 | 0 | 33 |
| src/Game/Map | core | 71 | 0 | 20 | 0 | 2 |
| src/Game/MapObj | objects | 333 | 0 | 82 | 0 | 27 |
| src/Game/NPC | enemies | 92 | 0 | 62 | 0 | 3 |
| src/Game/NWC24 | core | 7 | 0 | 0 | 0 | 2 |
| src/Game/NameObj | core | 13 | 0 | 1 | 0 | 0 |
| src/Game/Player | astra | 120 | 0 | 96 | 0 | 125 |
| src/Game/RhythmLib | enemies | 15 | 0 | 0 | 0 | 0 |
| src/Game/Ride | objects | 33 | 0 | 11 | 0 | 7 |
| src/Game/Scene | core | 23 | 0 | 5 | 0 | 1 |
| src/Game/Screen | enemies | 140 | 0 | 31 | 0 | 21 |
| src/Game/Speaker | core | 7 | 0 | 1 | 0 | 1 |
| src/Game/System | core | 164 | 0 | 33 | 0 | 23 |
| src/Game/Util | core | 116 | 0 | 67 | 0 | 271 |
| src/JSystem/J2DGraph | m1 | 7 | 0 | 9 | 0 | 0 |
| src/JSystem/J3DGraphAnimator | m1 | 11 | 0 | 19 | 0 | 1 |
| src/JSystem/J3DGraphBase | m1 | 13 | 0 | 65 | 0 | 7 |
| src/JSystem/J3DGraphLoader | m1 | 7 | 0 | 1 | 0 | 0 |
| src/JSystem/JAudio2 | m1 | 72 | 0 | 8 | 0 | 5 |
| src/JSystem/JGadget | m1 | 2 | 0 | 0 | 0 | 0 |
| src/JSystem/JKernel | m1 | 23 | 0 | 4 | 0 | 7 |
| src/JSystem/JMath | m1 | 3 | 0 | 0 | 0 | 7 |
| src/JSystem/JParticle | m1 | 16 | 3 | 4 | 0 | 45 |
| src/JSystem/JSupport | m1 | 5 | 0 | 0 | 0 | 1 |
| src/JSystem/JUtility | m1 | 12 | 0 | 1 | 0 | 0 |
| src/RVLFaceLib | m1 | 13 | 0 | 0 | 0 | 3 |
| src/RVL_SDK/ai | m1 | 1 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/aralt | m1 | 1 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/arc | m1 | 1 | 0 | 0 | 3 | 0 |
| src/RVL_SDK/ax | m1 | 4 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/axfx | m1 | 3 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/base | m1 | 1 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/bte | m1 | 97 | 0 | 3 | 15 | 0 |
| src/RVL_SDK/db | m1 | 1 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/dsp | m1 | 3 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/dvd | m1 | 8 | 0 | 3 | 2 | 0 |
| src/RVL_SDK/esp | m1 | 1 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/euart | m1 | 1 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/exi | m1 | 3 | 0 | 0 | 0 | 1 |
| src/RVL_SDK/fs | m1 | 1 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/gd | m1 | 8 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/gx | m1 | 14 | 0 | 0 | 0 | 3 |
| src/RVL_SDK/ipc | m1 | 4 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/kpad | m1 | 3 | 0 | 0 | 0 | 4 |
| src/RVL_SDK/mem | m1 | 4 | 0 | 2 | 0 | 4 |
| src/RVL_SDK/mtx | m1 | 5 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/nand | m1 | 7 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/net | m1 | 4 | 0 | 0 | 0 | 1 |
| src/RVL_SDK/nwc24 | m1 | 19 | 0 | 0 | 1 | 13 |
| src/RVL_SDK/os | m1 | 35 | 0 | 12 | 1 | 19 |
| src/RVL_SDK/pad | m1 | 1 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/rso | m1 | 1 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/sc | m1 | 3 | 0 | 2 | 0 | 0 |
| src/RVL_SDK/si | m1 | 2 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/thp | m1 | 2 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/tpl | m1 | 1 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/usb | m1 | 1 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/vf | m1 | 49 | 0 | 1 | 5 | 1 |
| src/RVL_SDK/vi | m1 | 3 | 0 | 0 | 0 | 2 |
| src/RVL_SDK/wenc | m1 | 1 | 0 | 0 | 0 | 0 |
| src/RVL_SDK/wpad | m1 | 6 | 0 | 1 | 0 | 30 |
| src/RVL_SDK/wud | m1 | 3 | 0 | 6 | 0 | 0 |
| src/nw4r/db | m1 | 3 | 0 | 0 | 0 | 0 |
| src/nw4r/lyt | m1 | 15 | 0 | 24 | 0 | 13 |
| src/nw4r/math | m1 | 2 | 0 | 0 | 0 | 0 |
| src/nw4r/ut | m1 | 10 | 0 | 3 | 0 | 11 |

Delimiter warnings in **0 files**; full file hashes, warnings and all MEDIUM/HIGH sites are in the JSON output. Scope: src/Game, src/JSystem, src/nw4r, src/RVLFaceLib, src/RVL_SDK; includes .inc fragments. RVL_SDK is optional via --include-sdk (included here only if listed). Source hashes identify this scan despite concurrent lane edits.

## Full HIGH inventory

Each line is one site; reasons name the classified effects. Candidate rank is RNG, stream/iterator, mutation; class A before B within each rank.

- **A** `src/JSystem/JParticle/JPAFieldBlock.cpp:105` (m1; RNG:emtr->get_r_zh) — `mAccel.set< f32 >(emtr->get_r_zh(), emtr->get_r_zh(), emtr->get_r_zh())`
- **A** `src/JSystem/JParticle/JPAParticle.cpp:137` (m1; RNG:emtr->mRndm.get_rndm_zh) — `rnd(emtr->mRndm.get_rndm_zh(), emtr->mRndm.get_rndm_zh(), emtr->mRndm.get_rndm_zh())`
- **A** `src/JSystem/JParticle/JPAParticle.cpp:149` (m1; RNG:emtr->get_r_zp) — `base_vel(emtr->get_r_zp(), emtr->get_r_zp(), emtr->get_r_zp())`
- **B** `src/RVL_SDK/arc/arc.c:132` (m1; mutation:++) — `tolower((unsigned char)*path++) != tolower((unsigned char)*string++)`
- **B** `src/RVL_SDK/arc/arc.c:134` (m1; mutation:++) — `tolower(*path++) != tolower(*string++)`
- **B** `src/RVL_SDK/arc/arc.c:233` (m1; mutation:++) — `*dest++ = *src++`
- **B** `src/RVL_SDK/bte/bd.c:53` (m1; mutation:++) — `*a++ = *b++`
- **B** `src/RVL_SDK/bte/bd.c:62` (m1; mutation:++) — `*a++ != *b++`
- **B** `src/RVL_SDK/bte/btm_inq.c:1085` (m1; mutation:++) — `*p_n1++ = *bdn++`
- **B** `src/RVL_SDK/bte/hcicmds.c:321` (m1; mutation:++) — `*pp++ = *pin_code++`
- **B** `src/RVL_SDK/bte/hidh_conn.c:628` (m1; mutation:++, unknown/stateful:HID_BUILD_HDR) — `*p_out++ = HID_BUILD_HDR(trans_type, param)`
- **B** `src/RVL_SDK/bte/rfc_ts_frames.c:207` (m1; mutation:++, unknown/stateful:RFCOMM_I_CR) — `*p_data++ = RFCOMM_EA | RFCOMM_I_CR(is_command) | RFCOMM_MX_PN`
- **B** `src/RVL_SDK/bte/rfc_ts_frames.c:240` (m1; mutation:++, unknown/stateful:RFCOMM_I_CR) — `*p_data++ = RFCOMM_EA | RFCOMM_I_CR(is_command) | RFCOMM_MX_FCON`
- **B** `src/RVL_SDK/bte/rfc_ts_frames.c:259` (m1; mutation:++, unknown/stateful:RFCOMM_I_CR) — `*p_data++ = RFCOMM_EA | RFCOMM_I_CR(is_command) | RFCOMM_MX_FCOFF`
- **B** `src/RVL_SDK/bte/rfc_ts_frames.c:287` (m1; mutation:++, unknown/stateful:RFCOMM_I_CR) — `*p_data++ = RFCOMM_EA | RFCOMM_I_CR(is_command) | RFCOMM_MX_MSC`
- **B** `src/RVL_SDK/bte/rfc_ts_frames.c:319` (m1; mutation:++, unknown/stateful:RFCOMM_I_CR) — `*p_data++ = RFCOMM_EA | RFCOMM_I_CR(is_command) | RFCOMM_MX_RLS`
- **B** `src/RVL_SDK/bte/rfc_ts_frames.c:341` (m1; mutation:++, unknown/stateful:RFCOMM_I_CR) — `*p_data++ = RFCOMM_EA | RFCOMM_I_CR(FALSE) | RFCOMM_MX_NSC`
- **B** `src/RVL_SDK/bte/rfc_ts_frames.c:365` (m1; mutation:++, unknown/stateful:RFCOMM_I_CR) — `*p_data++ = RFCOMM_EA | RFCOMM_I_CR(is_command) | RFCOMM_MX_RPN`
- **B** `src/RVL_SDK/bte/rfc_ts_frames.c:409` (m1; mutation:--) — `*p_dest-- = *p_src--`
- **B** `src/RVL_SDK/bte/rfc_ts_frames.c:417` (m1; mutation:++, unknown/stateful:RFCOMM_I_CR) — `*p_data++ = RFCOMM_EA | RFCOMM_I_CR(is_command) | RFCOMM_MX_TEST`
- **B** `src/RVL_SDK/bte/xml_parse.c:573` (m1; mutation:++) — `*p_tmp++ = *p_sc++`
- **B** `src/RVL_SDK/dvd/dvdfs.c:50` (m1; mutation:++) — `tolower(*path++) != tolower(*str++)`
- **B** `src/RVL_SDK/dvd/dvdfs.c:206` (m1; mutation:++) — `*dest++ = *src++`
- **B** `src/RVL_SDK/nwc24/NWC24StdAPI.c:8` (m1; mutation:++) — `*pDst++ = *pSrc++`
- **B** `src/RVL_SDK/os/OSFont.c:274` (m1; mutation:++) — `d[q++] = s[r25++]`
- **B** `src/RVL_SDK/vf/pf_clib.c:20` (m1; mutation:++) — `*ld++ = *ls++`
- **B** `src/RVL_SDK/vf/pf_path.c:445` (m1; mutation:++) — `*pDirEntry++ = *short_name++`
- **B** `src/RVL_SDK/vf/pf_path.c:452` (m1; mutation:++) — `*pDirEntry++ = *short_name++`
- **B** `src/RVL_SDK/vf/pf_path.c:473` (m1; mutation:++) — `*pDirEntry++ = *short_name++`
- **B** `src/RVL_SDK/vf/pf_str.c:177` (m1; mutation:++, unknown/stateful:VFipf_toupper) — `*dest++ = (s8)VFipf_toupper(*p)`

## Regression cross-checks

The named-temporary fixes below contain no remaining expression with repeated RNG effects. This checks the targeted hazard, not every possible call/alias in each file.

- `src/Game/Enemy/KirairaChain.cpp`: **PASS**.
- `src/Game/Map/Flag.cpp`: **PASS**.
- `src/Game/Map/FishGroup.cpp`: **PASS**.
- `src/Game/MapObj/StarPiece.cpp`: **PASS**.
- `src/Game/Map/WaterPlant.cpp`: **PASS**.
- `src/Game/Util/MathUtil.cpp`: **PASS**.

Read-only MacioGalaxy JPABaseParticle::init_c cross-check: **PASS**. Model findings are excluded from the SMG2 totals above.

- **A HIGH** `/Users/roberthaisfield/MacioGalaxy/src/JSystem/JParticle/JPAParticle.cpp:135` — `rnd(emtr->mRndm.get_rndm_zh(), emtr->mRndm.get_rndm_zh(), emtr->mRndm.get_rndm_zh())`
- **A HIGH** `/Users/roberthaisfield/MacioGalaxy/src/JSystem/JParticle/JPAParticle.cpp:147` — `base_vel(emtr->get_r_zp(), emtr->get_r_zp(), emtr->get_r_zp())`

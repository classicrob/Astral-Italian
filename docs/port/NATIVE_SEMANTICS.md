# Native replacement semantics review

Analysis only, 2026-10-19. No clang/CMake/build/test runs; no game-source edits, patches applied, commits, or reference-repository mutations.

**Scope:** ht4 report and `ht4/overlay`; the gate was still running at the review cutoff, so this uses the requested ht4 fallback. Report SHA-256 `c240ba970db115f473de74bd9cb5005134cd6a74c77f6e9b69dbee73d7725671`. Pristine Petari is `e5bc761c0101b31ff82599de491bf29c3a1a3e13` (read with `git show`). The overlay is never inferred from the changing shared checkout.

**Result: 46 applied groups in differing SMG2 functions: 5 DIVERGES, 41 OK.** Two additional declaration divergences accompany them. `OK` means the SMG2-specific delta survives modulo the described native adaptation; it is not a runtime/resource-format proof. One auto-launch integration question remains in D.

The inventory examines all 956 applied (`clean`/`adapted`/`relocated`) groups, not just file-level `adapted` flags. Native/Wii projections select GALAXY_NATIVE and __MWERKS__ without invoking a compiler. It locates `#else` replacements and native-only returns, compares the **whole enclosing Wii-visible SMG2 function** to pristine Petari tokens, then manually compares the native payload against the current SMG2 branch. Force-match report labels are not trusted as function names. Existing SMG2 non-MW fallbacks are also retained in the analysis.

Lexical inventory: 289 groups matched replacement/return blocks; 46 have differing enclosing functions. The rest have unchanged functions, declaration-only changes, or new helpers with no Wii counterpart. The 76 unmatched broad textual candidates were checked: includes/macros, namespace/typedef guards and file-scope additions, not unreviewed function replacements. TVec namespace-locator false matches were resolved against JGeometry; return-by-value/template fixes retain the shared arithmetic. No RNG-order divergence was found among the scoped groups.

Evidence: [complete JSON inventory](native-semantics-ht4.json) (group text, function diff, both branch payloads, source hashes and excluded locators). Helper: [native_semantics.py](/Users/roberthaisfield/MacioGalaxy2-wt/port/tools/port/native_semantics.py). Reproduce without a compiler:

```sh
nice -n 10 python3 tools/port/native_semantics.py --report ~/MacioGalaxy2-team/scratch-port/ht4/report.json --root ~/MacioGalaxy2-team/scratch-port/ht4/overlay --reference ~/MacioGalaxy --output ~/MacioGalaxy2-team/scratch-port/solb/native-semantics-ht4.json
```

## Every scoped applied group

Locations below are ht4 overlay native-branch anchors. Group keys are the report-text SHA-256 prefix plus occurrence within the origin file; combine with the file path to identify a group. The three Mario.cpp return groups originated in src/Game/Player/MarioCollision.cpp. Each JKRMemArchive group below applies to the named overload; the lexical evidence also lists the sibling overload because their replacement payloads share lines.

| File:line | Function | Group key | Verdict | SMG2 difference and native comparison |
| --- | --- | --- | --- | --- |
| `include/Game/Screen/MessageTagSkipTagProcessor.hpp:23` | `MessageEditorMessageTag::getGroup / MessageEditorMessageTag::isGroupTagId` | `d2255129e686:1` | **DIVERGES** | SMG2 reads a full tag-group word instead of Petari's byte. Native getGroup still masks with 0xFF, so group 0x010A becomes 10 and isGroupTagId changes meaning. See correction A. |
| `src/Game/AudioLib/AudRemixMgr.cpp:20` | `AudRemixMgr::setRemixSeqResource` | `06f919c61c17:1` | **DIVERGES** | SMG2 names the timing/track/note members mNoteTimings, mTrackNo, mNotes. The whole native loader restores _C/_0/_4, which are absent. Endian reads and loop/stride logic otherwise agree. See B plus record declaration G. |
| `src/Game/Effect/ParticleResourceHolder.cpp:17` | `ParticleResourceHolder::ParticleResourceHolder` | `5c9e71367d02:1` | **OK** | SMG2 adds mountArchive(..., true) and removes mNumEffectNums/countAutoEffectNum. All remain outside the override. Only the JPA manager input changes to a converted host image. |
| `src/Game/Enemy/RingBeam.cpp:255` | `RingBeam::setRadius` | `8205db45ee59:1` | **DIVERGES** | SMG2 replaces _c0 with mBeamPos and adds variable mMaxRadius/color helpers. Shared radius/color logic survives, but native temp2(_c0) uses the absent Petari field. See C. |
| `src/Game/Map/CollisionParts.cpp:35` | `CollisionParts::init` | `7dbe3d8801e3:1` | **OK** | SMG2 uses getSensorHost(pHitSensor) and a zoneId local. Both remain in the shared initAfterServerData tail; the native split only adds the host-KCL overload. |
| `src/Game/Map/FileSelector.cpp:1009` | `FileSelector::exeFileSelect` | `da79b570af77:1` | **DIVERGES** | SMG2 selected-item state is mSelectedItem, not Petari _B4; the early-return path writes _B4. SMG2 also adds updatePointingGuidance, bypassed only on native openFile auto-launch. Fix the definite field loss in D; auto-launch/confirmation behavior still needs an SMG2 integration review. This file is already excluded in the in-progress gate. |
| `src/Game/Map/KCollision.cpp:38` | `KCollisionServer::setData` | `94969d33a71d:1` | **OK** | SMG2 represents unrelocated offsets through pointer fields/casts rather than offset-union aliases. Native builds a separate KCL descriptor from the same sections; no SMG2 gameplay change is discarded. Converter availability/version coverage is separate integration work. |
| `src/Game/MapObj/DynamicCollisionObj.cpp:157` | `DynamicCollisionObj::createCollision` | `0d1df188e4d1:1` | **OK** | SMG2 coallocates collision arrays, renames triangle count, and uses sensor Body (capital B). All survive outside the hook. The override only writes the equivalent little-endian octree root word. |
| `src/Game/NPC/TalkTextFormer.cpp:52` | `MR::initTagProcessorRecursive` | `6ac67ecb54b6:1` | **OK** | SMG2 language-rate helper (TalkTextFormer) / loop rewrite (LayoutUtil) remains shared. Native only returns for a missing pane; valid-pane behavior keeps the SMG2 code. |
| `src/Game/Player/Mario.cpp:3206` | `Mario::getSideWallNorm` | `d765a0675f72:1` | **OK** | SMG2 explicit mMovementStates.mLowWord bit test remains before the hook. Only the temporary zero-vector return becomes a stable reference; true-contact normals are untouched. |
| `src/Game/Player/Mario.cpp:3218` | `Mario::getFrontWallNorm` | `d765a0675f72:2` | **OK** | SMG2 explicit mMovementStates.mLowWord bit test remains before the hook. Only the temporary zero-vector return becomes a stable reference; true-contact normals are untouched. |
| `src/Game/Player/Mario.cpp:3230` | `Mario::getBackWallNorm` | `d765a0675f72:3` | **OK** | SMG2 explicit mMovementStates.mLowWord bit test remains before the hook. Only the temporary zero-vector return becomes a stable reference; true-contact normals are untouched. |
| `src/Game/RhythmLib/AudChordInfo.cpp:32` | `AudChordTable::setChordTableResource` | `fde6362b5aa6:1` | **OK** | SMG2 replaces pointer relocation casts with uintptr_t arithmetic. Native instead decodes the same CITS offsets into host tables, without relocating four-byte resource slots. No distinct SMG2 chord/scale rule was removed. |
| `src/Game/RhythmLib/AudMePlayer.cpp:387` | `AudMePlayingParamsHolder::setResource` | `c9e42e9aecf0:1` | **OK** | SMG2 rewrites resource/offset arithmetic to byte pointers and uintptr_t. Native follows the same entry/name offsets with BE reads and a host name-pointer table. |
| `src/Game/Scene/GameScene.cpp:373` | `GameScene::startStagePlayFirst` | `08971b87b326:1` | **OK** | SMG2 replaces Petari wipe/BGM/action setup with startWorldMapBgm and welcome/retry flow. Those shared arms survive. The native override only gates the camera condition with opt-in PETARI_FAST_BOOT. |
| `src/Game/Screen/LayoutManager.cpp:140` | `LayoutManager::LayoutManager` | `a7e0bc651e0b:1` | **OK** | SMG2 renames the recursive-animation-binding initializer. It remains shared. Native only extends the prefix-conversion buffers' lifetimes; archive-name conversion matches the Wii branch. |
| `src/Game/Screen/LayoutPaneCtrl.cpp:34` | `LayoutPaneCtrl::start` | `78bdb339d261:1` | **OK** | SMG2 mUseRecursiveAnimationBinding and BindAnimation(..., true, false) survive. The native early return checks missing pane/frame/animation before the shared SMG2 start path. |
| `src/Game/Screen/PaneEffectKeeper.cpp:47` | `PaneEffectKeeper::add` | `599e51125108:1` | **OK** | SMG2 compares isExistPaneMtxRef to false instead of nullptr. That check and createPaneMtxRef remain shared. Native aborts only if the resulting pane reference is missing. |
| `src/Game/Speaker/SpkTable.cpp:11` | `SpkTable::setResource` | `103dccb6a54a:1` | **OK** | SMG2 uses byte-pointer/uintptr_t relocation. Native decodes the same speaker entries/names into host pointers and sets the same initialized state; resource offset slots intentionally stay unmodified. |
| `src/Game/System/HeapMemoryWatcher.cpp:227` | `HeapMemoryWatcher::createRootHeap` | `5643952e9b1c:1` | **DIVERGES** | SMG2 reserves ARGetBaseAddress()+0xDB0000; GameSystem requests JKRAram::create(0xDB0000, ...). Native hardcodes Petari 0x4000+0xE00000, over-reserving 0x50000 and changing graph/GDDR heap partitioning. See E. |
| `src/Game/System/HeapMemoryWatcher.cpp:269` | `HeapMemoryWatcher::createHeaps` | `1b69c058edea:1` | **OK** | SMG2 changes audio heap size, system-heap ordering, and computes stationed sizes from remaining free space. These remain shared. Native changes only the WPAD object/header allowance using sizeof. |
| `src/Game/System/HeapMemoryWatcher.cpp:301` | `HeapMemoryWatcher::memoryErrorCallback` | `a536ed05ade6:1` | **OK** | The sole SMG2 callback-body delta is OSPanic source-line 537 to 617. Native preserves fatal allocation handling and adds diagnostics using __LINE__; adjacent newly added helpers are not replacements of SMG2 functions. |
| `src/Game/System/MainLoopFramework.cpp:356` | `MainLoopFramework::clearEfb` | `7487f802c770:1` | **OK** | SMG2 normalizes 0f/1f float spellings. Native changes the position format to F32 XYZ for the far-plane depth-clear adaptation; shared projection, viewport, color and render state stay SMG2. |
| `src/Game/System/MainLoopFramework.cpp:392` | `MainLoopFramework::clearEfb` | `11224f799b00:1` | **OK** | Same clearEfb float-spelling-only SMG2 delta. Native sends the first same-XY corner with explicit far-plane Z; it does not restore an old color/coordinate constant. |
| `src/Game/System/MainLoopFramework.cpp:399` | `MainLoopFramework::clearEfb` | `b48b1cb98eb6:1` | **OK** | Same clearEfb delta; second same-XY corner gains only native far-plane Z. |
| `src/Game/System/MainLoopFramework.cpp:406` | `MainLoopFramework::clearEfb` | `84f3f30ab210:1` | **OK** | Same clearEfb delta; third same-XY corner gains only native far-plane Z. |
| `src/Game/System/MainLoopFramework.cpp:413` | `MainLoopFramework::clearEfb` | `9439dca6b9a2:1` | **OK** | Same clearEfb delta; fourth same-XY corner gains only native far-plane Z. |
| `src/Game/System/MainLoopFramework.cpp:530` | `handleGXAbortAlarm` | `846d45ad2bd3:1` | **OK** | SMG2 inserts GXFlush before GXAbortFrame. It remains in the common abort path. Native can re-arm/return while the host GPU is making progress; that platform-specific deferral does not revert the SMG2 abort sequence. |
| `src/Game/System/MainLoopFramework.cpp:558` | `handleGXAbortAlarm` | `066c200c5cc4:1` | **OK** | SMG2 GXFlush remains shared. Native substitutes command-emission APIs for the same 0x61/0x5800000f words, after the shared abort sequence. |
| `src/Game/Util/DrawUtil.cpp:140` | `MR::clearZBuffer` | `a876c35b403f:1` | **OK** | SMG2 changes four zero GXColor assignments to Color8 tev0(0). That exact SMG2 zero color remains shared. Native changes only the depth-clear vertex format. |
| `src/Game/Util/DrawUtil.cpp:175` | `MR::clearZBuffer` | `8e9570541a32:1` | **OK** | SMG2 Color8 tev0(0) remains shared; native only adds far-plane Z to corner (0,0). |
| `src/Game/Util/DrawUtil.cpp:182` | `MR::clearZBuffer` | `b4565c15e9ec:1` | **OK** | SMG2 zero Color8 remains shared; native only adds far-plane Z to corner (width,0). |
| `src/Game/Util/DrawUtil.cpp:189` | `MR::clearZBuffer` | `fe7bc56c9a53:1` | **OK** | SMG2 zero Color8 remains shared; native only adds far-plane Z to corner (width,height). |
| `src/Game/Util/DrawUtil.cpp:196` | `MR::clearZBuffer` | `47c361d56cd4:1` | **OK** | SMG2 zero Color8 remains shared; native only adds far-plane Z to corner (0,height). |
| `src/Game/Util/LayoutUtil.cpp:62` | `setInfluencedAlphaToChild` | `6ac67ecb54b6:1` | **OK** | SMG2 language-rate helper (TalkTextFormer) / loop rewrite (LayoutUtil) remains shared. Native only returns for a missing pane; valid-pane behavior keeps the SMG2 code. |
| `src/Game/Util/LayoutUtil.cpp:112` | `getTextDrawRectRecursive` | `1f2ed6f2f781:1` | **OK** | SMG2 reverses the top/bottom rectangle comparisons. Both corrected comparisons remain shared; native only returns false for a missing pane. |
| `src/Game/Util/MapUtil.cpp:537` | `MR::calcMapGroundUpper` | `47eeebcd4d58:1` | **OK** | SMG2 expands vector temporaries and adds a const_cast; those expressions remain shared. Native checks the same strike-line result before using hit.mHitPos, avoiding an uninitialized-hit path. |
| `src/Game/Util/MathUtil.cpp:409` | `MR::checkHitSemilinePlane` | `b04f875c5a8c:1` | **OK** | SMG2 adds a pHitPos null guard. It remains shared. Native additionally rejects nonfinite/parallel dot products; the SMG2 nullable-output behavior is retained. |
| `src/Game/Util/MessageUtil.cpp:189` | `MR::countMessageChar` | `3a6a51cb11a4:1` | **OK** | SMG2 changes marker 0x1A to 0xE, removes group 11 and adds group 10 length handling. All stay shared. Only numeric-argument extraction changes to getParam32. Header group masking is a separate divergence (A). |
| `src/Game/Util/MessageUtil.cpp:199` | `MR::countMessageChar` | `9c25cef2f6d3:1` | **OK** | Same SMG2 marker/group-10 changes survive. Native changes only group-7 pointer extraction to a host handle accessor. Depends on the separate tag-group correction A. |
| `src/JSystem/J3DGraphBase/J3DShape.cpp:238` | `J3DShape::makeVtxArrayCmd` | `316b31fd2944:1` | **OK** | SMG2 spells the null raw-array address as 0 instead of nullptr. Native emits a host array command for the same stream/stride plus native buffer extent; no distinct SMG2 rendering logic is overwritten. |
| `src/JSystem/JAudio2/osdsp_task.cpp:28` | `__DSPHandler` | `5224957236a0:1` | **OK** | SMG2 renames lbl_806B75B9 to sPendingFrameMode. All uses remain in the shared DSP handler. Native removes hardware/context setup only. |
| `src/JSystem/JKernel/JKRMemArchive.cpp:126` | `JKRMemArchive::open(s32, EMountDirection)` | `1122f4b7ff8d:1` | **OK** | SMG2 uses s32 instead of long and widens resource arithmetic. The file-open path retains the shared signature and derives the same data start from a decoded RARC header/host tables. |
| `src/JSystem/JKernel/JKRMemArchive.cpp:153` | `JKRMemArchive::open(void*, u32, JKRMemBreakFlag)` | `c9d4c2c23b72:1` | **OK** | SMG2 uses u32 instead of unsigned long and widens resource arithmetic. The memory-open branch preserves breakFlag/_6C, heap selection, _5C and success return while decoding host tables. |
| `src/RVLFaceLib/RFL_System.c:54` | `RFLInitResAsync` | `e54217a018c5:1` | **OK** | SMG2 adds a null workBuffer check and returns WrongParam for null input. Both occur before the native validation return and are preserved; only damaged native resources get the new Fatal path. |
| `src/nw4r/lyt/lyt_texMap.cpp:78` | `TexMap::ReplaceImage` | `91b354e7f86f:1` | **OK** | SMG2 widens the descriptor relocation test to uintptr_t. Native avoids in-place 32-bit-pointer relocation, decoding the same TPL image/palette fields; no SMG2-specific image policy is lost. |

## Corrected native branches for DIVERGES

These are concrete proposed replacements, **not applied or compiler-validated**. Retain the original Wii branch unchanged. Companion declaration fixes F/G are necessary; changing call sites alone is insufficient.

### A — full SMG2 tag-group word

`include/Game/Screen/MessageTagSkipTagProcessor.hpp:23`, `d2255129e686:1`. Replace the getter/predicate part of the native branch; retain its host-parameter accessor declarations. SMG2 has a u16 group, not the low byte of Petari's packed word.

```cpp
s32 getGroup() const {
    return static_cast<u16>(mMessage[0]);
}
int getTag() const {
    return static_cast<u16>(mMessage[1]);
}
bool isGroupTagId(int group, int tag) const NO_INLINE {
    return getGroup() == group && getTag() == tag;
}
```

### B — SMG2 remix members plus native endian reads

`src/Game/AudioLib/AudRemixMgr.cpp:20`, `06f919c61c17:1`. This preserves the SMG2 timing pointer, track number and note-pointer assignments, with the transferred endian-read change. Apply declaration fix G with it. Timing words remain wire data as in the existing native design; any future timing consumer must decode them rather than assume host order.

```cpp
// Resource layout (big-endian u32 words): group count, one word per group, then per group:
// track count, note count, note count words, and per track one word followed by
// note count RemixNoteData records (4 words each).
void AudRemixMgr::setRemixSeqResource(void* ptr) {
    u32* newPtr = (u32*)ptr;
    int groupCount = PetariNative::readU32BE(newPtr);
    newPtr++;
    mGroupCount = groupCount;
    newPtr += mGroupCount;
    mGroups = new (mHeap, 0) RemixNoteGroupData[mGroupCount];
    for (int i = 0; i < mGroupCount; i++) {
        RemixNoteGroupData* pGroupData = &mGroups[i];
        pGroupData->mIndex = i;
        pGroupData->mTrackCount = PetariNative::readU32BE(newPtr++);
        pGroupData->mNoteCount = PetariNative::readU32BE(newPtr++);
        pGroupData->mNoteTimings = newPtr;
        newPtr += pGroupData->mNoteCount;
        pGroupData->mRemixTracks = new (mHeap, 0) RemixNoteTrackData[pGroupData->mTrackCount];
        for (int j = 0; j < pGroupData->mTrackCount; j++) {
            pGroupData->mRemixTracks[j].mTrackNo = PetariNative::readU32BE(newPtr++);
            pGroupData->mRemixTracks[j].mNotes = reinterpret_cast< RemixNoteData* >(newPtr);
            newPtr += pGroupData->mNoteCount * 4;
        }
    }
}
```

### C — seed RingBeam from the SMG2 beam position

`src/Game/Enemy/RingBeam.cpp:255`, `8205db45ee59:1`. The native safety initialization stays; its source must be the actual SMG2 member. All shared mMaxRadius/color/shadow helpers remain untouched.

```cpp
TVec3f temp2(mBeamPos);
```

### D — SMG2 selected-item state on native auto-launch

`src/Game/Map/FileSelector.cpp:1009`, `da79b570af77:1`. Replace the native insertion with the following (definite field correction). The normal path still executes the shared SMG2 updatePointingGuidance tail. **Unsure:** whether the opt-in early-launch path should bypass SMG2 confirmation/pointing initialization; this does not establish that behavior is correct. The in-progress gate excludes this file already; do not re-enable it merely for this repair.

```cpp
const s32 openFile = PetariNative::PlayerLaunch::openFile;
PetariNative::PlayerLaunch::openFile = 0;
const int openIndex = openFile > 0 ? ::getItemArrayIndex(openFile) : -1;
FileSelectItem* pOpen = openIndex >= 0 ? static_cast< FileSelectItem* >(mItems->getActor(openIndex)) : nullptr;
if (pOpen != nullptr && !pOpen->isNew() && !isUserFileCorrupted(openFile)) {
    FILE_SELECTOR_MILESTONE("FileSelector.OpenFile");
    mSelectedItem = pOpen;
    setNerve(GET_NERVE(FileSelector, FileSelectorNrvDemoStartWait));
    return;
}
```

### E — preserve the SMG2 ARAM partition

`src/Game/System/HeapMemoryWatcher.cpp:227`, `5643952e9b1c:1`. Replace the whole native branch, including its stale SMG1-size comment. SMG2 GameSystem.cpp:88 requests 0xDB0000 bytes; include ARGetBaseAddress exactly once. The wider address arithmetic remains native.

```cpp
uintptr_t arenaHi, arenaLo;
JKRExpHeap::createRoot(1, true);
arenaLo = reinterpret_cast<uintptr_t>(OSGetMEM2ArenaLo());
arenaHi = reinterpret_cast<uintptr_t>(OSGetMEM2ArenaHi());
newHi = reinterpret_cast<void*>(arenaLo + ARGetBaseAddress() + 0xDB0000);
OSSetMEM2ArenaHi(newHi);
JKRHeap::setAltAramStartAdr(arenaLo);
pHeap = JKRExpHeap::create(newHi, arenaHi - reinterpret_cast<uintptr_t>(newHi), JKRHeap::sRootHeap, true);
```

## Related declaration replacements (outside the function-only count)

### F — Mario's seventh effect flag must remain in the same word

**DIVERGES:** `include/Game/Player/MarioActor.hpp:807`, `9bed8fcfe814:1`, effect-flag struct (no enclosing function). SMG2 appends mSpecialSmoke after the six Petari flags. The transferred native branch uses 26 padding bits plus six flags, then the shared mSpecialSmoke starts a second 32-bit unit. This changes the union size and loses its bit-25 alias in mEffectFlagsWord. **Replace the existing conditional AND the following common mSpecialSmoke declaration** with this complete region, leaving the Wii sequence identical:

```cpp
#ifdef GALAXY_NATIVE
unsigned : 25;
unsigned mSpecialSmoke : 1;
unsigned mBeeWind : 1;
unsigned mIsStationary : 1;
unsigned mSlopeSlip : 1;
unsigned _2 : 1;
unsigned mSnow : 1;
unsigned mSmoke : 1;
#else
unsigned mSmoke : 1;
unsigned mSnow : 1;
unsigned _2 : 1;
unsigned mSlopeSlip : 1;
unsigned mIsStationary : 1;
unsigned mBeeWind : 1;
unsigned mSpecialSmoke : 1;
#endif
```

### G — endian-wrap the SMG2 note record, retaining its fields

**DIVERGES:** `include/Game/AudioLib/AudRemixMgr.hpp:11`, `88d7b39f3b2b:1`, RemixNoteData (no enclosing function). Native restores Petari _0/_4/_8/_C instead of mPitch/mVelocity/mLength/mStartTime. SMG2 AudRemixSequencer.cpp:38–39,54–58 reads the named fields. Replace only the native record definition:

```cpp
struct RemixNoteData {
    BigEndianValue<s32> mPitch;
    BigEndianValue<s32> mVelocity;
    BigEndianValue<s32> mLength;
    BigEndianValue<s32> mStartTime;
};
```

## Limits and next action

Prioritize F (native object layout), then A/B/C/E and G; keep D gated until its SMG2 launch flow is reviewed. No compiler pass or Wii object proof is claimed for the proposed corrections. The gate may remove more groups; rerun/filter using its final report before treating this as the landed inventory. Standalone native helper additions with no Wii counterpart and unconditional/neutral-only edits are outside this replacement-function question. No scoped RNG-order loss was found; that does not certify the separate additions or whole native runtime.

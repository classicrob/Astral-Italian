# SMG2 replay coverage triage

This is a **planned coverage matrix**, not a replay result or an approved fixture
list. The starting review used `65e6e8b36ced01ecd1bfbc9a883907c8fd3f7b29`.
No game was built or run for this document. Preserve the exclusions, validity
rules and implementation status in [HARNESS.md](HARNESS.md); this matrix does
not extend its contract.

A one-galaxy idle/jump tape can exercise ordinary player integration, collision,
gravity and whichever background actors actually update. It does not establish
that an initialized actor's interaction, resource variant, failure branch or
persistent state was exercised. **No first galaxy/scenario has been selected
here.** Even the synthetic idle/jump tape is a harness test, not a captured
Dolphin fixture or a guarantee about the game's KPAD read sequence.

## Evidence and fixture admission

The imported SMG1 names, dome/mission tables and unlocked-save claims are **not
SMG2 evidence**. The P2 catalog/save owners are replacing those imports in
[LEVEL_SELECT.md](LEVEL_SELECT.md) and [SAVES.md](SAVES.md). Do not use a remembered
SMG1 or SMG2 mission number to fill this table.

These SMG2 source facts are available:

- [ScenarioDataParser.cpp](../src/Game/System/ScenarioDataParser.cpp),
  `ScenarioData::initialize` and `getScenarioDataIter`, read
  `ScenarioData.bcsv`, `ZoneList.bcsv`, `GalaxyInfo.bcsv` and look up the **actual
  `ScenarioNo`**. [GalaxyStatusAccessor.cpp](../src/Game/System/GalaxyStatusAccessor.cpp)
  reads `Comet` and `CometLimitTimer`. A row index, star identifier and scenario
  number must not be silently interchanged.
- [GameSystemDataHolder.cpp](../src/Game/System/GameSystemDataHolder.cpp) loads
  `GalaxyDataTable.bcsv`, `GalaxyWorldOrderList.bcsv`, and the MarioFaceShip
  event/cast tables. These are runtime resource names, not embedded stage data.
- [GameSequenceFunction.cpp](../src/Game/System/GameSequenceFunction.cpp),
  `requestChangeStageMarioFaceShip`, routes to `MarioFaceShipGalaxy`, with a
  negative requested scenario replaced by **2**. Its world-map route formats
  `WorldMap%02dGalaxy` from the saved current world and requests **scenario 1**.
  These are source-defined routes, not proof that a chosen save/world/NPC is
  valid or that a hub visit was captured.
- [GhostAttackGhost.cpp](../src/Game/NPC/GhostAttackGhost.cpp), `init`, has an
  explicit `StarCreekGalaxy` / scenario **2** special case, and a `MagicGalaxy`
  case without a fixed scenario. This supplies a **source-branch candidate**,
  not actor-placement, unlock or race-completion evidence. It derives a
  `GhostAttackGhostData%02d.gst` resource using the mapped scenario.

**Metadata pending:** neither a verified SMG2 scenario/placement inventory nor
an SMG2 unlocked-save receipt was available to this review. Requests were sent
to the catalog/save owners and port lead. `PENDING` below means the exact
stage/scenario is intentionally unknown. It is not a launch instruction.

To admit a concrete replay target, obtain a private, derived receipt containing:

1. SB4E01 disc/DOL hashes, metadata exporter revision, stage id, actual
   `ScenarioNo`, `PowerStarId`, star/comet type, world and resource provenance.
2. Zone plus **active scenario layers**, placed object name/arguments, relevant
   switches and start point for the action. A catalog of scenario names alone
   cannot prove a power-up, guide, NPC or ghost exists in the loaded layers.
3. For progression-sensitive rows, `PENDING_GAME_VERIFICATION` until an
   `unlocked-save.json` receipt names the selected identities and passes the
   game's own save and loader semantic readback. No guessed star totals, event
   flags or raw-byte unlock patches; no personal NAND use.
4. Captured filtered KPAD read prefix/action tape, rate/epoch, initial NAND
   hash, oracle revision and a system-specific activation/completion witness.
   Loading the stage or observing the aggregate actor count is insufficient.

Do not commit/upload game assets, NAND/save contents, input logs or gameplay
traces with this document. Derived public-in-repo source references are the
only stage evidence recorded here.

## Modes and risks

- **C**: strict filtered-KPAD, headless CPU replay under the existing contract.
  The current v1 trace observes scene/stage/scenario, mode/state stack,
  animation, positions/velocities/gravity, flags, HP/death, RNG and actor counts.
  It does **not** expose every system below. Each row names missing witnesses;
  proposing one does not mean that observer or oracle mapping exists.
- **U**: separate interactive UI/render test with real layouts and visible
  results; **G**: separate GPU/render/readback test; **D**: separate physical
  device/raw-input test; **A**: separate audio/decode/timing/output test;
  **S**: isolated persistence/loader test. None is a headless v1 pass.

Risk codes are prioritization, not assertions of current defects. Both source
worklists below describe historical snapshots; a lexical hit, a transferred
hook, or an `OK` semantics row is not runtime validation or current fix status.

| Code | Specific worklist/semantics reference | Why the action matters |
| --- | --- | --- |
| P | [HAZARDS: triage/format handling](../docs/port/HAZARDS.md#triage-and-format-handling), items 1 and 5; `TalkMessageCtrl` mixed pointer/value union row | arm64 pointer width, callback/retained object lifetimes, aliased flag words; invoke callbacks and leave/re-enter states, not just allocate them. |
| R | [HAZARDS: on-disc records](../docs/port/HAZARDS.md#on-disc-record-inventory), BCSV/JMap, RARC, KCL, J3D and JPAC records | Big-endian fields, four-byte wire offsets versus host descriptors and format/version variants; exercise actual resource consumers and collision/animation paths. |
| M | [HAZARDS: triage/format handling](../docs/port/HAZARDS.md#triage-and-format-handling), item 2 (LMS/MSBT/MSBF), and [NATIVE_SEMANTICS A](../docs/port/NATIVE_SEMANTICS.md#corrected-native-branches-for-diverges), full SMG2 tag-group word | SMG2 messages/flow branches and UTF-16 tags differ from SMG1; entering one conversation does not cover every tag/choice or text rendering. |
| F | [NATIVE_SEMANTICS F](../docs/port/NATIVE_SEMANTICS.md#related-declaration-replacements-outside-the-function-only-count), seven Mario effect flags; [HAZARDS Player bitfield row](../docs/port/HAZARDS.md#bitfield-layout--review-21) | Packed word aliases and mode-specific effect flags. Check actual bits/transitions; a visually plausible model is insufficient. |
| Q | [HAZARDS endian raw accesses](../docs/port/HAZARDS.md#endian-raw-access--review-334), `ItemDrill.cpp` sine/cosine table accesses; R/P as above | Host table reads, collision transforms, FP grouping/precision and state transitions. Preserve native RNG evaluation order; no FP-domain waiver from a short replay. |
| V | [HAZARDS: on-disc records](../docs/port/HAZARDS.md#on-disc-record-inventory), BRLYT/BRLAN/BRFNT, J3D/BTI/ResTIMG; [HARNESS exclusions](HARNESS.md#what-is-still-nondeterministic-or-unqualified) | Layout/texture/animation CPU work is distinct from rasterization, EFB/depth/occlusion and FIFO timing. |
| I | [HAZARDS: on-disc records](../docs/port/HAZARDS.md#on-disc-record-inventory), JAudio sequences/streams; [NATIVE_SEMANTICS B/E/G](../docs/port/NATIVE_SEMANTICS.md#corrected-native-branches-for-diverges) and [declarations G](../docs/port/NATIVE_SEMANTICS.md#related-declaration-replacements-outside-the-function-only-count) | Remix note fields/endian, ARAM partition and stream headers/blocks; CPU pulls/discard do not establish audible output or DSP timing. |
| X | [HAZARDS: RFL records](../docs/port/HAZARDS.md#on-disc-record-inventory) and [high-confidence findings](../docs/port/HAZARDS.md#findings-high-confidence-first); `RFLiCharData`/database bitfields, `THPDec.c` pointer casts | Mii, movie and hardware-facing paths require their own decoded-data/UI/device observations, not a galaxy-selection proxy. |
| S | [HAZARDS endian raw accesses](../docs/port/HAZARDS.md#endian-raw-access--review-334), `GameDataSomeScenarioAccessor.cpp`; [NATIVE_SEMANTICS D](../docs/port/NATIVE_SEMANTICS.md#corrected-native-branches-for-diverges), FileSelector selected item/early launch | Real SMG2 progression, serialization widths/checksums, async completion and reload semantics; bypassed selector flow is not validated by reaching gameplay. |

## Gameplay system matrix

`No assurance` means an unspecified first galaxy idle/jump run provides no
meaningful evidence of the listed interaction. It can still load resources or
initialize actors. All pending stage memberships require the admission receipt
above; no row is currently an eligible replay fixture.

| ID / system | First idle+jump coverage | Actual stage/scenario evidence and action to select | Risk / source path | Needed observation, mode and remaining limit |
| --- | --- | --- | --- | --- |
| Y1 Yoshi mount/tongue | No assurance; spawning an egg/model is not mounting or tongue targeting. | **PENDING** active placement of `YoshiEgg` and a tongue target. Hatch/mount, point/lock, eat or pull a target, flutter, dismount/remount. | P/R/F; [YoshiEgg](../src/Game/Player/YoshiEgg.cpp), [YoshiTongue](../src/Game/Player/YoshiTongue.cpp), [YoshiLockOnManager](../src/Game/Player/YoshiLockOnManager.cpp). | C: mode/state/position plus mount, target identity/lock and eat/pull outcome witnesses needed; v1 alone lacks them. G/U for tongue/model/reticle appearance; D for physical pointing. |
| Y2 Yoshi fruit variants | No assurance; a normal mounted jump does not exercise each fruit. | **PENDING** active fruit objects/arguments for dash, balloon and light routes. Eat each, use its ability, exhaust timer and return to normal. | P/R/F; [YoshiFruitCtrl](../src/Game/Player/YoshiFruitCtrl.cpp), [dash](../src/Game/Player/MarioYoshiDash.cpp), [balloon](../src/Game/Player/MarioYoshiBalloon.cpp). | C plus fruit kind/timer/transition and collision/target witnesses. G for light/reveal output; headless behavior of visibility-dependent geometry must be separately qualified. |
| D1 Spin Drill | No assurance; merely carrying `ItemDrill` does not drill through a planet. | **PENDING** `ItemDrill` placement and drillable collision/exit layers. Pick up, dive, traverse, exit/reverse and collide/drop the drill. | Q/P/R; [ItemDrill](../src/Game/MapObj/ItemDrill.cpp), `exeDrillDiveLoop`, `exeDrillling`, `exeReverseDrillling`. | C: bind/drill state, entry/exit positions, collision face/normal and held-item state witnesses needed. G for drill-hole/decal output. |
| C1 Cloud | No assurance; loading `PowerUpCloud` is not creating a platform. | **PENDING** active `SpinCloudItem` and usable cloud route. Acquire, create/stand on clouds, use charges and reset/reacquire. | F/P/R; [SpinCloudItem](../src/Game/MapObj/SpinCloudItem.cpp), [SpinCloudBlock](../src/Game/MapObj/SpinCloudBlock.cpp), [CloudMarioHat](../src/Game/Player/CloudMarioHat.cpp). | C plus cloud charge/spawn/lifetime and standing/contact witnesses; v1 actor count is not cloud identity. G for cloud/effect visuals. |
| R1 Rock | No assurance; ordinary jumping does not enter rolling/breakable collision states. | **PENDING** `MorphItemRock` plus breakable obstacle/rolling route. Transform, start roll, strike obstacle, rebound/end and transform back. | F/P/Q/R; [MorphItemRock](../src/Game/MapObj/MorphItemRock.cpp), [RollingRock](../src/Game/Ride/RollingRock.cpp). | C plus rolling/obstacle-hit/break state; measure velocity and exit state. G for debris/model effects. |
| B1 Bee | No assurance; default movement does not cover air-walk gauge or cling. | **PENDING** `MorphItemNeoBee` plus valid cling surface and water/damage route. Acquire, fly to drain/refill gauge, cling, then lose form. | F/P/R; [MorphItemObjNeo](../src/Game/MapObj/MorphItemObjNeo.cpp), `enableArchives`, `doChangePlayer`. | C plus Bee gauge/cling/contact and mode-loss witnesses; G/U for gauge/model/effect rendering. No assumed SMG1 Honeyhive mission. |
| B2 Boo / Teresa | No assurance; model initialization does not test disappearance or barrier traversal. | **PENDING** `MorphItemNeoTeresa` plus valid passage/interaction. Acquire, disappear/reappear, traverse intended barrier and lose form. | F/P/R; [MorphItemObjNeo](../src/Game/MapObj/MorphItemObjNeo.cpp), [MarioTeresa](../src/Game/Player/MarioTeresa.cpp). | C plus disappearance/barrier/collision witnesses; G for alpha/BRK/visibility. CPU replay cannot certify transparency. |
| S1 Spring / Hopper | No assurance; a jump in ordinary mode is not spring movement. | **PENDING** `MorphItemNeoHopper` placement/route. Transform, compress/release, land repeatedly and exit form. | F/P/Q/R; [MorphItemObjNeo](../src/Game/MapObj/MorphItemObjNeo.cpp), `PowerUpHopper` branches. | C: mode, ordered state and trajectory plus spring compression/timing witness. G for deformation; no SMG1 Toy Time fixture inference. |
| F1 Fire | No assurance; idle/jump does not spawn/hit/expire fireballs. | **PENDING** `MorphItemNeoFire` and a fire-reactive target. Acquire, fire, hit/miss, expire projectiles and lose mode. | F/P/R; [FireMarioBall](../src/Game/Player/FireMarioBall.cpp), [MorphItemObjNeo](../src/Game/MapObj/MorphItemObjNeo.cpp). | C plus projectile identity/lifetime/hit-message witness; G/A for flame/impact output. |
| P1 Other modes / damage | No assurance for invincibility or other item modes; source support alone is not placed content. | **PENDING** derived active placements for each additional item/mode. Take damage, recover, expire timed mode; include Ice/Foo only if SMG2 catalog/placement confirms use. | F/P/Q; [PlayerUtil](../src/Game/Util/PlayerUtil.cpp), [MarioActor](../src/Game/Player/MarioActor.cpp). | C mode/HP/dead/state and item-specific witness. Do not equate the inherited enum/order or a registered resource with an SMG2 scenario. |
| T1 Co-star / player 2 | No assurance from a channel-0 tape or a following SupportTico alone. | **PENDING** scenario with targetable object; connect player 2, move pointer, interact/hold/spin or collect using supported co-star actions. | P/M/V; [SupportTico](../src/Game/NPC/SupportTico.cpp), [SupportTicoStateSpin](../src/Game/NPC/SupportTicoStateSpin.cpp). | C with captured reads for every channel/read ordinal plus co-star target/action witnesses. U for cursor/menus; D for real multi-controller/raw WPAD and connection timing. No channel padding or invented attach sequence. |
| G1 Water/gravity/2D | Only the actual spawn's gravity/collision; not swimming, gravity transitions or 2D constraints. | **PENDING** verified water, gravity boundary and 2D-area placements (may need distinct scenarios). Enter/exit water, cross gravity boundary, jump/land in 2D. | Q/P/R; [MarioSwim](../src/Game/Player/MarioSwim.cpp), [Mario](../src/Game/Player/Mario.cpp). | C trajectories/state/gravity plus water/area/contact witness. G for refraction/readbacks; an unsupported GX feedback panic invalidates the run. |
| R2 Rides / motion games | No assurance of mount/steering or motion-derived actions. | **PENDING** `GliBird`/`Tamakoro` or other ride placement; mount, steer/tilt, collide and finish/dismount. Separate routes per ride. | Q/P/R; [GliBird](../src/Game/Ride/GliBird.cpp), [Tamakoro](../src/Game/Ride/Tamakoro.cpp). | C downstream of captured filtered motion samples plus ride/checkpoint witness; D for physical sensor filtering/calibration. No MotionPlus fidelity claim. |
| E1 Enemies/bosses/dynamic map | An actor may update nearby, but no assurance of attack/damage/phase/switch coverage. | **PENDING** active encounter/obstacle and state/switch conditions. Attack, take damage, change phase, activate map switch and complete/fail encounter. | P/Q/R; [NameObjFactory](../src/Game/NameObj/NameObjFactory.cpp) confirms factory availability only; [RingBeam](../src/Game/Enemy/RingBeam.cpp) is semantics C's position-seed example. | C plus encounter phase, hit/switch/target identity witnesses. G/A for beam/effects/audio; one boss does not cover all enemy/resource variants. |
| L1 Death/checkpoint/retry | Not assured by a successful idle/jump. | **PENDING** verified hazard/checkpoint/start-id route. Die before/after checkpoint, retry and compare returned start/state. | P/S/R; [GameSequenceFunction](../src/Game/System/GameSequenceFunction.cpp), `requestChangeStageAfterMiss`. | C scene epochs/start/checkpoint and retry witnesses; S for any persistent side effects. No teleport or progress reset inside the compared tape. |

## Progression, hub and scenario systems

All progression-sensitive targets below remain `PENDING_GAME_VERIFICATION`
until the save-owner receipt is available. Route constants establish the source
request, not the complete set of valid or unlocked destinations.

| ID / system | First idle+jump coverage | Actual stage/scenario evidence and action to select | Risk / source path | Needed observation, mode and remaining limit |
| --- | --- | --- | --- | --- |
| W1 World map | No assurance; reaching a galaxy does not prove route unlock, node selection or return flow. | Source route is `WorldMap%02dGalaxy`, scenario **1**, saved current-world id. **PENDING** verified world/node and neighboring unlocked/locked destinations. Select a node/scenario, enter, clear/leave and return; exercise route-open result separately. | P/R/S/M/V; [GameSequenceFunction](../src/Game/System/GameSequenceFunction.cpp), [GalaxyWorldMapInfo](../src/Game/System/GalaxyWorldMapInfo.cpp). | C scene/scenario transitions plus world/node/route/result witnesses; U/G for pointing, nameplates/fog and selection. Generic `Game` player trace can have invalid player groups in world-map scenes; no fabricated zero player state. |
| H1 Starship / Lubba | Hub resource loads or a background NPC do not prove conversation/event behavior. | Source hub route `MarioFaceShipGalaxy`; ordinary negative scenario request becomes **2**. **PENDING** exact start/layer, event/cast and localized Lubba identity (inspect `Meister` cast). Talk/choose a branch, trigger a hub event, leave and return. Other hub scenarios must be separately cataloged. | M/P/S/V; [Meister](../src/Game/NPC/Meister.cpp), [MarioFaceShipEventDataHolder](../src/Game/System/MarioFaceShipEventDataHolder.cpp), [GameSequenceMarioFaceShip](../src/Game/System/GameSequenceMarioFaceShip.cpp). | C event/choice/sequence witness plus U text/choices/layout. Exact current save/event state required; merely entering hub scenario2 does not force every NPC/demo. |
| C2 Comet / Prankster | **Conditional only:** selected comet timers/background actors may tick while idle; no assurance of subtype, challenge, retry or award. | **PENDING** actual catalog `Comet`, `CometLimitTimer`, scenario/star mapping and active challenge objects. Start challenge, consume timer, trigger win/fail/retry; choose separate subtype/condition variants from actual data. | P/R/Q/S; [GalaxyStatusAccessor](../src/Game/System/GalaxyStatusAccessor.cpp), [CometEventKeeper](../src/Game/NPC/CometEventKeeper.cpp), [CometEventExecutorTimeLimit](../src/Game/NPC/CometEventExecutorTimeLimit.cpp). | C plus actual comet/subtype/timer/objective/award witnesses. U/G for counter/filter. Normal scenario id with a guessed comet flag is not a fixture. |
| C3 Cosmic Guide | No assurance; guide may remain hidden under progression/miss/switch predicates. | **PENDING** placed `SuperDreamer`/`Dreamer` start id, scenario layers and actual appearance predicates. Meet trigger through game behavior, accept help, run, cancel/finish and check result. | M/P/S/R; [SuperDreamer](../src/Game/NPC/SuperDreamer.cpp), [DreamerFunction](../src/Game/NPC/DreamerFunction.cpp). | Dedicated C plus U flow; **existing strict gate rejects built-in PadRecorder playback**, so automated guide playback needs a separately qualified test/observer, not disabling that rejection. The [GameScene start path](../src/Game/Scene/GameScene.cpp) calls `startReplayPad` in pad mode, and the [headless observer](app/headless/observe.cpp) rejects that reading state. Verify guide state, input source and award type; no guessed miss count. |
| G2 Ghost races | No assurance; instantiated ghost or decoded animation is not race playback/completion. | Source candidate `StarCreekGalaxy` scenario **2**; `MagicGalaxy` scenario **PENDING**. Both still need actual placement/unlock/GST proof. Start race, follow course, finish/lose/retry; include mapped green-star scenario case separately. | P/R/Q/S; [GhostAttackGhost](../src/Game/NPC/GhostAttackGhost.cpp), [GhostPacketReader](../src/Game/Player/GhostPacketReader.cpp), [GhostAttackAccessor](../src/Game/NPC/GhostAttackAccessor.cpp). | C plus packet index/time/ghost pose, start/finish and best-time witnesses. `checkAppearGhostLuigi` tests pad mode and progression flags; the same stage name alone does not activate the ghost. G for ghost model/footprints. |
| N1 NPC talk / shops / messages | Boot may load message resources; idle/jump need not traverse text/branch/shop paths. | **PENDING** active talk/shop placement and specific SMG2 message/flow ids. Talk, select both branches, spend/decline and re-enter conversation; include tagged/non-ASCII text. Hub route H1 is a candidate, not a substitute for message ids. | M/P/R/S; [TalkMessageCtrl](../src/Game/NPC/TalkMessageCtrl.cpp), [TalkTextFormer](../src/Game/NPC/TalkTextFormer.cpp). | C flow/choice/currency/event witnesses plus U font/UTF-16/tag/layout rendering. One string rendered successfully is not LMS/MSBF or all language coverage. |
| P2 Star collection / results | No assurance from an uncompleted route. | **PENDING** actual scenario/star id, hidden/green/comet/grand-star type. Collect, complete result sequence, return to hub/map and revisit/reload; distinct award variants need separate receipts. | P/S/M/R; [GameSequenceFunction](../src/Game/System/GameSequenceFunction.cpp), `reflectStageResult`, [GameDataHolder](../src/Game/System/GameDataHolder.cpp). | C award/route/scene witnesses plus S persistent semantic readback. No star-total or mission-number inference from SMG1. |
| S2 Save/load / file select | **Startup only, conditional:** fresh NAND may exercise create/header/error paths, not existing-file round trips, migration or restored progress. | Scene route `FileSelect` scenario **1** occurs in source; exact seeded selection/progress is **PENDING**. In a separate isolated test create/select a file, save an actual change, quit, reload, verify selected file and semantic progress; add supported corruption/version/error cases separately. | S/P/R/M; [SaveDataHandler](../src/Game/System/SaveDataHandler.cpp), [FileSelector](../src/Game/Map/FileSelector.cpp). | S plus U selection/confirmation. Current headless contract explicitly excludes existing-save validation and requires fresh user directory. Do not weaken that guard, overwrite personal NAND or reuse imported save publications. |

## Systems requiring more than galaxy selection

Choosing a stage may cause CPU initialization or resource decode, but cannot
validate a framebuffer, speaker, controller, persistent file or platform menu.
These separate tests must be named as such in any result; they cannot be
credited by a headless galaxy pass.

| ID / system | First idle+jump coverage | Actual stage/scenario evidence and action to select | Risk / source path | Needed observation, mode and remaining limit |
| --- | --- | --- | --- | --- |
| U1 HBM / Home menu | Explicitly excluded. Resource initialization is not opening, navigating or resuming the menu. | Any **verified pausable** scene, not an invented mission; open HBM, navigate, resume and exercise supported exit/reconnect choices. Scene eligibility/fixture **PENDING**. | P/M/V/X; [HomeButtonLayout](../src/Game/Screen/HomeButtonLayout.cpp), [HomeButtonMenuWrapper](../include/Game/System/HomeButtonMenuWrapper.hpp). | U/G/D/A as applicable: menu state, visible selection/text, pause/resume, controller and sound callbacks. F1 native Mods/Level Select is not automatically the Wii HBM oracle path. |
| U2 Mii | RFL/resource initialization or a default icon does not test selection, decode, persistence or face rendering. | Separate file-select/Mii flow with a private known Mii database/resource fixture; choose/change icon and reload. No galaxy/scenario is sufficient; exact fixture **PENDING**. | X/R/S/V; [MiiSelect](../src/Game/Screen/MiiSelect.cpp), [MiiDatabase](../src/Game/NPC/MiiDatabase.cpp), [ConfigDataMii](../src/Game/System/ConfigDataMii.cpp). | Resource/codec test + U/G/S: decoded attributes/face checksum, selected identity, rendered face/icon and persistence. Headless v1 exposes none of those; no RFL layout proof from successful boot. |
| V1 Movies / THP | **Conditional startup only:** a boot movie might decode; neither route nor decoding/output is measured here. | Select an actual SMG2 movie resource and its source-triggered demo/boot event after metadata verification. Decode, seek/stop/finish where supported, then transition. Resource/trigger **PENDING**, no guessed movie filename/stage. | X/R/I/V; [THPSimplePlayerWrapper](../include/Game/Screen/THPSimplePlayerWrapper.hpp), [THPDec](../src/RVL_SDK/thp/THPDec.c), [THPAudio](../src/RVL_SDK/thp/THPAudio.c). | Separate codec/audio-visual integration test: frame dimensions/index/checksums, audio samples, EOF and A/V sync. Null GX cannot validate video presentation; discarded audio cannot validate sound/sync. |
| A1 Audio streaming / DSP | **Conditional CPU only:** JAudio/DSP/AI remain active and samples are pulled/discarded; this does not prove a particular stream/block/loop ran. | **PENDING** actual SMG2 stage-BGM/sound-table row and stream resource. Start, cross block/loop boundaries, fade/change scene and stop; add concurrent SE/voice/stream use. No stage name alone proves streaming. | I/P/R; [JASAramStream](../src/JSystem/JAudio2/JASAramStream.cpp), [JAUStreamFileTable](../src/JSystem/JAudio2/JAUStreamFileTable.cpp). | C can witness requests/completions/AI budgets only if separately observed; A must check decode samples, scheduling/underrun, pitch/loop/fade, latency and audible/device output. Current scheduling is explicitly unqualified. |
| A2 Remix / rhythm | No assurance; a normal BGM track need not read SMG2 remix timing/note fields. | **PENDING** actual remix resource and stage/action selecting it. Activate/change tracks and consume timed notes across a transition. | I/Q/R; [AudRemixMgr](../src/Game/AudioLib/AudRemixMgr.cpp), [AudRemixSequencer](../src/Game/AudioLib/AudRemixSequencer.cpp). | CPU resource/sequence test plus A: group/track/note fields, note timing, emitted events and PCM. Semantics B/G records named SMG2 fields; compilation is not an endian/timing test. |
| V2 Render / EFB / FIFO | Explicitly excluded; CPU display-list emission and discarded draw/state calls can occur. | **PENDING** active reflective/depth/occlusion/material/particle scene paths. Render actual objects, force relevant readbacks, clipping and scene transitions; galaxy load alone insufficient. | V/R/F; [HARNESS null GX limitations](HARNESS.md#what-is-still-nondeterministic-or-unqualified), [HAZARDS records](../docs/port/HAZARDS.md#on-disc-record-inventory). | G: qualified image/depth/readback and command/FIFO/token timing tests. Headless unsupported readbacks panic; never replace them with invented zero or call a no-readback path a renderer pass. |
| D2 Physical input / extensions | Gate A injects **after** KPAD filtering. It does not validate raw WPAD/calibration or physical controller behavior. | Separate D/raw gate B tests for required controllers/extensions, attach/detach, calibration and motions; no galaxy can validate hardware. Real metadata/timestamps **PENDING**. | P/X; [HARNESS replay interchange](HARNESS.md#replay-interchange), [REPLAY_DIFF raw gate B](../docs/port/REPLAY_DIFF.md). | D with raw reports, commands and callback timestamps; compare filtered output separately. MotionPlus fields being retained is not service/hardware fidelity; unsupported Balance Board/unknown unions must remain rejected. |
| O1 Async I/O / scheduler | Only operations actually taken by the chosen boot/route; no broad determinism claim. | Verified load-heavy scene transition, resource load/unload, NAND operation and error/completion paths. Exact workload/fixture **PENDING**. | P/R/I/S; [HARNESS clock and audio](HARNESS.md#clock-retraces-and-audio), [unqualified timing](HARNESS.md#what-is-still-nondeterministic-or-unqualified). | Repeated C plus separate OS/DVD/NAND/DSP integration tests. Observe request/completion order and scene lifecycle; virtual frame time does not certify asynchronous completion or equal-priority scheduling. |
| O2 Network / WiiConnect24 | Explicitly excluded; linking NWC24 code or constructing a holder is not network/mail behavior. | Separate isolated platform/service test with explicit endpoints/fixtures; no galaxy/scenario suffices. **PENDING** supported service contract, no live external traffic authorized here. | P/R/X; [HARNESS exclusions](HARNESS.md), [HAZARDS directory ownership](../docs/port/HAZARDS.md#directory-ownership), Game/NWC24 and SDK rows. | Service/device integration mode, not C; validate serialization, callbacks and error behavior independently. No unsolicited sends or network equivalence claim. |

## Ranked next targets, after metadata qualification

This ordering is a risk/coverage recommendation, **not measured success**, a
first-fixture choice or permission to run a game/build. All admission and
harness-readiness gates still apply; defer a blocked target rather than guessing
its stage or modifying progression.

| Priority | Candidate IDs | Why it adds coverage / prerequisites |
| --- | --- | --- |
| 1 | H1/N1/W1, with S2 separately | Source-known hub/world-map routing opens SMG2-only message/flow, pointer/callback and progression paths absent from idle/jump. Need actual event/cast/world-node metadata, save receipt and UI test alongside CPU trace. Existing-save behavior stays a separate S test. |
| 2 | D1 and Y1 | Drill table/collision/transform paths and Yoshi tongue/target/mount lifetimes are distinct SMG2 interactions. Need placed targets/drill geometry plus system witnesses, not just mode changes/actor counts. |
| 3 | C1/R1 then Y2 | Cloud creation/destruction, rolling collision and fruit transitions add mode/state/object-lifetime diversity. Need typed placement arguments, collision outcomes and timed-mode witnesses; preserve FP evaluation/RNG ordering. |
| 4 | C2/L1/P2 | Timed challenge, death/checkpoint/retry and award/return exercise scene epochs, timer/failure branches and persistence. Need actual comet subtype/star mapping and legitimate initial progression. |
| 5 | G2/C3 | Ghost packet playback and guide input-source switching target serialization and automation paths. Need mapped GST/start ids/unlocks; guide PadRecorder path cannot use the present strict gate unchanged. |
| 6 | B1/B2/S1/F1/P1/G1/R2/T1 | Finish other transformations, environmental/ride states and co-star interactions with actual receipts. Stage overlap may reduce fixture count only when each activation/outcome is observed independently. |
| Parallel platform qualification | U1/U2/V1/A1/A2/V2/D2/O1/O2 | No number of galaxy CPU passes closes these gaps. Separate codec/UI/GPU/audio/input/persistence/service tests are required; prioritize the concrete HAZARDS/NATIVE_SEMANTICS paths used by admitted fixtures. |

## What a future result must report

Report per ID: `not selected`, `metadata verified`, `initialized only`,
`action observed`, `compared`, or `invalid/blocked`, with the fixture/hash,
observation fields and exact covered action/branch. `Compared` additionally
requires the existing replay validity/oracle gates; a new observer needs its own
qualification. Do not aggregate initialized-only actors into interaction
coverage, turn missing fields into zero, reuse an incomplete run, or infer
coverage of a neighboring scenario/subsystem. A CPU pass must keep its separate
UI/GPU/audio/device/save exclusions visible.

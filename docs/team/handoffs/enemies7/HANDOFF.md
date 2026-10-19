# Enemies7 → M1 successor handoff — 2026-10-19

## Stop state and first actions

Server Mac returned to user use; parent explicitly requested stop/move. Lead thr_ctftdsiexy and all three children are frozen. No child should be resumed on this host. Successor is Sol high on M1 with fresh Sols, maximum three, no Claude children. Read docs/team/TEAM.md and prompts/enemies6-final.md, then this file and the three STATUS files below. Kaomoji required at the beginning of user-facing messages; none in code.

Final game landing: 8df5a303 (full build/landing result recorded in land6.log). Landed checker baseline: 5c45a2ba. WIP branch is origin/wt/enemies-wip; only handoff/design snapshots remain WIP, no unimplemented Python modules or unlanded game-source changes. Exact branch SHA is recorded in handoff/WIP-SHA.txt after push (and parent final).

1. Fetch main and wt/enemies-wip on M1. Use main as code baseline; WIP docs may be cherry-picked or read with git show. Do not merge an obsolete WIP code tree over newer main.
2. Transfer ~/MacioGalaxy2-team/scratch-enemies/sol7 (122 MB at freeze), especially handoff/, helpers/, helpers-extended/, flags/, pool-peruse/, spindriver/, repeatgrid/, flow/. The WIP commit also contains a durable copy under docs/team/handoffs/enemies7/.
3. Preserve the shared benign/enemies.txt update: 12 stale checker-derived lines were removed, their newer manual proofs retained. Snapshot in handoff/benign-enemies.snapshot.txt; exact removals/demotions in helpers/global/demotion.json and failed.json. Reconcile with newer M1 shared list; do not blindly replace later edits.
4. Parent's newest priority is SOUND proof throughput, not more register-allocation polishing. Read ~/MacioGalaxy2-team/scratch-astra/PROOF-THROUGHPUT.md and coordinate with Astra. Three scoped modules/designs are described below; none is implemented yet.
5. Request/confirm current owner state before source edits. MorphItemObjNeo remains exclusive to enemies successor (objects confirmed), no enemies source edits/trials there yet.

## Current priority and crew handoff

Parent: thr_qzkz8x9vp8. Owners: Player Astra thr_u2fktcr3n2; objects thr_7x3er9hxaj; core thr_qwpehrs9y2; SDK/libs m1 thr_wd9dzywq5d. Use BB CLI, up to three gpt-6.1-sol high child threads; whole disjoint CPP units, root sole git writer/lander. Tool module ownership can be separated as below. Do not restart these frozen server threads; create fresh M1 children.

- Sol2 thr_jfjzdkthdc: helpers-extended/STATUS.md. Task A, new rdhelpers.py + test_rdhelpers_extended.py. Neither exists. Read-only ranked Player snapshots saved; API approved. Must solve PS0/PS1 boundary issue before claiming inline/outlining FP equivalence.
- Sol3 thr_teku6xmzum: flags/STATUS.md. Task B, new rdflags.py + test_rdflags.py. Neither exists. API approved; no implementation/tests/row exits.
- Sol1 thr_u3wy5g8vbr: pool-peruse/STATUS.md. Task C, rdpool.py + new test_rdpool_peruse.py. rdpool unchanged, new test absent. Candidate API remains UNAPPROVED; settle completeness/immutability protocol first.
- Root owns rdclass.py/rdchecked.py integration. Separate explicit opt-in --helpers, --flags, --pool-uses modes; unchanged baseline behavior. Land each independently with negative tests; after ANY tool patch revalidate ALL lanes' checker-derived benign rows, demote failures, notify owners. No certificates published by unfinished tasks.

## Proof-mode designs and actual blockers

Astra stable input: scratch-astra/rdscan401/{rows.txt,run.log}; 80 open tail rows, 1 positive/79 REAL: 33 pool guards, 5 helper ABI, 41 other. Full stable Player364 rows: trial/REALDIFF-astra400stable.md. Main unity source workers stay productive with Astra; no new handover until owner names it.

A. Helpers approved API: decode_helper(line, enabled=False) -> None/descriptor; lower_helpers(graph, enabled=False, decoder, runtime_root=None) -> (graph,evidence). Disabled must be exact no-op. Validate complete actual DOL helper bodies + canonical entries, exact affine r11/frame slots/register values, private-stack escape/alias rules, restored SP/LR/nonvolatiles and call barriers. GPR save/rest bases8062E130/8062E17C, FPR8062E098/8062E0E4, N14..31. FPR DOL validator not yet implemented. Scalar STFD reads PS0 and LFD restores PS0 only: must compare PS1 boundaries explicitly or reject affected allocation changes; no full-FPR restore fiction. Model BL LR side-specific; remove helper-only LR/address setup only when fully proved dead. No tail/dynamic-frame/unknown-helper support by default.

B. Flags approved API: decode_flag(line) -> ordinary use/def descriptor or None; FlagAlgebra(value).evaluate(d, uses) -> tuple interned results or None. Proposed deterministic bounded ROBDD exact32 bitvectors; opaque producer IDs are independent inputs, side-specific call clobbers never correlated. Start numeric li/lis/masks/OR/rotate/RLWIMI constant0/1, cntlzw/subfic, CR bit cror/crxor/creqv and Rc+XER.SO. Full32bit result, seed liveness, unmodified XER bits, CA/OV/SO, CR field updates, joins/call boundaries required. Replace pure instruction-count nodes only for explicitly modeled nontrapping operations with every defined result represented at checked live uses/boundaries. Preserve all memory/call/FP/FPSCR ordering; no CFG rewriting initially. Ranked cases MarioFaint start, MarioFoo close, MarioYoshiShot start/close; Foo start manual witness scratch-astra/sol-foo391/{start-certificate.json,start-leaf-seed-proof.json}.

C. Pool candidate UNAPPROVED API: leave PoolComparator.verify defaults unchanged; add per-use verifier/session recording all tainted actual uses/paths, exact full terminated string/width/record/index extent, DOL/source bytes and ADDR32 identities, complete closure with no unresolved use. Root guard must defer formation to actual use, not merely truncate a coarse target tail. Unknown dynamic index/return provenance/spill/escape/consumer/mutable storage reject. ELF .data alone does not prove immutability. Need a principled storage/consumer/extent/completeness protocol, not hand-waived ranges. All negative cases in pool-peruse/STATUS.md.

Astra post-baseline rerun407 on5c45a2ba: 0 new positives, all5 past old helper reject. New exact blockers: CloudMarioHat initDynamics stwx; MarioScreenStrip BB50 lwzx; MarioHeadCtrl AB50 subic.; MarioHeadCtrl AA70 lfsx; YoshiModelTargetState calc helper-frame address escapes to call. Exact pins/results scratch-astra/rdscan407/{rows.txt,REPORT.md,run.log}. No broad opcode acceptance requested; indexed address/width/alias, CR/XER and frame privacy must be proved. Prior four objects benchmark rows similarly advance to bctrl/xoris/cyclicCFG, still0 certificates. Prioritize a small complete mode with actual row exits over a big unvalidated framework.

## Landings and measured gains

All landed batches passed full ninja/original DOL hash and all-unit nonregression guard. Per-batch report.json matched-code deltas (not fuzzy scores):

| SHA | Change | Gained code bytes |
|---|---|---:|
| f5d51b3a | Typed sound pointer API; canonical const JAS accessors; four dummy consumers removed | 104 |
| fb1ddb00 | Full-width resource-base arithmetic in audio/Speaker | 0 |
| bfa04b0d | Real292B weak rotate ownership;14 canonical AudSystem records; Tongue ordered FP | 292 |
| 03f0178e | MarinePlant bounds arithmetic; Swim call/FP order; duplicate Flow doFlow removal | 88 |
| 5c45a2ba | Exact bounded integer compiler-helper checker;180tests | 0 |
| 8df5a303 | Flow table order/readable names; RepeatGrid native conversion; Spin data/name ownership | 0 |
| Total | | 484 |

SpinDriver gains208 strict SG bytes from two newly exact relocation identities; the report already counted their masked instruction bodies, so batch6 report gain is0. Do not count208 twice.

Report caches: ~/MacioGalaxy2-team/main-report/<fullSHA>.json; logs land1.log .. land6.log in this directory. f5d51b3a commit title had an incorrect invented aggregate72/77: ignore it; exact unit SG counts below and land1-counts/summary.json supersede it. Report matched_code is relocation-masked and differs from SG strict; do not conflate them.

## Landed units, strict SG matched/total

| Unit | Strict |
|---|---:|
| ActorActionSoundKeeper | 12/12 |
| ActionSound | 15/17 |
| SyncBckEffectInfo | 4/5 |
| MultiEmitter | 38/44 |
| AnimStampController | 11/11 |
| KCollision | 17/31 |
| SpkSystem | 16/17 |
| AudInstListMgr | 35/40 |
| JASBasicInst | 14/15 |
| JASDrumSet | 10/12 |
| AudChordInfo | 40/40 |
| AudMePlayer | 21/30 |
| SpkTable | 2/2 |
| SpkWave | 7/7 |
| BegomanBase | 43/75 |
| AudSystem | 93/120 |
| YoshiTongue | 39/57 |
| MarinePlant | 15/26 |
| MarioSwim | 57/189 |
| RepeatBlockGrid | 22/38 |
| MarioFlow | 0/4 (masked2/4 ctor+close) |
| SpinDriver | 39/61 (was37/61; +208B) |

YoshiTongueAction was inspected unchanged86/121, Yoshi unchanged137/231, WoodLogBridge unchanged9/22; these were not source changes landed by this session. No blanket matching claims for data.

## Correctness work and evidence

- sound API: ActorActionSoundKeeper's actor action animation pointer now passes as ActorAnimKeeper*, not s32. Canonical ActionSound::createSyncBckParam and SyncBckEffectInfo ctor/addBck declarations, callers and mangled symbols migrated atomically. MultiEmitter passes nullptr. No layout change.
- m1-approved canonical JASBasicInst const getKeyRegion and TPerc const getVeloRegion maps corrected; mutable/const pairs no longer falsely local. Missing16B TPerc const definition recovered; AudInstListMgr selects const views. Header fixes at source, no local ABI declarations.
- Weak cleanup: four fake consumers removed. Real template NO_INLINE constructor reached from Anim mDirection; KCollision canonical deferred ==,/ source-owned weak bodies; Spk generic JSUList destructor explicit instantiation. All four required actual-DOL weak bodies exact, survivors unchanged. Evidence weak/{REPORT.md,preservation.json,weak-target.json};54header-user compiles. Approved default-off TVec/JSUList guards preserve defaults/layout.
- Begoman rotate: genuine canonical generic body+explicit float instantiation via separate default-off rotate guard, no fake caller.292B target80162700, actual sole callee exact;106existing functions/data/refs preserved. Split ends80162830 (12B padding).52header users +native O2/sanitizers. Evidence weak/rotate/.
- Audio/Speaker pointer narrowing removed with uintptr_t or typed byte bases; all original Wii instruction bodies/refs preserved, native high-address probes pass. This is address-width repair ONLY: Wii4B serialized name/pointer slots vs native8B storage/endian conversion still needs native loader work. AudScaleData API uses uintptr_t (same Wii Ul mangling). Evidence weak/speaker-pointer*, audmeplayer/, native chord probe in scratch.
- AudSystem14 category-record definitions exact2B actual-DOL payloads, canonical names/size2 maps and odd-label removal atomic. CW folds same-TU consts to immediates and gives4B source spacing vs2B target: explicitly nonmatching, no blanket data equality. Evidence audsystem-records/.
- Tongue: natural factor*savedProgress operand order and separate prefix(distance+oldTotal) vs total(oldTotal+distance), one trial each.39strict/49masked unchanged, only2bodies changed. Still FRSP/scheduling residue, no forcing. tongue/FINAL.md.
- Swim: close now getPlayer before virtual getGravityVec; FD5E0 both verticalSpeed+waterDepth additions match target order.57strict/122masked and all33prior accepted bodies retained. swim/FINAL.md.
- MarinePlant bounds: extent local followed by *=0.5f retains target FP operand order; rdclass a proof. Axis/Spread individual b candidates, no blanket waiver. marine/proposed-benign.txt and source/object/DOL witnesses. Owner told SHA03f0178e, released back.
- Flow duplicate Mario::doFlow removed (canonical identical Mario.cpp remains), aligning close's string pool; +88reportB. Later start selects active const table before getFrontVec. Own fields _12/_14/_18 -> mFrameCounter/mPhase/mVelocity, same u16/u16/TVec3 layout, Airborne0/Landed1, existing increment forms. Astra approved.13emitted raw bodies/allALLOC bytes/effective refs identical after readability rename; only generated labels/debug names differ. flow/{HELPER-FINAL.md,readable-preservation.json}.
- RepeatGrid calcCell float->s32->s16 follows target FCTIWZ32/low16 and fixes native finite-signed32 values outside16range UB.22anchors/data retained,51/52body/ref equality.14endpoints*3axes O2+sanitizers pass; original negative control fails. NaN/inf/out-of32range not solved. repeatgrid/REPORT.md.
- SpinDriver source/header unchanged. Canonical mutable StartFrame807CFDF8=10 and EndFrameFC=50 plus full928B.data806AE390..806AE730 verified.44ADDR32/41complete strings/12B PMF exact;688Bprefix was generated-auto tail, no source owner. Split widen+same-address rename add208strictB.89survivor bodies/data/refs same. Two bounded manual row proofs for matrix3disjointstores and startBind extra-copy/setup; default rdclass still0/2. spindriver/{REPORT.md,individual-proofs.md,pool-witness.json,matrix-witness.json,startbind-witness.json}.

## Checker baseline landed — do not overclaim

5c45a2ba changes rdchecked.py + test_rdhelpers.py only: exact _savegpr_N/_restgpr_N, N14..31, full actual DOL/mapping pins, fixed private frame/r11/entry-register save+restore proof; expands true memory operations and side-specific BL LR. Ordinary call effects/FP/clobbers/alias barriers intact. No FPR, tail/dynamic frame, unknown names, mismatched inline restore support.180tests including21new pass independently on root. Four supplied objects rows remain0certificates; helper reject advances to bctrl/xoris/cyclicCFG. helpers/REPORT.md.

ALL current140derived benign rows revalidated:134after-sync +6Astra rebound supplement, all pass. Root removed12stale enemies derived lines (pool-tail reach guards), preserving their newer manual b proofs. No other lane lines removed. helpers/global/{root-final-validation.json,latest-manifest.json,after-sync.log,supplemental.log,demotion.json}. New extensions still need fresh all-lane revalidation, even though baseline passed. Do not mutate current-main proof lists during future binding prep; stage privately then update after actual landed fingerprints.

## Ownership / handovers at move

- Player Yoshi, Tongue/Action, Swim returned to Astra with SHAs; no active enemies writes. Flow release to Astra after8df5a303. Approved Flow names landed atomically. Prior19Animator proofs transferred by Astra into its own benign list; do not redo.
- MapObj MarinePlant and WoodLogBridge returned to objects. RepeatBlockGrid and SpinDriver release with8df5a303; header RepeatBlock.hpp was READONLY (objectsB owns RepeatBlock.cpp).
- MorphItemObjNeo.cpp+ownheader EXCLUSIVE enemies successor: objects confirmed no edits there. Four remainingREAL glowEffect/init/exeSwitchAppear/tryGetItem. Fresh54functions9388B/99.1%fuzzy baseline saved morph-initial.sg.txt; zero trials/writes. Do NOT redo owner's calcBase finite-u16 multiply+reload or enableResource7strings proofs. Objects already landed8nerve CPP definitions/declarations and Collection8B startup correction; source data/model-name10pointer/effectpool closure remains needed, no confirmed wrong values. Coordinate header changes with objects.
- All three proof-mode workers stopped; no fresh code files. Parent will start M1 successor/fresh Sols.

## NONMATCHING and genuine remaining issues

Detailed function list for first10units: land1-counts/summary.json; per-unit current remaining lists/reasons in reports above. No new full-matching claims.

- KCollision14rows scalar-pool relocation operands; SpkSystem sinit guard/registration identity; target weak bodies independently exact. JAS constructors/accessor-related residuals and audio loading rows documented unchanged.
- ActionSound rate/pitch-volume args, SyncBck delete-frame helper, MultiEmitter constructors/sync/rotation: existing allocation/pool/scheduling residue retained, exact symbol list in land1 summary.
- AudSystem27unmatched: category records fold/spacing, singleton construction order remains explicit (no blanket teardown-order waiver), AudAudience/JAIStreamAram wrappers, etc. AudSystem BGM enum placeholder still exists at seFanfareProcess: header table is SMG1, SMG2 names unrecovered.
- AudRhythmHolder getPlayTimeSec/getPrev mixed-precision target fdivs consumes unrounded __cvt_ull_dbl; source fdiv+frsp. ~12natural forms exhausted; no speculative pragma/builtin/asm/alias. Sole tracked own precision placeholder remains.7/9strict.
- CameraRotChecker extra frsp: no new natural proposal; f64 attempt already known changes products. Do not repeat.
- Begoman32oldunmatched unchanged after recoveringrotate; Tongue FP/FRSP/scheduling; Swim27rows pool/indexed tables/bitfields/cachedloads/vector stack/FPR/save; Flow flags/save/stack/registers; RepeatGrid7rows xoris/cyclicCFG/BSS-copy/helper/arithmetic; Spin2bounded witnesses not yet automatic. No register polish.
- SimpleEffectObj getClippingCenterOffset returns pointer to local; actual DOL does too. Three overrides return address of temporary. Not an ABI mismatch. Native port needs coordinated by-value virtual API migration, not a target matching shim. Proposal native-lifetime.md, parent notified. No source edit.
- Native resource wire layout remains separate pointer-slot/endian-loader task, explicitly not solved by uintptr_t bases.
- XanimeCore reciprocal already fixed main metadata globalweak8006C300 vs local80045970; target now matches, no source patch. NWC24 validator placeholder removed by m1; verified. Scoped-local datacontent fix af19adaa landed, use current auditor and verify actual DOL identity.

## Useful tricks / process

Read NOTES/HEADERS newest entries. Natural accumulator compounds can preserve target FP operand order when source operand swaps do not. uintptr_t preserves Wii unsigned-long mangling; typed byte locals sometimes change MWCC inlining, minimal uintptr_t expression spelling can retain exact calls. Canonical declaration-only template guard+real generic explicit instantiation emits weak functions without fake consumers. No anonymous union field aliases or pointer in u32/s32.

Only root commits/lands. safe_land_sol.py is safe_land.py with the obsolete Claude coauthor trailer removed; same FIFO/fullhash/all-unit guard/sync behavior. Snapshot occurs AFTER acquiring lock, so EXCLUDE every active file basename including future tasks; previous root once unintentionally included Flow duplicate cleanup while queued, documented as part03f0178e. Do not repeat. All source snapshots frozen before final8df landing. New branch work should rebase/fetch current main first. Disk123GiB free at stop; user floor40GB. Quota rule: no last banked reset; switch to Claude only after Codex exhausted per parent/user, no Claude children now.

Final placeholders run: tracked12 (core11/enemies1); extern-C0; #if1 pins3 upstream. The forcing-candidate inventory is advisory and reports stale/source-only objects across many dirs, so its raw candidate totals are not a claim of real dummy consumers. See placeholders-final.txt; do not undo the four independently proved dummy removals.

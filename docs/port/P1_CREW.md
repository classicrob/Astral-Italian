# P1 native compile+link crew brief (Sols B, D, E, F; libraries: m1 lane) - PORT is the team's top priority
Goal: make all SMG2 game + library code compile natively (clang arm64, GALAXY_NATIVE=1) and link, so the headless logic harness
(Sol A) can run one galaxy. Measure: docs/port/COMPILE_STATUS.md audit (1,693/2,045 TUs after hooks; Sol B's 12 patches took a
scratch copy to 1,904) and the link preview docs/port/LINK_PREVIEW.md (2,965 undefined symbols, mostly
game providers that do not compile yet: FootPrint.cpp (NameObj::startMovement/endMovement, 873 refs), LiveActorUtil.cpp,
ActorAnimUtil.cpp ...).
## Partition (disjoint files; never edit outside yours; a fix you need elsewhere -> note it in your HANDOFF for that owner)
- Sol B: core: src/Game/{Util,LiveActor,NameObj,System,Scene,Gravity}/** and include/Game/{same}/**, include/*.hpp
- m1 lane (thr_smqqn642y7, its own Sols): libraries: libs/** and src/{JSystem,nw4r,RVL_SDK,RVLFaceLib,libms,MSL_C,Runtime}/**
- Sol D (M1): src/Game and include/Game {MapObj, Ride, AreaObj, Boss}
- Sol E (M1): src/Game and include/Game {Enemy, RhythmLib, Screen, NPC}
- Sol F (server): src/Game and include/Game {Map, Camera, Animation, Effect, Demo, AudioLib, GameAudio, Speaker, NWC24, Player}.
  Player files under astra's exclusive handover: leave Mario.cpp, MarioActor.cpp and the Mario*.inc unity parts for last and
  list them; the lead asks astra.
- Native-semantics corrections (hooks that were NOT transferred because their native branch carried Petari logic; ready-made fixes
  in docs/port/NATIVE_SEMANTICS.md, sections A-G): owners port them
  with the rest of the file's hooks: MessageTagSkipTagProcessor.hpp (E), RingBeam.cpp (E), AudRemixMgr.cpp/.hpp (F),
  FileSelector.cpp (F), MarioActor.hpp (F), HeapMemoryWatcher.cpp (B).
- Compile-gate exclusions (hooks that broke MWCC: missing definitions/includes) are listed as `excluded` in docs/port/HOOK_TRANSFER.md;
  the owner of each file ports its hooks by hand, adding the missing include or definition.
- Pruning (user rule): after each batch delete your intermediates (scratch trees you re-synced, audit JSONs you no longer need,
  diff dumps); keep final reports, current handoff, accepted evidence (one .tgz).

## Workflow
1. Base: the lead names a base commit. `rsync -a --delete ~/MacioGalaxy2-wt/port/{src,include,libs,native} <you>/tree/` (plus
   tools/ for the audit), then `cd <you>/tree && git init -q && git add -A && git commit -qm base` (a private scratch repo outside
   the team repo). Re-sync only when the lead says so.
2. Fix in your tree. EVERY change must leave the Wii build byte-identical: put native code in `#ifdef GALAXY_NATIVE ... #else <original
   untouched> #endif`, or make a readable Wii-neutral rewrite that expands to identical Wii tokens (team-lead rule: prefer a GALAXY_NATIVE
   block over casts/macros that only placate clang; no reformatting, no renames, no shims: no extern "C" stubs, no no-op bodies, no
   `#if 1`). Port MacioGalaxy's solution where it has one: ~/MacioGalaxy (READ-ONLY), its pristine base is e5bc761c0; the unapplied hooks
   are in docs/port/hooks-manual/<path>.diff and triaged in docs/port/HOOKS_MANUAL.md (port-clang/port-native rows are yours).
   SMG2-specific code MacioGalaxy never had: write the native fix yourself (64-bit: pointer-width types; endian: JSU_BE(T) for disc
   records as MacioGalaxy does).
3. Check: `nice -n 10 python3 native/tools/audit_compile.py --root <you>/tree --jobs 4 --output <you>/audit.json <paths>` on the TUs you
   touched and their dependents; no regressions.
4. Deliver in batches of ~30-60 min of work: `git -C <you>/tree diff > <you>/patches/NN-<topic>.diff` plus a line in
   <you>/patches/README.md (files, what, TUs unblocked). Commit in your scratch repo after each batch so the next diff is incremental.
   The lead applies, runs the Wii compile gate + a clean-build object proof, and lands. Tell the lead when a batch is ready
   (your final message, or keep going and list ready batches in <you>/HANDOFF.md).
5. Heavy builds (any cmake/ninja/clang batch over ~50 TUs) through the shared lock: `lockf -k ~/MacioGalaxy2-team/slots/native.lock
   nice -n 10 <cmd>`; -j 4 or less. Small single-file clang checks need no lock.
6. Codex runs out ~22:00: keep <you>/HANDOFF.md current (done / in progress / next), so a Claude successor can continue.
Rules: ~/MacioGalaxy2-team/scratch-port/COMMON.md (no commits to the team repo, no edits in the worktree, never contact upstream).

## Sols on the M1 (own worktree there)
- Your tree is your own M1 worktree: `git fetch origin && git rebase origin/main` once the lead posts the base commit (the re-landed
  hook transfer). Work on a branch `port-p1-<you>`; commit each batch (message `native P1 (<partition>): <what>`), then
  `git push origin port-p1-<you>` and tell the lead the commit range. The lead cherry-picks, gates, proves and lands; do not land yourself.
- ~/MacioGalaxy (the SMG1 reference) is on the server only. Use docs/port/hooks-manual/<path>.diff (every MacioGalaxy hook the
  transfer could not apply, with Petari line/function) and docs/port/HOOKS_MANUAL.md; ask the lead for any other reference file.
- The native audit runs on the M1 too (native/tools/audit_compile.py, Apple clang); M1 builds may use -j 6 without the server lock.

# Native gameplay playtest — 2026-09-29

These are full-app tests on the extracted RMGE01 disc, using the public native
input layer, game UI targeting, and read-only game-state observations. They do
not teleport Mario or change the game's state to advance the test. Desktop
computer control was unavailable for app27/app28, so those runs were not visual
inspection. The subsequent rendering investigation below uses desktop captures.

## Existing save: app27

`PETARI_SMOKE=reload PETARI_SMOKE_FRAMES=36000`, isolated user directory
`build/playtest-reload-27`, log `build/native-boot-27.log`.

- Loaded a previously created Mario slot without creating another file.
- Advanced the five prologue pages and Peach's letter with normal A presses.
- Idle: no displacement in 120 frames, on the ground.
- Jump: rose 203.5 units and landed after 36 frames.
- Up/down: moved about 373 and 362 units; direction cosine -0.999365.
- Pause opened on a Plus hold. Movement input caused zero displacement while paused.
- Resume closed the menu; movement then covered 360.48 units.
- PASS at frame 4699; normal power-off returned exit status 0.
- SHA-256 hashes of GameData.bin and banner.bin matched the pre-run copies.

## Fresh save: app28

`PETARI_SMOKE=gameplay PETARI_SMOKE_FRAMES=36000`, isolated user directory
`build/playtest-fresh-28`, log `build/native-boot-28.log`.

Created a Mario file, completed both saves, advanced all five prologue pages and
the letter, then passed idle, jump/landing, opposite movement, pause/freeze, and
resume/movement checks. Jump height was 203.5 units; movement after resuming was
360.38 units. PASS at frame 5411, followed by normal power-off and exit status 0.

Component verification: `native_app_smoke`, `native_app_seam`, and `native_input`
pass in the root build. The 137 smoke checks and 27 seam checks also pass
under ASan/UBSan.

## Bug reproduced and fixed

App26 performed the fresh-file path, idle, jump and opposite movement checks,
then failed to pause. The default Escape binding sent Plus and B together. The
game's pause checker requires Plus held for 12 frames while A/B are up, so B
prevented pausing. Its sampled state confirmed Plus held, B held, operating yes.

Escape now sends only Plus; Backspace sends B for backing out of menus. Hold
Escape briefly to open pause, then tap it to close. The real KPAD input regression
checks that Escape keeps A/B up throughout a hold and that Backspace sends B
without Plus. App27 verified the fix in the game.

## Limits

The opening and these controls are covered. Later stages, camera controls, spin,
motion-controlled activities, movie presentation, and overall visual fidelity
have not been established by these runs. The next route toward the Bowser attack
has been researched from stage placement data but has not been playtested.

## Rendering and device-audio investigation

The manual playtest exposed defects that the input/state smoke tests could not
see: a solid-blue title background, a white/pink logo reaction, missing animated
text, and irregular sound.

- **Depth clears:** Aurora's `GXSetZTexture` is unimplemented. The game's clear
  quads therefore wrote their vertex depth (near) instead of the constant
  `0xffffff` texture depth (far), rejecting later geometry. The native clear
  routines now place these quads on the orthographic far plane. Wii paths are
  unchanged. Desktop captures confirmed the starfield and planet reappeared
  with ordinary depth testing enabled.
- **Animation conversion:** `__OSf32tou8` and `__OSf32tos16` had no native body,
  returning uninitialized values. Layout and particle animations use these for
  opacity and color. Native implementations saturate and truncate, with scalar
  regression coverage for negative values, fractions, and both range limits.
- **Device audio pacing:** SDL requests can span multiple AI DMA blocks. Pulling
  the whole request immediately could relatch a block before the game's audio
  thread prepared its successor. A paced producer now feeds a bounded stereo
  ring; SDL only drains it. The DMA-spacing regression passes at 32/48 kHz and
  fails against the old sink (successive interrupts 0.000 ms apart).

These fixes do not establish fidelity of every effect, level, or sound. Device
playback still needs listening confirmation; the timing test does not measure
perceived noise.

Verification after the fixes:
- Desktop inspection: title background, animated A+B prompt and copyright text
  are visible; clicking B produces a soft blue glow rather than the opaque
  jagged white/pink patch.
- `native_core`, `native_audio_sdl`, `native_app_smoke`, `native_app_seam`: 4/4 pass.
  The conversion regression fails against the previous SDK header.
- Fresh full-app `PETARI_SMOKE=gameplay`: PASS at frame 5398, normal shutdown
  exit 0. Log: `build/visual-fix-gameplay.log`. This includes file creation,
  prologue, idle, jump/landing, opposite movement, pause/freeze, and resume.

## Follow-up fidelity checks

- Generated collision now has a typed initialization path, preserving its mutable
  descriptor and explicit triangle count. Its octree leaf is written in host
  order. The root generated-collision and asset tests pass, including 22,293
  triangle queries over 150 disc collision files. This addresses the manual
  playtest crash that treated runtime-generated collision as unconverted disc data.
- Actor lights previously left their direction uninitialized. A live debugger
  read found directions around 2.3e20 in lights 0 and 1, large enough to overflow
  the spotlight calculation. Native light objects now start initialized. The
  optimized regression passes. Desktop inspection confirms the white wash is
  gone. A second bug kept the eyes closed: `getMaterialAnm()` rejected all host
  pointers above 4 GiB using a Wii address test. The native accessor now rejects
  only the original 32-bit sentinel range. Desktop inspection confirms open
  blue irises. Both root material-animation tests pass, covering texture swaps,
  TEV colors, material colors and texture transforms with real disc resources.
- The normal 1280x720-point Retina window already renders at 2560x1440. GPU
  tests show a hard edge at its native texel and a 1:1 presentation scale. The
  prologue illustrations are 608x224 source textures, and many UI icons are
  24–64 pixels; raising render resolution does not add detail to those assets.
  Presentation now fits the render target to the displayed aspect when the
  window has a different shape, avoiding an extra resampling step. The GPU
  regression covers 4:3 and 16:9, resizing, and replacement of stale copies.

The integrated reload run (`build/fidelity-reload.log`) passed at frame 4702
and exited normally with status 0. Idle, jump/landing, opposite movement, pause
freeze and resumed movement passed. The isolated NAND files remained identical
by SHA-256. Across 83 device-audio reports there were zero output underrun
frames, but 17 AI block replays; audio fidelity is not yet established. Replays
cluster around slow frames, so scheduling diagnostics are the next check.


## Audio scheduling follow-up

The first pacing revision eliminated output-ring starvation but still repeated
DMA blocks. CPU-wait samples identified two separate causes:

- A lower-priority thread could be assigned the simulated CPU while its host
  thread was still asleep. An audio interrupt then waited for that sleeper to
  wake before it could yield. The scheduler now immediately redirects an
  unstarted dispatch to the highest-priority ready thread. A deterministic
  dispatch regression fails without this change; OS tests pass in Release,
  ASan and TSan.
- After that fix, `build/fidelity-scheduler-reload.log` passed the gameplay
  checks with zero underruns over 79 reports, but five replays remained during
  a sampled 78.2 ms `FileRipper::decompressSzsSub` call. The native decoder now
  offers higher-priority preemption between groups every 16 KiB of output,
  preserving Wii code and decode state. Root game-data checks pass on 311
  archives, 55 uncompressed files and 16 streamed archives requiring refills.

The sink now refills to a level based on whole device requests, with catch-up
bounded by elapsed time; it never discards pulled samples. The producer and AI
interrupt thread use real-time scheduling, with higher QoS for the DSP and
high-priority OS threads. Buffer latency is roughly 64 ms plus the device's
largest request at 32 kHz. Diagnostics distinguish underruns from repeated AI
blocks (`PETARI_AUDIO_DIAG=1`); optional CPU-wait sampling uses
`PETARI_BATON_DIAG=1`. The diagnostic mutex has process lifetime so its detached
reporter remains safe during normal exit.


Final integrated run: `build/fidelity-final-reload.log`, rebuilt with both
scheduler and decoder fixes. The existing-save gameplay sequence passed and
exited 0. All 79 audio reports had zero underrun frames; one AI block replay
occurred during startup, with none subsequently reported through title, file
load, prologue and gameplay. No CPU-wait report exceeded the diagnostic's 5 ms
reporting threshold. The isolated NAND hashes remained unchanged. This shows
the measured sustained distortion mechanisms are corrected on this route; it
does not establish perceptual audio fidelity or zero stalls in untested levels.

Desktop capture during the arrival sequence confirms white Toad caps with
red/green/blue/yellow spots and distinct clothing colors, without the previous
gold wash. Mario has his red cap and blue overalls. The earlier file-select
capture confirmed open blue irises. These are visual spot checks, not a
pixel-level comparison against Wii output.


The one startup replay is expected silent JAudio2 initialization, not a late
production block: `JASDriver::initAI` clears all DAC buffers and starts buffer 2;
`lastRspMadep` is initially null, so the first `updateDac` interrupt registers
no successor. The zeroed initial buffer therefore plays twice, as on the Wii.
Subsequent interrupts register the prepared buffer. The diagnostic intentionally
counts this event rather than masking it. No unexpected replay was measured
in the final route; listening remains a separate fidelity check.

## Longer-route diagnostics

For timing measurements, use `PETARI_BATON_DIAG=1 PETARI_BATON_SAMPLE=0`
with `PETARI_AUDIO_DIAG=1`. Disabling the stack sampler avoids suspending the
thread being measured. Reports include holder CPU time, interrupt-disable
counts, and a mid-wait host run-state sample. The sample describes one instant,
not the entire wait. Mach CPU accounting may lag a scheduling slice.

With sampling enabled, the diagnostic also reports the measured sampler pause;
its intervention can lengthen the wait. Early timing magnitudes were collected
while two stale TSan test processes were consuming CPU. Those processes were
stopped; do not use those magnitudes as clean-machine benchmarks.

See `RELIABILITY.md` for the expanding story-route coverage and open findings.

### Manual assistance in automated runs

Physical presses and releases of keys or mouse buttons bound to game controls
are now recorded separately from the driver's injected input. A completed run
with such input reports `ASSISTED` and exits 3; it is not an unattended PASS.
Failures and blocked runs retain exit 1 and 2 and note any assistance. The log
records the frame, phase, position, event count and latest event in the first
observed input batch. Pointer motion and focus changes are logged separately;
they do not steer the story route and do not alone change its result.

A playtest observer supplied a jump and steered around a wall in story run 6
(screen recording). That run establishes HeavensDoorGalaxy loading and
heap headroom, but not autonomous traversal. The revised castle route avoids
the curb and building using a collision-derived path with stricter step and
clearance limits. Story run 7 then completed the revised route with no physical gameplay input,
reached HeavensDoorGalaxy for 60 consecutive ready frames at frame 21006, and
exited 0. It recorded 64 pointer movements and 10 focus changes. Three automatic
sidestep recoveries succeeded; no manual jump or steering was needed. Both NAND
files remained byte-identical to the isolated fixture before the runs.


### Latest synchronization run

Story run 9 (`build/proactive-story-9.log`) passed at frame 20242 with no physical
gameplay input and no stuck recoveries. Both movies played, HeavensDoorGalaxy
stayed ready for 60 frames, and both isolated NAND hashes were unchanged.
Its 342 audio reports contained zero underrun frames, one expected silent
startup replay and two unexpected replays. Audio therefore remains an open
reliability item despite the route pass. See `RELIABILITY.md` for timing evidence
and coverage limits.

The AI registration lock now uses owner-aware blocking instead of busy spinning.
The scheduler temporarily requests higher host QoS for a running baton holder
when a higher-priority game thread waits; it ends the override after unlocking.
Audio diagnostic formatting runs on a joined reporter thread rather than the
real-time producer. `[audio-host]` records producer and callback interval stats;
these help localize future failures without asserting their cause.


### Observatory and Good Egg fixture

The `galaxy` smoke loads an isolated post-tutorial fixture, walks from the
observatory to the Terrace, points at the Pull Star, selects Good Egg Galaxy
and mission 1 through the normal UI, then checks movement, jumping and
pause/resume. The native UI hooks only publish visible targets; the driver
uses the normal input bindings. This is not a full mission completion test.
The route has simulated coverage; a live unattended pass is still pending.

Create a fresh fixture from an existing saved-file seed (source is read only):

```sh
python3 native/tools/create_observatory_fixture.py --source PATH_TO_SEED_USER --output NEW_TEST_USER
PETARI_SMOKE=galaxy PETARI_SMOKE_FRAMES=108000 PETARI_TRACE_BOOT=1 \
  build/macos-gx/native/app/Petari.app/Contents/MacOS/Petari \
  --disc build/game-data/SB4E01 --user NEW_TEST_USER --test-fixture observatory
```

The explicit fixture flag requires a marker in that isolated directory and
refuses the normal user directory. At reload it supplies the tutorial Grand
Star and completed observatory introduction on both game-data snapshots.
This bootstrap is not earned progression and does not validate those skipped
sequences. Good Egg progress is otherwise unchanged. Physical bound input
still makes an otherwise successful run `ASSISTED` (exit 3).

The first observatory attempt entered the Terrace but later aborted while
compiling a shader whose indirect texture order referenced a disabled texture
coordinate. The native Aurora patch maps that indirect coordinate to coordinate
0, matching Dolphin's handling, in both shader analysis and generation. Valid
coordinates remain unchanged. The exact failing configuration and all 454
captured GX configurations now pass source generation; this alone does not
validate their Metal compilation or rendered appearance.

Known cached pipelines now finish compiling before the game starts. The window
title reports remaining work and closing the window cancels startup. New
configurations encountered during play can still compile on demand. An optional
`-DPETARI_PIPELINE_SEED=/absolute/path/to/seed.db` CMake setting bundles a curated
Aurora pipeline-configuration database as `Resources/initial_pipeline_cache.db`.
Export a live cache with SQLite's backup API, not a raw copy of its database
file while WAL writes may be active. Do not substitute the device-specific Dawn
cache. No seed is required by default.

### Unlocked saves

`build/saves/all-missions` (Mario, 120 stars) opens every dome, galaxy and
mission through the normal observatory UI. `complete-luigi` and `grand-finale`
add Luigi and the Grand Finale Galaxy. Launch with `--user build/saves/VARIANT`,
or use a copy of it. It needs no fixture flag. The game's own code makes and
checks these saves. See [SAVES.md](SAVES.md) for what each save contains, how
it is made and verified, and how to copy one into your normal save directory.
The observatory fixture's bootstrap now only raises story progress. With
`--test-fixture stage`, a copy of an unlocked save keeps its full progression.

### Human-style macOS window playtest

Use the normal keyboard/mouse route, with no `PETARI_SMOKE` driver. Prepare a new
isolated fixture once, then launch each bounded session through the helper:

```sh
python3 native/tools/cu_playtest.py --prepare
# Freeze the current app (with no build running):
ditto build/macos-gx/native/app/Petari.app build/cu-playtest/frozen/Petari.app
shasum -a 256 build/cu-playtest/frozen/Petari.app/Contents/MacOS/Petari
python3 native/tools/cu_playtest.py --session 1 --app build/cu-playtest/frozen/Petari.app
# After clean quit, use the same isolated save to check persistence:
python3 native/tools/cu_playtest.py --session 2 --app build/cu-playtest/frozen/Petari.app
```

The helper owns `build/cu-playtest/user`,
launches only the app binary with that directory and the observatory marker,
and limits each actual app session to 25 minutes (20 minutes by default).
Run timing measurements on an otherwise idle machine, one app at a time, with
no concurrent builds. It records binary/save hashes,
PID, UTC times, exit status, boot/audio logs and frame CSV. It enables baton
diagnostics with `PETARI_BATON_SAMPLE=0` to avoid sampling-induced pauses. A forced deadline exit
is reported as failure, not a clean quit. `--prepare` refuses to overwrite an
existing fixture; preserve prior evidence before selecting a new output location.

After `build/cu-playtest/active-session.json` appears, confirm that PID is still
running. Paste `native/tools/cu_playtest_repl.js` into `cua_repl` after reading its
first-use documentation, then `await playtest.bind()`. Bind again after **every**
relaunch; do not ask CUA to open the app while waiting for the launcher. CUA's
`getApp` can launch an app if none exists. Do not use `Play MacioGalaxy.command` (or the old `Play Petari.command` forwarder).

The helper's `tap(key,count)`, `click(observedTarget,button)`, `drag(from,to)` and
`capture(label)` send macOS UI events, refresh accessibility state, and save UTC
input records/screenshots under `build/cu-playtest/`. Use coordinates only from
a fresh screenshot. `note(text)` records a judgment or reproduction detail.
The CUA interface has no explicit key-down/hold-duration or mouse-move-only API;
pulses and clicks cannot establish the feel of sustained key or Pull Star holds.
Do not mislabel that interface limit as a game-input failure.

Repeatable scenario script (observe between steps; do not run blind coordinates):

1. Capture title; press Return once to send A+B at title (plain A elsewhere).
   Capture save selection. Select the existing Mario file and enter the
   observatory through normal UI; this fixture supplies post-tutorial progression.
2. Exercise WASD, Space, F, Q/E and C. Compare pointer placement before/after window
   resize; test fullscreen followed by at least 20 seconds of gameplay, focus loss
   and focus regain. Save `build/list-app-windows <own-petari-pid>` output before
   fullscreen, during fullscreen, after restoring windowed mode, and immediately
   after any failed click, using distinct timestamped files beside the screenshots.
   This helper is read-only. Compare window bounds/order with the screenshot before
   attributing a click failure to SDL or CUA. Record whether a game controller is
   attached and the evidence used; silence in device logs does not prove absence.
   Test a single Escape/Minus tap to pause and another to resume. Check that inputs release.
3. Follow visible paths to the Terrace. Interact with its Pull Star, select Good
   Egg, wait for any first-time reveal, and select mission 1.
4. Try movement/jump/spin, pointer/Star Bit collection and shooting, NPC dialogue,
   launch stars and pause/resume. Attempt the mission and return-to-observatory UI.
5. Return to the observatory through the pause menu, then select Quit and confirm
   “Save your progress and quit?” Capture the save result before closing the app
   with Cmd+Q or F1 → Quit. Allow at least two minutes before the session deadline
   for these steps. Check a zero exit and save writes. Relaunch as the next numbered
   session and verify the same file and any earned progress; do not infer save
   persistence merely from fixture state or a changed save-file hash.
6. Quit the final session. Run `python3 native/tools/cu_playtest.py --summarize`
   to refresh `build/cu-playtest/analysis.json` without launching anything.
   Write a prioritized issue/coverage report at
   `build/cu-playtest/REPORT.md`, linking screenshots and timestamped log context.

Good Egg mission 1 landmark route, verified with real held input in session 12:
walk off the first disk's rim to its underside, then go toward the green pipe.
Approach the central stem on the pipe-facing side, approximately a quarter turn
around from the question block. The question-block face is a wall. Enter the
narrow curved brick ramp with sidewalls and keep walking up the stem to its end
cap. Talk to the Luma there to create a Sling Star. Center under it and spin;
spin again when caught by the Launch Star to fly to Peanut. Short repeated F taps
completed this sequence in the playtest; a long screenshot gap between the two
spins can miss the activation window. The stone bridge with the Star Bit crystal
is a dead end and is not required for this route. Observe after every movement
batch rather than replaying fixed coordinates through rotating gravity/camera.

Report observed facts separately from likely causes and untested scenarios. A
screenshot is not continuous video, and device/audio counters are not a listening
assessment. Do not modify game code during this diagnostic task.

For this playtest the optional macOS CGEvent keyboard-hold helper was
authorized, because CUA has no held-key API. Compile it **before**
launching the app:

```sh
swiftc native/tools/cu_hold_input.swift -o build/cu-playtest/cu-hold-input
```

After binding the running app with CUA and making it frontmost, a bounded forward
hold is:

```sh
build/cu-playtest/cu-hold-input build/cu-playtest/active-session.json w 1.0
```

Use fresh CUA accessibility/screenshot observations after each hold. Supported
keys are W/A/S/D, Space, F, Q/E, C and Shift (lowercase argument names); up to three
may be combined with commas. Holds are capped at three seconds. The helper checks
the recorded PID and bundle and the frontmost application.
It logs every key event to `actions.jsonl` and sends cleanup key-up events only
to the original PID on normal return, focus loss, SIGTERM, SIGINT or SIGHUP.
It does not open a macOS permission prompt if event-post access is unavailable.
Continue using CUA for screenshots, pointer operations, window control and all
other interactions. This helper sends OS events; it never injects game inputs or
changes game state directly. SIGKILL cannot run cleanup; prefer the handled
signals when stopping it.

## Stall log: why a frame hitched

Every frame whose wall-clock interval exceeds 20 ms prints one line to stderr (the play log), with no setup:

```
[stall] frame 9973 34.2 ms phase=gameplay cause=pipeline-compile+resource-load (pipelines 3, textures 0, loads 1) stage YosshiHomeGalaxy scenario 1 zone 5
[stall] frame 7 41.0 ms phase=gameplay cause=other (game_work 38.0 ms; pipelines 0, textures 0, loads 0) stage ...
[stall] 12 frames over the threshold during loading, worst 412.0 ms (frame 331); causes pipeline-compile+texture-upload
```

- `frame` is the seam's frame counter (the same one as the `PETARI_FRAME_CSV` index). `phase` is startup, loading, menu or gameplay;
  `unfocused` is appended if the window lacked focus during the frame.
- Causes come from counters bumped where the work happens (`native/include/petari/frame_telemetry.hpp`): `pipeline-compile` is a
  blocking GPU pipeline resolve on the GX processor (`PipelineWait`), `texture-upload` a static texture hash/create/upload
  (`TextureHash`, `TextureUpload`), `resource-load` disc reads on the DVD drive thread (`DiscRead`: JKR archive mounts, file and
  effect-resource loads). A cause counts when it took at least 1 ms in the frame; all that count are listed, the longest first.
  `loads` counts disc reads, including background streaming, so it is a hint, not proof. With no cause the frame is `other`,
  and the dominating game-thread part of the frame (`GameWork`, `DrawDoneWait`, `SeamEndFrame`, ... see `frame_stats.hpp`) is named.
- Startup and Loading stalls are not printed one by one: one summary line is printed when the load ends.
- `PETARI_STALL_MS=<ms>` changes the threshold; `PETARI_STALL_MS=0` turns the log off.
- The M / F9 capture (`PETARI_CAPTURE_DIR`) writes a `stalls (last 300 frames): ...` line into `state.txt`: `none over 20 ms`, or the
  count followed by one `stall frame N ms ... cause=...` line each (only the newest 32 stalls are remembered: `at least 32` means more).
- `other (unattributed N ms ...)` means the game thread's frame-loop marks were missing for that frame or the thread was off-CPU (an
  overloaded host): the stall is real but not one of the named causes.

Code: `native/app/stall_log.hpp` (classifier, ring, rate limit), hooked in `frame_seam.cpp`; test `native_stall_log`.

## No partial scenes: pipelines outside play, and the pop-in log

By default a GX draw whose pipeline is still compiling waits at most 6 ms per
frame and is then skipped until the compile lands (`bounded`, pipeline_runtime.inc),
so that a compile during play costs a short wait rather than a long hitch. Outside active play (the stage's
opening flyover, menus, the file select, loading, pause) that showed a partial
scene: geometry popped in during the galaxy intro's fade-in (2026-10-21: up to
1,600 skipped draws in frames 544-550 of YosshiHomeGalaxy on a cold, unseeded
cache). There draws now wait for their pipelines (the frame, behind the fade,
holds instead), as the original never draws a partial scene. Active play keeps
the bounded wait. "Play" is `Host::playerInPlay`: the game's own
`permitsStartPauseMenu`, no ScenarioStarter opening, demo, system talk or pause,
not the file select or a world map, held for 6 frames.
`PETARI_PIPELINE_COMPLETE_SCENES=0` restores the old behaviour.

`[pop-in] frame N skipped_draws K (intro|play|menu/loading)` is printed for every
frame that skipped draws; `[play ready] frame N wall_ms=...` marks the first
frame of each return to play (launch-to-control timing).

## Recording a tape: a user session that replays exactly

**Your tape, replayed (for the player).** Play as usual from the launcher (5.3 and later): every session saves a tape folder under
`~/MacioGalaxy2-play/tapes/<galaxy>-<time>/` by itself, nothing to switch on (`ASTRAL_TAPE=off` turns it off). Quit the game normally
when you are done, then tell the team the folder name. They replay it with one command, and it says per scene whether the replay
stayed on your exact path and whether it reached the star:

```sh
python3 tools/port/replay_tape.py ~/MacioGalaxy2-play/tapes/<folder> --headless <petari_headless> --disc <extracted disc> --out <new empty folder>
```

A *tape* is a session recorded so that `petari_headless` can play it back and land on the same frames, through scene changes
(galaxy, star get, Starship, the next galaxy). Recording costs nothing you can measure (appends to buffered files at the game's own
input reads).

**An ordinary launcher session writes one**, with no extra step: `native/play/play.sh` (and the double-click `.command`) sets
`PETARI_TAPE_RECORD=~/MacioGalaxy2-play/tapes/<galaxy>-<time>/` for builds from 5.3 on (older builds ignore it). About 150 MB an hour.
`ASTRAL_TAPE=off` turns it off, `ASTRAL_TAPES=<dir>` moves the folder. By hand: `PETARI_TAPE_RECORD=<dir> <the usual launch>`, or
`PETARI_ROUTE_RECORD=<x>.csv` (the files go beside it). It also works in background mode, so scripted sessions (`PETARI_INPUT_SCRIPT`) can
be taped. **Quit the game normally** (a session that is killed leaves a tape valid up to its last checkpoint, 60 loops, with
`"complete": false`; `replay_tape.py --allow-incomplete` replays that part).

The folder holds `route.csv` (every game loop; positions at `%.9g`, the float32 exactly), `route.kpd` (every KPADReadEx the game made:
count, error, **every sample**, the connection it saw; the format `tools/headless.py pack` writes and `petari_headless --inputs`
reads), `route.actions` (mod buttons: Cappy etc., by game loop) and `route.tape.json`: the *segments*, the enabled mods, the exe sha256,
`replay_env` (PETARI_TEST_RETURN_TO_SHIP, a test hook that changes scenes, must fire in the replay too) and counts, including
`unencodable_samples` / `lossy_samples`, which must be 0.

**Segments.** A segment is one scene visit that reached *play-ready*: it starts at the visit's first play-ready seam (the game's own
"ready for play" test, `native/app/play_ready.cpp`, the `[play ready]` log line; the game's RNG seed there is recorded) and runs to the
visit's last seam. Tape frame 0 is the loop after the start seam. The loops between segments (scene teardown, loading, the opening
flyover) are not taped: their length depends on asynchronous loading (the flyover ran ~430 loops longer in headless than in the GX
session), so the replay re-anchors at every segment's own play-ready seam (`--sync-play`, RNG seed set there) and feeds neutral
reads in between. A star get happens inside a segment (the star demo is deterministic scene time); the Starship visit after it is the next
segment.

Replay, one command (needs the extracted disc and a `petari_headless` of the same source as the recording):

```sh
python3 tools/port/replay_tape.py ~/MacioGalaxy2-play/tapes/flip-swap-20261022-101500 \
    --headless build/macos-arm64-headless/native/app/headless/petari_headless \
    --disc ~/MacioGalaxy2-gamedata/SB4E01-extracted --out ~/tapes/replay-1 [--allow-incomplete] [--max-frames N] [--no-ftz]
```

It writes a trimmed copy of the tape, runs the replay (`--stage <first segment>`, `--segments`, `--sync-play`, `--inputs-from-sync`,
`--mod-actions`, PETARI_MODS = the recorded mods, the recorded `replay_env`) and `first-divergence.json`: per segment, the first row where
Mario's position (float32) is more than 0.01 away or stage / scenario / player-valid / dead / grounded differ, the first bit difference and
the number of differing rows; overall `all_segments_matched`, and `reached star: yes|no`: the route CSV's own power-star count rose and
the replay matched through that row. Exit status 0 means every segment matched to its end. Mods are loaded by `petari_headless` exactly as
by the app (`<user>/mods.txt`, then `PETARI_MODS`); mod buttons come from the tape (`petari_mod_take_press/held`), not from a keyboard.

Limits: the replay starts from a fresh user (a tape recorded on a save with progress needs that save copied in; the sidecar does not carry
it); the selected scenario is taken to be the placed one; presses during the loops between segments are not replayed (neutral reads);
`PETARI_TEST_WARP` (a Level Select test driver in the GX app) is not replayed; pointer/mouse-only mods (PointToMove, the editor) are not taped.

### Watch a tape in the game window (GX replay)

A tape can also be replayed in the normal GX window, so you can *watch* the session you played:

```sh
sh native/play/play.sh --replay                      # the newest tape (menu: "Play Astral Italian.command", r)
sh native/play/play.sh --replay ~/MacioGalaxy2-play/tapes/<folder> [--build NAME]
```

or `Petari --replay-tape <folder>` / `PETARI_TAPE_PLAY=<folder>` directly. Physical input is ignored (close the window to stop).
`--replay` runs the chosen build only if its executable is the one that recorded the tape (`exe_sha256` in `route.tape.json`);
otherwise it names the frozen build that is needed and stops, and never silently uses another (`--replay-any-build` runs it anyway;
expect drift, and the report shows where). The replay uses a throwaway copy of your save (`ASTRAL_REPLAY_USER`), never your own.

How it follows the tape is the headless rule (`native/app/tape_play.cpp`, feeding the recorded reads through the headless `Replay`):
the first segment is entered like `--stage` (the app picks file 1 by itself); each segment starts at the first seam where the game's
"ready for play" holds on the segment's stage and scenario, the recorded RNG seed is set there, and the segment's KPAD reads and mod
buttons are fed from then on; between segments the pad is neutral. **The loops between segments are not on the tape** (menus, the
world map, loading), so the app reaches each next segment with the Level Select warp: those transitions are jump cuts, not replays.

At the end (or when the game reads differently than recorded and the replay gives up) the app prints a divergence report and writes
`<tape>-replay-<time>/` next to the tape: a copy of the tape plus `divergence.json` / `divergence.txt`. Per segment it gives the rows
compared, the row where the replayed position first leaves the recorded `route.csv` (more than 0.01, or another stage, scenario, alive
or grounded state), and the largest error. A replay that drifts is shown as drifting; the headless replay is the exact reference.
`PETARI_TAPE_PLAY_OUT=<dir>` chooses the report folder; `PETARI_TAPE_PLAY_QUIT=<loops>` closes the app that many loops after the end
(scripted runs; otherwise the window stays open). CTest: `native_tape_play` (the session, game-free).

The launcher keeps only the 20 newest session tapes in ~/MacioGalaxy2-play/tapes (about 150 MB an hour of play); older ones are removed
when a new session starts. Copy a tape worth keeping (a star get, a bug) to ~/MacioGalaxy2-gamedata/rec-tapes first.

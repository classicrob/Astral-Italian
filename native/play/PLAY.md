# Playing Astral Italian (native Super Mario Galaxy 2)

Astral Italian is the Mac build of the Super Mario Galaxy 2 decompilation. It runs from the frozen build on the
machine that holds your extracted disc. Nothing here touches MacioGalaxy (the SMG1 port) or its saves.


## Build 5.2.1 with Cappy (default)

The default frozen app is `~/MacioGalaxy2-play/build5.2.1/Petari.app`. Pick **c) 5.2.1 with Cappy (default)** or **f) 5.1 (fallback)** in the launcher. **n) 5.3 candidate (not default; pad untested; build5.3)** is selectable once `~/MacioGalaxy2-play/build5.3` has been frozen (until then it reports "no app" and returns to the menu; `--build build5.3` is the same). Build 5.2 also remains available through `--build build5.2`.

Controller (5.3 candidate): the Switch Pro Controller layouts in 5.3 have not yet been tested with a real pad. If the pad misbehaves, switch it off or disconnect it (System Settings > Bluetooth > Disconnect, or unplug the USB cable): the keyboard works the same with or without a pad, and 5.3 has no in-game switch to ignore a connected pad.

If 5.2.1 misbehaves, pick 5.1 in the launcher or run `native/play/play.sh --build build5.1`; press M to capture the problem.

## Build 5.1 (fallback)

The fallback frozen build is **5.1**. It includes the native heap/scene-return changes, Home-menu Level Select consumer, corrected paired-single math, and headless loading/synchronization fixes. The freeze records the source, clean app/headless hashes, replay result and twelve fresh/warm lifecycle checks.

The renderer key is shared with build 5. Matching build-keyed shader caches are reused; older or foreign namespaces are preserved. The invited destinations were warmed against the real machine cache during the freeze checks. First-use effects outside that coverage can still compile shaders. Build 5 remains selectable with `--build build5`; build 4 remains selectable with `--build build4`. Cappy remains unavailable.

## Frozen builds only

The user plays **only frozen builds**. Every player launcher requires a
`Petari.app` under `~/MacioGalaxy2-play/` with a sibling `FROZEN.txt`.
Unfrozen apps and apps outside that tree are refused, including symlinks that
resolve outside it. There is no development-build fallback.

Use the menu's **b** option to choose an available frozen build, or:

```sh
native/play/play.sh --list-builds
native/play/play.sh --build build3 flip-swap
```

`--build default` selects `~/MacioGalaxy2-play/build5.2.1/Petari.app`. Use --build build4 for the preserved build-4 app at the root. `ASTRAL_APP` may
select another frozen app under the same tree and is checked by the same rule.
A marker records the build owner's freeze; it does not isolate saves or caches.
The versioned [persistent-state inventory](PERSISTENT-STATE.md)
lists every runtime destination and its sharing policy. In particular, normal
builds share `~/Library/Caches/AstralItalian` even with separate user folders.

## Start

Double-click **Play Astral Italian.command** (in this folder). A Terminal window opens with a menu:

```
  c) 5.2.1 with Cappy (default)
  f) 5.1 (fallback)
  n) 5.3 candidate (not default; pad untested; build5.3)
  1) Flip-Swap Galaxy      (World 1)
  2) Fluffy Bluff Galaxy   (World 1)
  3) Sky Station Galaxy    (World 1, fallback)
  4) Title screen          (normal start, file select)
  b) Choose a frozen build
  o) Odyssey movement: off
  a) Audio: on
```

Press a number and Return. The game opens its window, goes through the title and file select by itself and drops
you into mission 1 of that galaxy. Quit the game with Cmd+Q (or close the window); the menu comes back.

From a terminal, one command per galaxy (same thing without the menu):

```sh
native/play/play.sh flip-swap       # Flip-Swap Galaxy (RedBlueExGalaxy)
native/play/play.sh fluffy-bluff    # Fluffy Bluff Galaxy (MokumokuValleyGalaxy)
native/play/play.sh sky-station     # Sky Station Galaxy (IslandFleetGalaxy), the fallback
native/play/play.sh title           # the normal start: title screen and file select
native/play/play.sh flip-swap --scenario 2 --odyssey --audio off
```

`--scenario N` picks another mission (the actual mission numbers are listed by
`Petari --disc <disc> --stage list`).

## Options

| What | Menu | Command | Notes |
| --- | --- | --- | --- |
| Odyssey movement (Super Mario Odyssey moveset) | `o` | `--odyssey` | For this run only. In the game: the Home menu (` or F1), then Mods, then Odyssey movement (saved in `mods.txt`). Builds 3 and later include the Odyssey hooks (build2/ and build1/ do not). Moves and keys: native/MODS.md, "Odyssey movement". |
| Audio | `a` | `--audio off` | On by default since build 4. Off runs the game's audio through a silent driver. |

Paths can be changed with environment variables: `ASTRAL_APP` (the frozen `Petari.app`, default
`~/MacioGalaxy2-play/build5.2.1/Petari.app`), `ASTRAL_DISC` (the extracted disc, the folder holding `files/`, default
`~/MacioGalaxy2-gamedata/SB4E01-extracted`), `ASTRAL_USER` (saves, settings, controls, logs; default
`~/MacioGalaxy2-play/user`). The scripts refuse MacioGalaxy's own user folder
(`~/Library/Application Support/Petari`), so its saves are never read or written.

## Everything-unlocked save (separate progress)

Use the menu's **u** toggle, or `native/play/play.sh title --unlocked`, then
**pick file 1 for everything unlocked**. The option uses only
`~/Library/Application Support/AstralItalian-unlocked`. Normal progress in
`~/MacioGalaxy2-play/user`, `~/Library/Application Support/AstralItalian`, and
the SMG1 `Petari` directory is not modified by creation or this option.
Direct entry also works: `native/play/play.sh WorldMap07Galaxy --unlocked` or
`native/play/play.sh MarioFaceShipGalaxy --unlocked`.

The build owner creates the file once, through the game's own star, progression
and save/loader APIs (no save-byte patching):

```sh
python3 native/tools/complete_save.py \
  --executable build/macos-arm64-headless/native/app/headless/petari_headless
```

Creation refuses a nonempty destination or an existing game file. It completes
all 242 actual star records, opens galaxies and SMG2 progression flags, marks
world checkpoints and Hungry Luma routes complete, saves normally, reloads and
checks all stars/unlocks plus unchanged other save records. The same command
runs the World S map and Starship for at least 18,000 ready frames each; logs,
traces and `acceptance.json` stay privately under
`~/MacioGalaxy2-gamedata/unlocked-validation`. A later check uses `--verify-only`
and awards nothing. `complete-save.json` in the separate user directory records
the regular save/reload check; it is not by itself a five-minute runtime pass.

## Controls (keyboard and mouse)

The same keys as MacioGalaxy (full list and remapping: native/CONTROLS.md; the Home menu, ` or F1, remaps them in the game).

| Action | Key |
| --- | --- |
| Move | W A S D |
| Jump, confirm, advance text | Space |
| Spin (shake the Remote) | F |
| Crouch; ground pound in the air | Shift |
| Star Pointer | mouse |
| Shoot Star Bits; back in menus | left mouse button (or B, Backspace) |
| Grab with the pointer / Pull Star | hold right mouse button |
| Rotate camera | Q / E (or Left / Right) |
| Recenter camera | C |
| First-person view | Up (Down to leave) |
| Pause | Escape |
| Start | Return |
| Home menu: controls, mods, level select | ` (between Escape and Tab) or F1 |
| Co-star Luma (second player pointer) | F2 |
| Walk slowly | hold Left Alt while moving |
| Playtest capture (last 5 s of input + Mario state + screenshot to `~/MacioGalaxy2-play/captures/`; build 5 and later) | M or F9 |

Power-ups use the same keys: Cloud Mario makes a cloud platform with F (Spin), Spin Drill starts with F, Yoshi
jumps with Space (hold to flutter), his tongue is the left mouse button where you point.

## When something goes wrong

Each run writes a log to `~/MacioGalaxy2-play/user/logs/` (the menu prints its name). When the game stops on its
own, the menu shows the last lines of that log. Restart with the same menu entry; saves stay in
`~/MacioGalaxy2-play/user`.

## More galaxies (headless boot sweep, 2026-10-20 04:00)
A no-graphics sweep booted every galaxy folder on the disc with `--stage`. These **40** reached Mario standing in the level. Only Flip-Swap and Fluffy Bluff have been tested with graphics, so the others may still have rendering, object or audio problems, but they are worth a try:

```sh
native/play/play.sh <InternalName> [--odyssey]     # e.g. native/play/play.sh IslandFleetGalaxy
```

- `AbekobeGalaxy`
- `BigGalaxy`
- `ChallengeGliderGalaxy`
- `ChaosGravityGalaxy`
- `ChildRoomGalaxy`
- `CloudGardenGalaxy`
- `DigMineGalaxy`
- `FlowerHighlandGalaxy`
- `GoroRockGalaxy`
- `HomeCenterGalaxy`
- `HoneyBeeVillageGalaxy`
- `IslandFleetGalaxy`
- `JungleGliderGalaxy`
- `KachikochiLavaGalaxy`
- `KingOfKingsGalaxy`
- `LongForCastleGalaxy`
- `MadnessOnimasuGalaxy`
- `MagicGalaxy`
- `MarioFaceShipGalaxy`
- `MarioOnMarioGalaxy`
- `MokumokuValleyGalaxy`
- `OmoteuLandGalaxy`
- `PeachCastleGalaxy`
- `QuicksandGalaxy`
- `RedBlueExGalaxy`
- `SecretAthleticGalaxy`
- `StarCreekGalaxy`
- `SwitchMoveBlockGalaxy`
- `TamakoroPlanetGalaxy`
- `TamakoroSliderGalaxy`
- `TeresaLabyrinthGalaxy`
- `ThunderFleetGalaxy`
- `TimerSwitchingPlatformGalaxy`
- `UnderGroundDangeonGalaxy`
- `VsKoopaGravityGalaxy`
- `VsKoopaJrLv1Galaxy`
- `VsKoopaJrLv3Galaxy`
- `VsKoopaLv1Galaxy`
- `WanwanFactoryGalaxy`
- `WhiteSnowGalaxy`

Build 5.1 checks WorldMap01 load, walking and idle. Full world-map travel is not established. WorldMap08 is unsupported direct entry (aggregate data, not a normal playable world) and is excluded from scored coverage. Historical sweep evidence remains separate from the current freeze gates.

## Build 5: preserved historical build

The app and its clean-build provenance are in ~/MacioGalaxy2-play/build5/. The sibling PERSISTENT-STATE.md inventories saves, settings and keyed cache paths. Build 4 and its saves remain at their original paths; select it with --build build4.

- On preserved build 5, clearing a galaxy and returning to the Starship can abort with the native heap-budget check (HeapMemoryWatcher 617). Build 5.1 includes the heap changes; its freeze checks the normal scene-return request, not an actual power-star award/cutscene/save-clear sequence.
- Star Bit mods: confirmed by the player in Fluffy Bluff on build 5 (2026-10-20): G pulls in the Star Bits and V shoots the nearest enemy ("All that seemed to work well!"). The automated M1 check had not seen V fire (no enemy in range).
- Odyssey movement: the moveset (long jump, dive, wall jump, side flip, backflip) was confirmed by the player in Fluffy Bluff on build 5 (2026-10-20).
- Build 5 starts with a fresh renderer-build-keyed shader cache and the bundled seed. Old cache files are ignored and preserved, not deleted. New effects outside the seed can still compile on first use.

## Historical build 4 checks (frozen 2026-10-20 09:05)

What was checked (`~/MacioGalaxy2-play/FROZEN.txt` has the numbers):
- **Flip-Swap Galaxy** and **Fluffy Bluff Galaxy**: start, load and play; 120-second smoke runs pass with Odyssey movement
  off and on. Flip-Swap was also played with scripted input: walking, jumping (a held jump rises 260 units and lands
  after 44 frames), running, falling off the panel edges. Two identical runs gave frame-identical results.
- **Odyssey movement**: the menu's `o` (or `--odyssey`) really switches it: the same crouch + Spin input rolls Mario about
  1,350 units with it on and leaves him standing with it off. Cappy and the Odyssey camera are separate mods, not in
  this build.

Known issues (honest list):
- **Home-menu Level Select does nothing** (builds 4 and 5): Go sets `PlayerLaunch::warpPending`
  (native/home_menu/hbm_bridge.cpp), but no SMG2 code consumes it. MacioGalaxy does it in `GameScene::update`
  (once `isPermitToPauseMenu()`, through the game's own galaxy move); that hook was never carried over. Use the
  launcher's galaxy menu or `--stage` instead. Build 5.1 includes the GameScene consumer.
- **Build 4 and Yoshi Star Galaxy / poisoned shader cache**: build 4 aborts in Yoshi Star Galaxy ("unhandled tcg src
  21": the alpha-shadow pass draws with zero texgens; fixed for build 5 in the Aurora pipeline layer). Aurora stores a
  pipeline config before compiling it, so the crash leaves that row in the shared cache, and build 4 prebuilds every
  cached row at startup without validation: each later launch then aborts within seconds. Avoid Yoshi Star Galaxy on
  build 4. After any build-4 crash, move `~/Library/Caches/AstralItalian/pipeline_cache.db`, `-wal` and `-shm` aside
  (keep them as a backup; leave `dawn_cache` and the saves alone) before relaunching. Build 5 validates cached rows
  and keys the cache per build, so rows from another build are never loaded.
- **Audio is on by default** (since the morning of 2026-10-20). It was measured, not yet listened to by a person: a raw
  capture of the device feed over 3.5 minutes of each galaxy had no underruns, no replayed blocks, no clipping and no
  dropouts during play, with music around -20 dBFS RMS. If anything sounds wrong (pitch, envelopes, stuck notes),
  press `a` in the menu to turn it off and tell us what you heard.
- **World maps** (new in build 4): they load and render (WorldMap01 checked in the app, 01/02 for 30,000 frames
  headless). Walking the map and entering a galaxy from it were not checked yet.
- **First-run hitches**: the first time a new effect or object appears, the game can stall briefly while its shader
  compiles (about 15 short hitches in two minutes of Fluffy Bluff on a first run). They go away on later runs: the
  shaders are cached under `~/Library/Caches/AstralItalian`, and on this Mac that cache is already warm for Flip-Swap and
  Fluffy Bluff (no frame over 25 ms in 90 s of each).
- **Untested**: Sky Station (the fallback entry), every other galaxy, the title-screen path (entry 4) beyond the file
  select, the world map, Yoshi, Cloud/Rock/Spin Drill power-ups (Fluffy Bluff's Cloud Flower included), co-star mode,
  and the Odyssey moves other than the roll (long jump, dive, ground-pound jump, wall jump, sideflip, backflip).
- **Not compared with a real Wii**: jump height and run speed come from the game's own code but were not checked
  against Dolphin yet.
- The Home menu uses a built-in font (the SMG1 font file it looks for is not on the SMG2 disc).
- Fluffy Bluff loads a little slower than Flip-Swap (more sound data).

Older builds: `~/MacioGalaxy2-play/build3/` (Odyssey, no world map), `build2/` (no Odyssey hooks) and `build1/`
(Flip-Swap only). Start one with `ASTRAL_APP=~/MacioGalaxy2-play/build3/Petari.app native/play/play.sh flip-swap`.

## For the build owner: the 60-second smoke

`native/play/smoke_frozen.sh [flip-swap|fluffy-bluff|sky-station]` starts the frozen app in the background (silent
audio, no physical input, a fresh user folder under `~/MacioGalaxy2-play/smoke`), asks for mission 1 of the galaxy,
quits it after 60 seconds and passes when it reached a menu scene (logo/title/file select), logged the stage request,
played at least 120 gameplay frames and never crashed or panicked (the app's per-frame phase CSV and log are kept in
the run folder). Run it for each galaxy before handing the build over.

# Playing Astral Italian (native Super Mario Galaxy 2)

Astral Italian is the Mac build of the Super Mario Galaxy 2 decompilation. It runs from the frozen build on the
machine that holds your extracted disc. Nothing here touches MacioGalaxy (the SMG1 port) or its saves.

## Start

Double-click **Play Astral Italian.command** (in this folder). A Terminal window opens with a menu:

```
  1) Flip-Swap Galaxy      (World 1)
  2) Fluffy Bluff Galaxy   (World 1)
  3) Sky Station Galaxy    (World 1, fallback)
  4) Title screen          (normal start, file select)
  o) Odyssey movement: off
  a) Audio: off
```

Press a number and Return. The game opens its window, goes through the title and file select by itself and drops
you into mission 1 of that galaxy. Quit the game with Cmd+Q (or close the window); the menu comes back.

From a terminal, one command per galaxy (same thing without the menu):

```sh
native/play/play.sh flip-swap       # Flip-Swap Galaxy (RedBlueExGalaxy)
native/play/play.sh fluffy-bluff    # Fluffy Bluff Galaxy (MokumokuValleyGalaxy)
native/play/play.sh sky-station     # Sky Station Galaxy (IslandFleetGalaxy), the fallback
native/play/play.sh title           # the normal start: title screen and file select
native/play/play.sh flip-swap --scenario 2 --odyssey --audio on
```

`--scenario N` picks another mission (the actual mission numbers are listed by
`Petari --disc <disc> --stage list`).

## Options

| What | Menu | Command | Notes |
| --- | --- | --- | --- |
| Odyssey movement (Super Mario Odyssey moveset) | `o` | `--odyssey` | For this run only. In the game: F1, then Mods, then Odyssey movement (saved in `mods.txt`). The frozen build 3 includes the Odyssey hooks (build2/ and build1/ do not). Moves and keys: native/MODS.md, "Odyssey movement". |
| Audio | `a` | `--audio on` | Off by default (the game still runs its audio, through a silent driver). Turn it on if it sounds right on this build. |

Paths can be changed with environment variables: `ASTRAL_APP` (the frozen `Petari.app`, default
`~/MacioGalaxy2-play/Petari.app`), `ASTRAL_DISC` (the extracted disc, the folder holding `files/`, default
`~/MacioGalaxy2-gamedata/SB4E01-extracted`), `ASTRAL_USER` (saves, settings, controls, logs; default
`~/MacioGalaxy2-play/user`). The scripts refuse MacioGalaxy's own user folder
(`~/Library/Application Support/Petari`), so its saves are never read or written.

## Controls (keyboard and mouse)

The same keys as MacioGalaxy (full list and remapping: native/CONTROLS.md; F1 remaps them in the game).

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
| Home menu: controls, mods, level select | F1 |
| Co-star Luma (second player pointer) | F2 |
| Walk slowly | hold Left Alt while moving |

Power-ups use the same keys: Cloud Mario makes a cloud platform with F (Spin), Spin Drill starts with F, Yoshi
jumps with Space (hold to flutter), his tongue is the left mouse button where you point.

## When something goes wrong

Each run writes a log to `~/MacioGalaxy2-play/user/logs/` (the menu prints its name). When the game stops on its
own, the menu shows the last lines of that log. Restart with the same menu entry; saves stay in
`~/MacioGalaxy2-play/user`.

## Status and known issues (build 3, frozen 2026-10-20 02:45)

What was checked (`~/MacioGalaxy2-play/FROZEN.txt` has the numbers):
- **Flip-Swap Galaxy** and **Fluffy Bluff Galaxy**: start, load and play; 120-second smoke runs pass with Odyssey movement
  off and on. Flip-Swap was also played with scripted input: walking, jumping (a held jump rises 260 units and lands
  after 44 frames), running, falling off the panel edges. Two identical runs gave frame-identical results.
- **Odyssey movement**: the menu's `o` (or `--odyssey`) really switches it: the same crouch + Spin input rolls Mario about
  1,350 units with it on and leaves him standing with it off. Cappy and the Odyssey camera are separate mods, not in
  this build.

Known issues (honest list):
- **Audio is off by default** (silent driver; the game still runs its audio code). It was never listened to on this
  build: turn it on with `a` at your own risk.
- **First-run hitches**: the first time a new effect or object appears, the game can stall briefly while its shader
  compiles (about 15 short hitches in two minutes of Fluffy Bluff on a first run). They go away on later runs: the
  shaders are cached under `~/Library/Caches/AstralItalian`.
- **Untested**: Sky Station (the fallback entry), every other galaxy, the title-screen path (entry 4) beyond the file
  select, the world map, Yoshi, Cloud/Rock/Spin Drill power-ups (Fluffy Bluff's Cloud Flower included), co-star mode,
  and the Odyssey moves other than the roll (long jump, dive, ground-pound jump, wall jump, sideflip, backflip).
- **Not compared with a real Wii**: jump height and run speed come from the game's own code but were not checked
  against Dolphin yet.
- The F1 menu uses a built-in font (the SMG1 font file it looks for is not on the SMG2 disc).
- Fluffy Bluff loads a little slower than Flip-Swap (more sound data).

Older builds: `~/MacioGalaxy2-play/build2/` (no Odyssey hooks; Flip-Swap + Fluffy Bluff) and `build1/` (Flip-Swap
only). Start one with `ASTRAL_APP=~/MacioGalaxy2-play/build2/Petari.app native/play/play.sh flip-swap`.

## For the build owner: the 60-second smoke

`native/play/smoke_frozen.sh [flip-swap|fluffy-bluff|sky-station]` starts the frozen app in the background (silent
audio, no physical input, a fresh user folder under `~/MacioGalaxy2-play/smoke`), asks for mission 1 of the galaxy,
quits it after 60 seconds and passes when it reached a menu scene (logo/title/file select), logged the stage request,
played at least 120 gameplay frames and never crashed or panicked (the app's per-frame phase CSV and log are kept in
the run folder). Run it for each galaxy before handing the build over.

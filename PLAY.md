# Playing Astral Italian (Super Mario Galaxy 2, native macOS)

Your own disc is the source of all game data. It stays private in `~/MacioGalaxy2-gamedata/`
(`SB4E01.iso`, extracted to `SB4E01-extracted/`). Nothing from it is in this repository.

## Launch

Double-click **`Play Astral Italian.command`** in the repository folder. It starts
**Flip-Swap Galaxy**. At the file select, pick any file (or start a new one): the game jumps
straight to the galaxy. It does not award stars or unlock anything in your file.

Other galaxies: run it from Terminal with a galaxy name (internal name, alias or English name):

```sh
./"Play Astral Italian.command"                         # Flip-Swap Galaxy (RedBlueExGalaxy)
./"Play Astral Italian.command" fluffy-bluff-galaxy     # Fluffy Bluff Galaxy (MokumokuValleyGalaxy)
./"Play Astral Italian.command" RedBlueExGalaxy --scenario 2
```

Saves go to `~/MacioGalaxy2-gamedata/play-user` (set `GALAXY_USER_DIR` to use another folder).

## Controls

Same keyboard layout as MacioGalaxy. Press **F1** in game for the full list and to remap
(saved to `controls.txt`). The basics: WASD move, Space jump, F spin, Shift crouch / ground pound,
mouse = Star Pointer, left click shoots Star Bits, Q/E camera, C recenter, Return start/pause,
Esc back. Full table: `native/CONTROLS.md`.

## Building

```sh
cmake --build build/macos-arm64-release --target petari -j 8
```

(First-time configure: see `native/HARNESS.md` and `scratch-port/BOOT-STATE.md` in the team folder.)

## Known not to work yet

This file is updated as the overnight work progresses; see the status section below.

## Status
- Native boot: reaches the title/file select with the same random seed as the original game.
- Galaxies: Flip-Swap first, then Fluffy Bluff. Stage arrival and play are being brought up tonight.
- Audio: off or partial. Odyssey movement mod: being ported by the core lane, not in this build yet.

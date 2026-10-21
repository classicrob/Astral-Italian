# Playing Astral Italian (Super Mario Galaxy 2, native macOS)

Your own disc is the source of all game data. It stays private in `~/MacioGalaxy2-gamedata/`
(`SB4E01.iso`, extracted to `SB4E01-extracted/`). Nothing from it is in this repository.


## Build 5.2.1 with Cappy (default)

The default frozen app is `~/MacioGalaxy2-play/build5.2.1/Petari.app`. Pick **c) 5.2.1 with Cappy (default)** or **f) 5.1 (fallback)** in the launcher. **n) 5.3 candidate (not default; build5.3)** is selectable once `~/MacioGalaxy2-play/build5.3` has been frozen (until then it reports "no app" and returns to the menu; `--build build5.3` is the same). Build 5.2 also remains available through `--build build5.2`.

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
The versioned [persistent-state inventory](native/play/PERSISTENT-STATE.md)
lists every runtime destination and its sharing policy. In particular, normal
builds share `~/Library/Caches/AstralItalian` even with separate user folders.

## Launch

Double-click **`Play Astral Italian.command`** in the repository folder. A menu opens:

```
  c) 5.2.1 with Cappy (default)
  f) 5.1 (fallback)
  n) 5.3 candidate (not default; build5.3)
  1) Flip-Swap Galaxy      (World 1)
  2) Fluffy Bluff Galaxy   (World 1)
  3) Sky Station Galaxy    (World 1, fallback)
  4) Title screen          (normal start, file select)
  b) Choose a frozen build
  o) Odyssey movement: off
  a) Audio: on
```

Press a number and Return. The game skips the title, opens file 1 (creating it if needed) and drops you into
mission 1 of that galaxy. It does not award stars or unlock anything in the file. Quit with Cmd+Q.

It plays frozen build 5.2.1 at `~/MacioGalaxy2-play/build5.2.1/Petari.app` (see `FROZEN.txt` next to it). Saves, settings and logs go
to `~/MacioGalaxy2-play/user`; MacioGalaxy's own folder (`~/Library/Application Support/Petari`) is refused.

## Odyssey movement: one toggle

**Off by default.** In the menu press **`o`** (it shows `Odyssey movement: ON`), then pick a galaxy. From Terminal:

```sh
./"Play Astral Italian.command" flip-swap --odyssey
```

In the game: the Home menu (` or F1), then Mods, then Odyssey movement (saved in `mods.txt`). The moves and keys are in
`native/MODS.md`, "Odyssey movement". The switch only does something on a build that includes the Odyssey hooks;
the frozen build 3 does.

## What to look for (Flip-Swap)

- The red and blue flip panels swap when you spin (F). Each spin flips every panel.
- If the panel under Mario flips away, he falls. That is the galaxy working, not a bug.
- The camera follows Mario smoothly and should not snap or jump when the panels flip.
- Jumps: a full jump (Space held) rises about 260 units and lands after about 44 frames (measured on this build).
  Mario stands still when you let go (no drift).
- Coins, purple coins, the 1-Up and the electric rails show up and react.

## Something looks or feels off? Press M (or F9) (build 5 and later)

Available in build 5.1 and later. On build 4, just tell us what happened and roughly when. From build 5 on:
M (or F9) saves a capture to `~/MacioGalaxy2-play/captures/<date-time>/`: the last 5 seconds (300 frames) of your inputs and
Mario's movement, Mario's state at that moment, and a screenshot. Nothing appears on screen; keep playing. Afterwards,
write one line about what felt off into that folder's `state.txt` (the last line, "note:"), or just say which capture it
was. Each capture can be replayed and measured.

## Controls

The same keys as MacioGalaxy. Press **`** (the key between Escape and Tab; F1 works too) in the game for the full list and to remap. Basics: WASD move,
Space jump, F spin, Shift crouch / ground pound, ` (the key between Escape and Tab) or F1 opens the Home menu
(controls, mods), mouse = Star Pointer, left click shoots Star Bits, Q/E camera,
C recenter, Return start, Escape pause. Full table: `native/play/PLAY.md` and `native/CONTROLS.md`.

## More

The full guide (options, environment variables, logs, the frozen-build smoke) is
[`native/play/PLAY.md`](native/play/PLAY.md).

## Status and known issues

- **Home-menu Level Select does nothing (builds 4 and 5).** Choosing a galaxy and **Go** closes the menu but never
  warps: the warp it queues is not picked up by the game in those builds (consumer included in build 5.1). To play a
  particular galaxy, use the launcher's galaxy menu or `play.sh <galaxy>` / `--stage` instead.

Build 5.1 remains a fallback. Its freeze record is ~/MacioGalaxy2-play/build5.1/FROZEN.txt. Build 5 is preserved and selectable with --build build5.

- On preserved build 5, clearing a galaxy and returning to the Starship can abort with the native heap-budget check (HeapMemoryWatcher 617). Build 5.1 includes the heap changes; its freeze checks the normal scene-return request, not an actual power-star award/cutscene/save-clear sequence.
- Star Bit mods: confirmed by the player in Fluffy Bluff on build 5 (2026-10-20): G pulls in the Star Bits and V shoots the nearest enemy ("All that seemed to work well!"). The automated M1 check had not seen V fire (no enemy in range).
- Odyssey movement: the moveset (long jump, dive, wall jump, side flip, backflip) was confirmed by the player in Fluffy Bluff on build 5 (2026-10-20).
- Build 5 starts with a fresh renderer-build-keyed shader cache and the bundled seed. Old cache files are ignored and preserved, not deleted. New effects outside the seed can still compile on first use.


- **Build 4: avoid Yoshi Star Galaxy.** It crashes there (a renderer bug fixed for build 5), and the crash can leave a
  bad entry in the shader cache that makes every later build-4 launch crash at startup. After ANY build-4 crash, move
  `~/Library/Caches/AstralItalian/pipeline_cache.db*` (the `.db`, `-wal` and `-shm` files) aside into a backup folder
  before relaunching; do not touch `dawn_cache` or your saves. Build 5 keeps a separate, checked cache per build.
- Historical build 4 (frozen 09:05; selectable with --build build4): Flip-Swap and Fluffy Bluff load and play; smoke runs pass with Odyssey movement off
  and on, and the toggle demonstrably changes Mario's moves (crouch + Spin rolls him only with it on). New: the world
  maps load (walking them and entering galaxies from them not checked yet).
- Audio: ON by default since build 4. Measured, not yet listened to by a person: no underruns or dropouts in 3.5 minutes
  of each galaxy, no clipping, music around -20 dBFS. If it sounds wrong, press `a` in the menu to turn it off.
- First run: brief hitches the first time new effects appear (shaders compiling); later runs use the cache, which is
  already warm on this Mac for Flip-Swap and Fluffy Bluff.
- The 5.1 freeze checks bounded Yoshi Star, Flip-Swap, Fluffy Bluff, unlocked-save ship load, WorldMap01 walking/idle and galaxy-to-ship scene return. It does not establish every galaxy, Yoshi gameplay, power-ups, full world-map travel, co-star mode or actual star-clear/save-return coverage.
- The full list and the measured numbers: [`native/play/PLAY.md`](native/play/PLAY.md), "Status and known issues",
  and `~/MacioGalaxy2-play/build5.1/FROZEN.txt`. Older builds: `~/MacioGalaxy2-play/build3/` (last night's), `build2/`, `build1/`.

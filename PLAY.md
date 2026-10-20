# Playing Astral Italian (Super Mario Galaxy 2, native macOS)

Your own disc is the source of all game data. It stays private in `~/MacioGalaxy2-gamedata/`
(`SB4E01.iso`, extracted to `SB4E01-extracted/`). Nothing from it is in this repository.

## Launch

Double-click **`Play Astral Italian.command`** in the repository folder. A menu opens:

```
  1) Flip-Swap Galaxy      (World 1)
  2) Fluffy Bluff Galaxy   (World 1)
  3) Sky Station Galaxy    (World 1, fallback)
  4) Title screen          (normal start, file select)
  o) Odyssey movement: off
  a) Audio: on
```

Press a number and Return. The game skips the title, opens file 1 (creating it if needed) and drops you into
mission 1 of that galaxy. It does not award stars or unlock anything in the file. Quit with Cmd+Q.

It plays the frozen build `~/MacioGalaxy2-play/Petari.app` (see `FROZEN.txt` next to it). Saves, settings and logs go
to `~/MacioGalaxy2-play/user`; MacioGalaxy's own folder (`~/Library/Application Support/Petari`) is refused.

## Odyssey movement: one toggle

**Off by default.** In the menu press **`o`** (it shows `Odyssey movement: ON`), then pick a galaxy. From Terminal:

```sh
./"Play Astral Italian.command" flip-swap --odyssey
```

In the game: F1, then Mods, then Odyssey movement (saved in `mods.txt`). The moves and keys are in
`native/MODS.md`, "Odyssey movement". The switch only does something on a build that includes the Odyssey hooks;
the frozen build 3 does.

## What to look for (Flip-Swap)

- The red and blue flip panels swap when you spin (F). Each spin flips every panel.
- If the panel under Mario flips away, he falls. That is the galaxy working, not a bug.
- The camera follows Mario smoothly and should not snap or jump when the panels flip.
- Jumps: a full jump (Space held) rises about 260 units and lands after about 44 frames (measured on this build).
  Mario stands still when you let go (no drift).
- Coins, purple coins, the 1-Up and the electric rails show up and react.

## Something looks or feels off? Press F9 (build 5 and later)

Not in the current default build 4: there, just tell us what happened and roughly when. From build 5 on:
F9 saves a capture to `~/MacioGalaxy2-play/captures/<date-time>/`: the last 5 seconds (300 frames) of your inputs and
Mario's movement, Mario's state at that moment, and a screenshot. Nothing appears on screen; keep playing. Afterwards,
write one line about what felt off into that folder's `state.txt` (the last line, "note:"), or just say which capture it
was. Each capture can be replayed and measured.

## Controls

The same keys as MacioGalaxy. Press **F1** in the game for the full list and to remap. Basics: WASD move,
Space jump, F spin, Shift crouch / ground pound, mouse = Star Pointer, left click shoots Star Bits, Q/E camera,
C recenter, Return start, Escape pause. Full table: `native/play/PLAY.md` and `native/CONTROLS.md`.

## More

The full guide (options, environment variables, logs, the frozen-build smoke) is
[`native/play/PLAY.md`](native/play/PLAY.md).

## Status and known issues

- Build 4 (default, frozen 09:05): Flip-Swap and Fluffy Bluff load and play; smoke runs pass with Odyssey movement off
  and on, and the toggle demonstrably changes Mario's moves (crouch + Spin rolls him only with it on). New: the world
  maps load (walking them and entering galaxies from them not checked yet).
- Audio: ON by default since build 4. Measured, not yet listened to by a person: no underruns or dropouts in 3.5 minutes
  of each galaxy, no clipping, music around -20 dBFS. If it sounds wrong, press `a` in the menu to turn it off.
- First run: brief hitches the first time new effects appear (shaders compiling); later runs use the cache, which is
  already warm on this Mac for Flip-Swap and Fluffy Bluff.
- Untested: Sky Station and every other galaxy, power-ups (Cloud Flower), Yoshi, world-map travel, co-star mode, and the
  Odyssey moves other than the roll.
- The full list and the measured numbers: [`native/play/PLAY.md`](native/play/PLAY.md), "Status and known issues",
  and `~/MacioGalaxy2-play/FROZEN.txt`. Older builds: `~/MacioGalaxy2-play/build3/` (last night's), `build2/`, `build1/`.

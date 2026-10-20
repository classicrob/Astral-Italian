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
  a) Audio: off
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
the status below says whether the frozen build does.

## What to look for (Flip-Swap)

- The red and blue flip panels swap when you spin (F). Each spin flips every panel.
- If the panel under Mario flips away, he falls. That is the galaxy working, not a bug.
- The camera follows Mario smoothly and should not snap or jump when the panels flip.
- Jumps: a single jump rises about 200 units and lands after about 40 frames. Mario stands still when you let go
  (no drift).
- Coins, purple coins, the 1-Up and the electric rails show up and react.

## Controls

The same keys as MacioGalaxy. Press **F1** in the game for the full list and to remap. Basics: WASD move,
Space jump, F spin, Shift crouch / ground pound, mouse = Star Pointer, left click shoots Star Bits, Q/E camera,
C recenter, Return start, Escape pause. Full table: `native/play/PLAY.md` and `native/CONTROLS.md`.

## More

The full guide (options, environment variables, logs, the frozen-build smoke) is
[`native/play/PLAY.md`](native/play/PLAY.md).

## Status and known issues

- Flip-Swap Galaxy: plays (Mario, panels, HUD, walking, jumping, falling off edges).
- Fluffy Bluff Galaxy: may not load. The stage loads in the headless harness, but in the app it can sit on the
  loading screen while the stage's sound data loads. If nothing happens after a minute, quit and pick Flip-Swap.
- Sky Station Galaxy: the fallback, not checked in this build yet.
- Audio: off by default (the game runs its audio through a silent driver). Turn it on with `a` at your own risk.
- Odyssey movement: not in the frozen build yet; the toggle does nothing until it is.
- SMG2-only systems (Yoshi, Cloud Flower, the world map, co-star mode) run their decompiled code but were not
  validated natively.

# Astral Italian

> [!IMPORTANT]
> **Astral Italian** is an independent, AI-agent-built continuation of the Super Mario Galaxy 2 decomp, plus a native macOS port.
> It builds on the human work of [Garigari](https://github.com/SMGCommunity/Garigari) (project scaffolding, symbols, splits, initial matching)
> and [Petari](https://github.com/SMGCommunity/Petari) (SMG1 code reused for shared engine code).
> It isn't affiliated with or endorsed by either project. Project code is CC0, like upstream; third-party dependencies retain their own licences.

![Code 96.16%](https://img.shields.io/badge/code-96.16%25-2185d0)
![Functions 98.96%](https://img.shields.io/badge/functions-98.96%25-2185d0)
![Data 86.35%](https://img.shields.io/badge/data-86.35%25-2185d0)
![Fuzzy 99.8%](https://img.shields.io/badge/fuzzy-99.8%25-2185d0)

![Decompilation progress by source unit; area is code size, colour is matched code percentage](docs/images/progress-treemap.svg)

Regenerate after a Wii build: `python3 tools/team/progress_treemap.py build/SB4E01/report.json docs/images/progress-treemap.svg`.

## What this is

Astral Italian combines a Super Mario Galaxy 2 decompilation (USA Rev 0,
`SB4E01`) with a native macOS port for Apple Silicon. **No game assets are
included. You need your own disc.**

## Status

As of **2026-10-25**:

- **96.16% matched code** (6,028,644 / 6,269,292 bytes), **98.96% matched
  functions** (49,831 / 50,352), **86.35% matched data** (1,468,907 /
  1,701,146 bytes) and **99.82% fuzzy match**. Matched code is different
  from fully completed/linked code, currently **1.50%**. These are local
  objdiff report numbers, not upstream's decomp.dev statistics.
  All data credit is objdiff-exact; none comes from linker-behaviour
  arguments. The unmatched code that remains is mostly register and
  stack-slot differences; a triage of every remaining function, with the 72
  most likely to matter checked against the original, found no further
  behaviour differences.
- **5.4.2** is the default frozen build; **5.4.1** and **5.3.2** are the
  fallbacks. A packaged tester release (0.1.0) built from 5.4.2 is being
  prepared.
- A player **cleared Yoshi Star Galaxy** and **completed Flip-Swap Galaxy to
  its star**; that recorded run is replayed as a regression check. Fluffy
  Bluff's Odyssey moves and Star Bit mods were player-confirmed. A headless
  sweep boots and runs **252 of 260** galaxy scenarios (the rest are
  non-gameplay scenes or unreachable in play).
- Coverage of free movement and mission completion is limited; this is not a
  full-game completion claim. **Switch 2 Pro Controller** support is
  experimental and not yet confirmed on hardware. There is no native Windows
  or Linux port.

Progress comes from the objdiff report for main at batch 70 (`cfb9c299`),
the same report used for the treemap. Play coverage and build status were
checked against team reports and frozen-build receipts; [PLAY.md](PLAY.md)
describes the launcher.

## Try it (testers)

A packaged **Astral Italian.app** for Apple Silicon Macs is built from frozen builds; see the [tester guide](docs/TESTING.md)
for requirements, first launch, controls and how to report bugs. The app extracts the game from **your own** disc image
on first run; no game data is distributed.

## Play it (developer launcher)

Read [PLAY.md](PLAY.md), then double-click
[Play Astral Italian.command](Play%20Astral%20Italian.command). The launcher
uses your privately extracted disc, normally
`~/MacioGalaxy2-gamedata/SB4E01-extracted/`.

Choose **d** for 5.4.2 (the default), **e** for 5.4.1, **p** for 5.3.2, or
**b** for other available frozen builds. Choose a galaxy or the title screen;
Odyssey movement and audio have menu toggles. The launcher requires an app
with a `FROZEN.txt` marker under `~/MacioGalaxy2-play/`.

Normal saves, settings, mods and controls live in `~/MacioGalaxy2-play/user/`
(saves under `NAND/`); run logs are in `user/logs/`. The optional unlocked
save uses `~/Library/Application Support/AstralItalian-unlocked/`.
Builds can share saves and graphics caches. See the
[persistent-state inventory](native/play/PERSISTENT-STATE.md) for exact paths.

## Mods and menus

Open the Home menu with **grave/backtick (`)** or **F1** to change controls,
enable mods, or use **Level Select**. Optional mods are off until enabled:

- **Odyssey movement:** long jumps, dives, rolls, wall jumps and more.
- **Cappy:** enable both Odyssey movement and Cappy, then reload the stage.
  **R** throws the cap; cap contact can flip Flip-Swap panels. A cap
  bounce restores the airborne Spin (5.3.2 and later).
- **Collect-all-Star-Bits (G):** pulls eligible Star Bits in the current scene to Mario.
- **Shoot-nearest (V):** fires a Star Bit at the nearest eligible visible enemy in range.

Details: [Mods](native/MODS.md), [Controls](native/CONTROLS.md),
[Home menu](native/HOME_MENU.md), [Level Select](native/LEVEL_SELECT.md).

## Controls

| Action | Default keyboard / mouse |
| --- | --- |
| Move / jump / spin | WASD / Space / F |
| Crouch; ground pound in air | Shift |
| Star Pointer / shoot / hold pointer target | Mouse / left click / right click |
| Rotate / recenter camera | Q and E / C |
| Start / pause / Home menu | Return / Escape / grave (`) or F1 |
| Capture a playtest problem | M or F9 |

Xbox and PlayStation controllers use SDL gamepad input. Switch Pro layouts
use **Nintendo button labels**, with separate Galaxy and Odyssey layouts;
physical-pad validation remains limited. Switch 2 Pro support is experimental.
See [CONTROLS.md](native/CONTROLS.md) for mappings, power-ups and remapping.

## How it is built

The Wii decompilation uses **decomp-toolkit (dtk), Ninja and objdiff**.
The upstream dependency, build and diffing instructions now live in
[Building the decompilation](docs/building-decomp.md).

The native port uses CMake and native platform replacements with
`GALAXY_NATIVE` hooks. [native/README.md](native/README.md) describes the
build setup and native platform layer. For how the game itself is structured, see
[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md). Use [PLAY.md](PLAY.md) for current play instructions.

## How the work is done

AI agent teams work in separate worktrees with proof gates. Native hook
changes go through [hookproof](tools/team/hookproof.sh) and
[proofland](tools/team/proofland.sh): clean before/after Wii object builds
must be identical, with a perturbation control to check the proof itself.
The [crossed-dispatch oracle](tools/team/dispatchcheck.py) separately checks
bindings against the original DOL, catching wrong vtable or nerve targets
that byte matching can miss. Dispatch fixes have their own image-proof path.
[land.sh](tools/team/land.sh) serializes landings, rebases, checks and pushes;
its docs-only path skips the Wii build.

See the [team playbook](docs/team/TEAM.md) and
[dispatch/proof guide](docs/team/DISPATCH.md).

## Docs map

| Document | Purpose |
| --- | --- |
| [How SMG2's code works](docs/ARCHITECTURE.md) | Architecture guide with diagrams: layers, frame loop, actors, nerves, sensors, levels, Mario |
| [Tester guide](docs/TESTING.md) | Install the packaged app, controls, reporting bugs |
| [Releasing](docs/RELEASING.md) | How tester packages are built (maintainers) |
| [PLAY.md](PLAY.md) | Start playing; build choices and known issues |
| [Full play guide](native/play/PLAY.md) | Launcher options and troubleshooting |
| [Persistent state](native/play/PERSISTENT-STATE.md) | Saves, settings, logs and caches |
| [Controls](native/CONTROLS.md) | Keyboard, mouse, controllers and remapping |
| [Mods](native/MODS.md) | Optional movement and gameplay helpers |
| [Home menu](native/HOME_MENU.md) | Native menu and controls page |
| [Level Select](native/LEVEL_SELECT.md) | Galaxy/mission requests and their limits |
| [Wii build guide](docs/building-decomp.md) | Dependencies, building and objdiff |
| [Native port](native/README.md) | Architecture and build reference (historical status text) |
| [Team playbook](docs/team/TEAM.md) / [Dispatch checks](docs/team/DISPATCH.md) | Agent workflow and proof gates |

Credit to Garigari and Petari, and to the [doldecomp team](https://github.com/doldecomp),
[Zelda TP](https://github.com/zeldaret/tp) and [OGWS](https://github.com/doldecomp/ogws/tree/master)
for code and headers used by this project.

Native app builds bundle libusb 1.0.30 under LGPL-2.1-or-later, from the exact source
release https://github.com/libusb/libusb/releases/download/v1.0.30/libusb-1.0.30.tar.bz2.
The library is a separate dynamically loaded file the user can replace at
`Petari.app/Contents/Frameworks/libusb-1.0.0.dylib`. Its licence and notice ship in
`Contents/Resources/licenses/`; see [third-party notices](native/licenses/THIRD_PARTY_NOTICES.txt).

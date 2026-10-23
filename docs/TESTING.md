# Testing Astral Italian

Astral Italian is a native Apple Silicon Mac port of Super Mario Galaxy 2, built on a community decompilation. It is an
early tester build: parts of the game are well covered, others are not. This page tells you how to install it, play it and
report what goes wrong.

## What you need

- A Mac with **Apple Silicon** (M1 or newer). Intel Macs are not supported.
- **macOS 14 or newer** (each release states its exact minimum in its notes).
- About **6 GB** of free disk space (the app copies about 1.4 GB of game files out of your disc image).
- **Your own copy of Super Mario Galaxy 2, USA version (game ID SB4E01).** No game data is included or available here.
  Accepted: a disc image (`.iso`, `.wbfs`, `.rvz`, `.wia`) or an already extracted folder that holds `files/` and `sys/`.
  Other regions and revisions are refused with a message saying so.
- Optional: a game controller (Xbox or PlayStation pads work; Switch Pro layouts are untested; the Switch 2 Pro Controller
  does not work yet).

## Install

1. Download `AstralItalian-<version>.zip` from the [Releases](https://github.com/classicrob/Astral-Italian/releases) page and
   open it. It contains `Astral Italian.app`, `READ ME FIRST.txt` and `Collect Bug Report.command`.
2. Drag **Astral Italian.app** to your Applications folder.
3. **First launch:** the app is not signed with an Apple Developer ID yet (that costs money; it may come later), so macOS
   blocks it once:
   - **macOS 15 and newer:** open the app, click *Done* on the warning, then *System Settings > Privacy & Security*, scroll to
     "Astral Italian was blocked" and click **Open Anyway**.
   - **macOS 14:** right-click (or Control-click) the app, choose **Open**, then **Open**.
   - **Terminal, any version:** `xattr -dr com.apple.quarantine "/Applications/Astral Italian.app"`
4. The app asks for your game disc. Choose the disc image (or the folder). It checks that it is the right game and extracts
   what it needs to `~/Library/Application Support/Astral Italian/Game Data/` (about 20 seconds to a few minutes). You do this
   once; the choice is remembered. *Astral Italian > Choose Game Disc...* (or launching with `--setup`) lets you change it.

Your saves, settings, controls and logs live in `~/Library/Application Support/Astral Italian/`. It never touches the user
folder of any other Super Mario Galaxy port.

## Playing

| Action | Default keyboard / mouse |
| --- | --- |
| Move / jump / spin | WASD / Space / F |
| Crouch; ground pound in air | Shift |
| Star Pointer / shoot / hold pointer target | Mouse / left click / right click |
| Rotate / recenter camera | Q and E / C |
| Start / pause / Home menu | Return / Escape / grave (`) or F1 |
| Capture a problem | M or F9 |
| Quit | Cmd+Q |

The **Home menu** (` or F1) is where you remap controls, switch mods on and off, pick a galaxy with Level Select, and change
**Display** options (fullscreen, window size, VSync; VSync takes effect after a restart). Full lists:
[Controls](../native/CONTROLS.md), [Home menu](../native/HOME_MENU.md), [Mods](../native/MODS.md).

**Mods** are all off until you enable them in the Home menu:

- *Odyssey movement*: long jump, dive, roll, wall jump and more.
- *Cappy* (needs Odyssey movement; reload the stage after enabling): R throws the cap.
- *Collect-all-Star-Bits* (G) pulls nearby Star Bits to Mario.
- *Shoot-nearest* (V) fires a Star Bit at the nearest enemy in range.

The first time a new effect appears the game can hitch for a moment while shaders compile; that settles down.

## What to test

Anything you like: the title screen and file select, the Starship, the World 1 map, Flip-Swap, Fluffy Bluff, Yoshi Star. We
mostly want to hear about crashes, hangs, wrong visuals or sound, controls that feel off, and anything that differs from the
original game. Coverage of the full game is limited; do not expect every galaxy to work yet.

## Reporting a problem

1. **Right after it happens, press M (or F9).** The game saves the last 5 seconds of your inputs and Mario's movement, his
   state at that moment and a screenshot. Nothing shows on screen; keep playing.
2. Quit the app, then double-click **Collect Bug Report.command** (from the download). It bundles the newest logs, captures,
   crash reports and session tape plus basic system info into a zip on your Desktop. Nothing is uploaded.
3. [Open an issue](https://github.com/classicrob/Astral-Italian/issues/new/choose), say what you expected and what happened,
   and attach the zip. If a zip is too big for GitHub, say so and describe it; we will find another way.

Where things are, if you want to look yourself (`~/Library/Application Support/Astral Italian/`):

| Folder | What |
| --- | --- |
| `logs/` | one log per run (the newest 20 are kept) |
| `Captures/` | the M/F9 captures |
| `Tapes/` | the exact record of recent sessions, so we can replay a problem frame for frame (the newest 10 are kept; about 150 MB per hour; set `ASTRAL_TAPE=off` to disable) |
| `Crashes/` | crash reports written by the game; macOS also writes its own to `~/Library/Logs/DiagnosticReports/` |
| `NAND/` | your save data |

The logs and system info contain your Mac model, macOS version and the file paths on your Mac (which include your user name).
They contain no part of the game's data.

## Removing it

Delete `Astral Italian.app` and the folder `~/Library/Application Support/Astral Italian/` (your saves are in there; copy
`NAND/` somewhere first if you want to keep them) and `~/Library/Caches/AstralItalian` (shader cache).

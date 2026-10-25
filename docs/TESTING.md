# Testing Astral Italian

> **No game assets are included, ever.** Astral Italian is a native Apple Silicon Mac port of Super Mario Galaxy 2 built on a
> community decompilation. You **import your own copy of the game (USA disc image, game ID SB4E01)** the first time you open the
> app; it never leaves your Mac and is not part of any download here.

It is an early tester build: parts of the game are well covered, others are not. This page tells you how to install it, play
it and report what goes wrong. If you know this game well, the [expert tester brief](#expert-tester-brief-what-felt-different)
below is the most useful thing you can do for us.

## What you need

- A Mac with **Apple Silicon** (M1 or newer). Intel Macs are not supported.
- **macOS 15 (Sequoia) or newer.**
- About **6 GB** of free disk space (the app copies about 1.4 GB of game files out of your disc image).
- **Your own copy of Super Mario Galaxy 2, USA version (game ID SB4E01).** No game data is included or available here.
  Accepted: a disc image (`.iso`, `.wbfs`, `.rvz`, `.wia`) or an already extracted folder that holds `files/` and `sys/`.
  Other regions and revisions are refused with a message saying so.
- Optional: a game controller (Xbox or PlayStation pads work; Switch Pro layouts are untested with a physical pad; Switch 2 Pro
  Controller support is experimental and not yet confirmed on hardware).

## Install

1. Download `AstralItalian-<version>.zip` from the [Releases](https://github.com/classicrob/Astral-Italian/releases) page and
   open it. It contains `Astral Italian.app`, `READ ME FIRST.txt` and `Collect Bug Report.command`.
2. Drag **Astral Italian.app** to your Applications folder.
3. **First launch:** the app is not signed with an Apple Developer ID yet (that costs money; it may come later), so macOS
   blocks it once:
   - **Open Anyway:** open the app, click *Done* on the warning, then *System Settings > Privacy & Security*, scroll to
     "Astral Italian was blocked" and click **Open Anyway**.
   - **Terminal, any version:** `xattr -dr com.apple.quarantine "/Applications/Astral Italian.app"`
     (`Collect Bug Report.command` is blocked the same way the first time; use the same trick on it, or right-click > Open.)
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
**Display** options (fullscreen, window size, VSync (Auto by default: on in a window, off in fullscreen; On/Off force one mode), Ubershaders; VSync and Ubershaders take effect after a restart). Full lists:
[Controls](../native/CONTROLS.md), [Home menu](../native/HOME_MENU.md), [Mods](../native/MODS.md).

**Mods** are all off until you enable them in the Home menu:

- *Odyssey movement*: long jump, dive, roll, wall jump and more.
- *Cappy* (needs Odyssey movement; reload the stage after enabling): R throws the cap.
- *Collect-all-Star-Bits* (G) pulls nearby Star Bits to Mario.
- *Shoot-nearest* (V) fires a Star Bit at the nearest enemy in range.

**The first launch of a new version shows a "Preparing graphics… N%" screen for a few seconds** (about 5 seconds longer than usual
on an M1, about 2 on an M4) before the title appears: the game prepares the graphics for your Mac. Later launches skip it. Let it
finish; the window stays responsive.

Rarely, the first time a new effect appears the game can still hitch for a moment while a shader compiles; that settles down. Known limits are listed in the release notes.

**Audio crackle:** the sound can occasionally crackle or drop out for a moment, sometimes in short bursts (seen mostly on M1-class
Macs); we are investigating. If you hear it, **press M (or F9) right after**, then write what you heard in the note when you run Collect Bug
Report.command (which galaxy, whether it was during a loud scene or while loading, headphones or speakers). The capture and the
session tape let us line your report up with what the game was doing.

## What to test

Anything you like: the title screen and file select, the Starship, the World 1 map, Flip-Swap, Fluffy Bluff, Yoshi Star. We
mostly want to hear about crashes, hangs, wrong visuals or sound, controls that feel off, and anything that differs from the
original game. Coverage of the full game is limited; do not expect every galaxy to work yet.

## Expert tester brief: what felt different?

If you have played Super Mario Galaxy 2 a lot, your hands know things we cannot measure. We do not need "any bugs"; we need
**"what felt different from the original?"** Play the same moment on the Wii (or Dolphin) if you can, then here (for the Odyssey mod moves, compare with Super Mario Odyssey), and tell us
which one felt different and how. "Feels floatier", "the spin is a hair late", "the camera trails me" are exactly the right
level. You do not need numbers. Press **M** (or F9) *right after* a moment that felt off: the game then keeps the last 5 seconds
of what you did, and the exact tape of the session lets us replay it frame by frame.

Try these, in roughly this order, and answer what you can:

| Area | What to try | What to tell us |
| --- | --- | --- |
| **Spin** | Spin on the ground, in the air, and while running, then spin into a Star Bit or an enemy. | Does the spin start at the moment you press? Is the airborne spin height/hover the same? Is the cooldown (when you can spin again) the same? |
| **Long jump** (the Odyssey movement *mod*, off until you enable it in the Home menu; run, then Shift, then Space) | A long jump along flat ground; a long jump off a ledge; dive and roll out of it. | These moves are not in the original game: compare with Super Mario Odyssey. Are the distance and arc what you expect? Does the landing keep your speed the way Odyssey does? |
| **Jumps and wall kicks** | Triple jump, backflip, side flip, and wall kicks against a wall: timing window, height, direction after the kick. | Is the wall-kick window the same? Does Mario stick or slide differently? |
| **Camera** | Run in circles, turn quickly, enter and leave narrow spaces, use Q/E and C to recenter. | Does the camera lag behind more, or snap, compared with the original? Does it ever fight you or clip through geometry? |
| **Star Pointer** | Collect Star Bits with the mouse, shoot them at enemies, hold the pointer target (right click). | Is the pointer position where you expect (no offset, no lag)? Is the pickup/shoot timing the same? Does it feel right at the screen edges and at different window sizes? |
| **Launch stars and cannons** | Fly through several launch stars; fire cannons. | Is the arc, speed and landing spot the same? Is the pointer-to-aim feel right? |
| **Water** | Swim, dive, surface, spin-swim, enter and leave water in a galaxy that has it. | Swimming speed and turning, the surface bob, camera behaviour, any odd visuals or sound. |
| **Frame rate / 120 Hz** | Play a while; if you have a 120 Hz (ProMotion) display, also watch pans and fast turns. | Does motion look smooth and evenly spaced, or stuttery/doubled? Any hitch when something new appears (shaders compiling)? Does it feel like 60 or like 120? |
| **Sound and music** | Listen through a galaxy and a world map. | Missing or doubled sounds, music that loops wrong or cuts, anything delayed relative to the picture. |

Also welcome: anything that is not in the original (a crash, a hang, a wrong colour or missing object, a place you cannot reach
that you can reach on the Wii) and anything you only notice when you compare side by side.

### Sending it

1. Press **M** (or F9) when it happens, and keep playing or quit normally.
2. Quit, then run **Collect Bug Report.command** from the download. It asks for one or two sentences; write what felt different
   and where (galaxy and mission if you know). It makes a zip on your Desktop with your newest session tape, build version,
   logs, captures and crash reports. **README-PRIVACY.txt** inside lists exactly what is in it; no saves and no game files.
3. Send the zip to the developers (cloud-drive link or direct message works best; GitHub issues take files up to 25 MB, and a
   second smaller zip *without the tape* is made automatically when the full one is large), or
   [open an issue](https://github.com/classicrob/Astral-Italian/issues/new/choose) and attach it or its link.


## Reporting a problem

1. **Right after it happens, press M (or F9).** The game saves the last 5 seconds of your inputs and Mario's movement, his
   state at that moment and a screenshot. Nothing shows on screen; keep playing.
2. Quit the app, then double-click **Collect Bug Report.command** (from the download). It bundles the newest logs, captures,
   crash reports, your latest exact session tape and the build identity into a zip on your Desktop (see
   README-PRIVACY.txt inside it). Nothing is uploaded.
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

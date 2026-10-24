# Home menu look

The native Home menu (F1 or backtick; `native/home_menu/`) draws with the game's own
lettering and button icons, styled after the game's pause screen.

## Fonts from the disc

At startup the app reads `files/LayoutData/Font.arc` from the player's disc
(`petari/galaxy_font.hpp`): a Yaz0-compressed RARC of NW4R fonts (`.brfnt`).
The glyph sheets are decoded (I4, IA4, RGB5A3 and the other GX formats the
fonts use) and added to the Dear ImGui font atlas (`installGalaxyFonts`):

| Font | Used for |
| --- | --- |
| MessageFont26 | All menu text: messages, buttons, the controls table, the footer, the title hint, the My Progress badges |
| MenuFont64 | Page titles (the outlined lettering of the game's banners) |
| PictureFont | Colored icons embedded in text: the A, B, C, Z, +, -, 1, 2 and HOME buttons, the + Control Pad, the Control Stick, the Wii Remote |

Picture character `c` is exposed as U+E000 + (c - 0x20); `home_menu.hpp`
has the UTF-8 constants (`PETARI_ICON_A` and so on). Icons keep their own
colors in tinted text.

Nothing from the disc is stored in the repository. Without the file (no disc,
CI) the app logs `petari: menus use the built-in font: ...` and the menus use
ImGui's built-in font, with icons shown as `(A)`, `(+)` and so on. The layout
is the same either way.

## Style

- The main list and the Restart/Quit confirmations use the game's pause-screen
  layout: translucent teal bands across the top (title and message) and the
  bottom (the footer), with the buttons over the scene between them.
- The other pages dim the game in deep teal and sit on a panel: a teal
  gradient with a cyan rim and a faint inner line.
- Buttons: the game's white-to-blue capsules with a cyan rim; the focused
  one turns gold and glows.
- Text has a soft drop shadow; titles are white MenuFont64.
- A footer band along the bottom of the image names the buttons that work on
  the page, with their icons and current keys ("(A) Space: choose  (+)
  Escape: back  (HOME) F1: close"; Display, Level Select, Odyssey camera and My
  Progress add the + Control Pad's "change").
- Sizes follow the game image's height, so the pages read the same at any
  window size; long messages wrap to two lines.

## Controls page

The Controls page is a remapper (native/CONTROLS.md "Changing controls"): a
scrolling table of every action, with its icon, two keyboard/mouse slots and
two controller slots as key caps, and a scrollbar on the right (arrow
buttons, a track that pages when clicked, and a draggable capsule thumb sized
to the shown fraction). The focused slot is a gold capsule that
pulses "Press..." while it waits for an input. Shared inputs are outlined in
orange, and changed actions are named in gold. The buttons below are Clear
slot, Reset action, Reset all and Back, or Swap, Keep both and Cancel for a
conflict. The page logic is in `home_menu.cpp` (`Menu::updateRemap`) on a
`ControlsBackend`. The bridge's backend (`hbm_bridge.cpp`, `InputControls`)
applies changes to the live bindings and saves `controls.txt`.

## Checking the look

- Offline, every page to PNG (no game, any size):
  `cmake --build build/macos-gx --target petari_home_menu_render`, then
  `build/macos-gx/native/home_menu/petari_home_menu_render <out-dir> --font
  build/game-data/SB4E01/files/LayoutData/Font.arc [--background bg.ppm]
  [--size 1280x720] [--wide 0|1]`. It rasterizes ImGui's draw data in
  software.
- In the game: the stage smoke with `PETARI_STAGE_HOME_TOUR=1` opens the
  Home menu after its pause checks and visits every page, marking
  `home-<page>`; with `PETARI_SURFACE_DUMP=<dir>
  PETARI_SURFACE_DUMP_LABELS=home-,pause` the window images (overlay
  included) are written there. The game's own pause menu is marked `pause`.
  On Controls the tour remaps Cap throw to F (Spin's key) through a real
  capture and chooses Swap (`home-remap-capture`, `-conflict`, `-done`), so
  the run's `controls.txt` ends with `ModCappyThrow=Key:F,...` and
  `Shake=Key:R,...`. Before that it scrolls the list with two wheel notches
  (`home-scroll-wheel`) and drags the scrollbar's thumb to the bottom with the
  left mouse button (`home-scroll-drag`).

## Tests

`native_galaxy_font` (`native/tests/galaxy_font_tests.cpp`): Yaz0, RARC,
texture and RFNT decoding on synthetic data (including truncated and
malformed input), the ImGui atlas with the fonts installed (letters white,
icons in color, ImGui's own font kept first), a missing archive leaving the
built-in font, and, when the disc is present, the real Font.arc.
Both also run in the standalone build (`cmake -S native/home_menu -B
build/home-menu -DPETARI_SANITIZERS=ON`), which uses ImGui's stb_truetype
builder instead of FreeType. `native_home_menu` checks the overlay's geometry, colors and readability at
480 lines with the built-in font, and the remapper on a fake backend:
navigation and scrolling (the wheel, pages, LB/LT, the scrollbar's arrows,
track and thumb drag, clicks on the panel's empty space), capture, the conflict choices, Escape and the
10-second timeout, the refused Home change, the buttons (Reset all needs a
second A), the pointer, and a black-out cancelling a capture.

## Display page

Open the native Home menu with **F1** or **backtick (`)**, then choose
**Display**. Fullscreen toggles macOS desktop fullscreen (a fullscreen Space).
Window size cycles through **1280×720**, **1600×900**, **1920×1080** and
**2560×1440** with Left/Right or the confirm button; clicking advances it.
The size row is disabled in fullscreen. Sizes are requested window points;
macOS can constrain them to the usable screen area. The existing presentation
viewport keeps the game's aspect ratio and letterboxing.

VSync is saved for the **next application launch**. A changed value shows
“restart needed”; quit and reopen the app to apply it. **Restart from Title**
does not recreate the window or apply VSync. The default is **On** (Fifo
presentation) since 5.4: the game still paces itself from VI/display-link
retraces, but with VSync off (Immediate) a 120 Hz ProMotion panel put 16.3 % of
warm frames off the 60 fps cadence (visible judder) against 0.3 % with Fifo.
**Off** (Immediate) remains for anyone who wants the lowest latency. 5.4 saves
the choice as `vsync2`; the `vsync` key that 5.3 and earlier wrote (with the old
default Off) is ignored, so existing user folders get the new default. No pacing
or runtime surface-mode changes are made by this page. Developer launches using
`PETARI_PRESENT_MODE` still override the present mode.

**Ubershaders: On/Off** defaults to On (hybrid rendering). It reduces stalls while
new shaders compile. The preference applies after quitting and reopening the app;
Restart from Title does not apply it. A changed value shows “restart needed”.
Developer launches resolve `PETARI_UBERSHADER` first (`0` fully disables fallback
preparation and draws, `1` enables hybrid, `all` forces covered fallback draws),
then the saved preference, then default On. Boot logs the effective mode and source:
`[gx uber] mode=off|hybrid|all source=env|setting|default`.

Use Up/Down to choose a row, A/Space to activate, Left/Right to change,
Plus/Escape to go back, or HOME/F1 to resume. Mouse and controller input use
the same path as the existing menu pages; the footer names current bindings.
The main menu also points to **Controls** for the live key/button reference
and remapper. The default key table remains in `native/CONTROLS.md`.

`native/app/display_settings.{hpp,cpp}` loads `display.txt` in the selected
user directory before Aurora creates its window. Missing files retain the
defaults: windowed, 1280×720, VSync On, Ubershaders On. The text file contains
`fullscreen=0|1`, `size=0..3` (presets in the order above), `vsync2=0|1`, and `ubershaders=0|1`. Older files without the new key retain default On.
Out-of-range numeric preset indices are clamped; malformed known values reset
the load to defaults and produce an actionable message. Changes use a unique
sibling `display.txt.tmp-XXXXXX`, flush it and atomically rename it over the
old file. Failed saves are shown on the page. Remove `display.txt` while the
app is closed to restore defaults.

A thread-safe `DisplayBackend` queues changes from the game's menu thread;
`display_window.cpp` performs SDL operations on the host frame seam. It waits
for fullscreen/resize transitions to settle before accepting another change,
and observes fullscreen changes made through macOS. Fullscreen animations
can briefly pause presentation. Window mutations and persistence are tested
with a fake window; the actual macOS Spaces/Metal behavior still needs an
app run on supported build hardware.

Standalone tests: `native_display_settings` in `native/app` covers defaults,
parsing, clamping, atomic replacement/failures, queued host changes, OS
fullscreen changes, VSync/Ubershaders relaunch state and environment precedence. `native_display_menu` in
`native/home_menu` covers navigation, disabled rows, notices and ImGui drawing
at 480 lines in both aspects. The offline renderer writes `display.png`,
`display-restart.png` and `display-fullscreen.png`; create its output
directory before running it. The stage Home tour also marks `home-display`.

## Astral Italian __VERSION__ (tester build)

A native Apple Silicon Mac port of Super Mario Galaxy 2, built on the community decompilation. **It contains no game data. You need your own copy of the USA version (SB4E01)** as a disc image (.iso/.wbfs/.rvz) or an extracted folder; the app asks for it on first launch.

- Requires macOS __MINMACOS__ or newer on Apple Silicon.
- Source commit: `__COMMIT__`. Debug symbols are the separate `.dSYM.zip` asset (not needed to play).
- The app is **not notarized** yet: follow `READ ME FIRST.txt` (right-click Open, System Settings > Privacy & Security > Open Anyway, or the `xattr` command) the first time.
- **First launch of a new version shows "Preparing graphics… N%" for a few seconds** (about 5 s longer than usual on an M1 Max, 2 s on an M4 Max), then the title appears; later launches skip it. Much smoother first play on M1/M2-class Macs than earlier builds: no frozen menu frames and no multi-second pauses when a stage starts (M1 Max: worst menu frame about 0.1 s instead of 0.4-1.7 s; 0 stalls over 100 ms in 9 of 9 cold runs, 6 of 9 stalled before).
- **Display options** (Home menu: ` or F1, then Display): Fullscreen, window size, **VSync** (Auto by default: on while windowed for an even frame cadence, including on 120 Hz ProMotion panels, off in fullscreen for the lowest latency; or force On/Off) and **Ubershaders** (On by default: they remove the pauses when a new effect first appears; Off to compare). Changes to VSync and Ubershaders apply after quitting and reopening.
- Optional mods are off until enabled in the Home menu (` or F1): Odyssey movement, Cappy, collect-all-Star-Bits, shoot-nearest.
- Found a problem? Press **M** (or F9) right after it happens, run `Collect Bug Report.command`, and open an issue with the zip. Guide: [docs/TESTING.md](https://github.com/classicrob/Astral-Italian/blob/main/docs/TESTING.md).

### Known limits
- Not a full-game completion claim: free movement and mission coverage are partial. Switch 2 Pro Controller does not work yet; Switch Pro layouts are untested with a physical pad.
- On M1-class Macs the title screen can show intermittent bursts of slow frames on a relaunch (in our measurements 7-41 frames over 50 ms in the first 5 minutes, worst 70-600 ms); 5.4.1 shows the same intermittently, so it is not new in this build.
- Rare gameplay stalls remain: one 258 ms frame in the VsKoopaLv1 Bowser battle galaxy and one 111 ms frame on the first visit to Sky Station.
- The cold-start menu can still be slightly less even than 5.4.1 on some runs, though its worst frames are far better.
- A direct boot into World 8's map galaxy crashes (not reachable in normal play).
- Windowed play adds some input latency compared with fullscreen (VSync Auto turns VSync off in fullscreen).

SHA-256 of the zip: `__SHA256__`

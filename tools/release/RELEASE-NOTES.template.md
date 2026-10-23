## Astral Italian __VERSION__ (tester build)

A native Apple Silicon Mac port of Super Mario Galaxy 2, built on the community decompilation. **It contains no game data. You need your own copy of the USA version (SB4E01)** as a disc image (.iso/.wbfs/.rvz) or an extracted folder; the app asks for it on first launch.

- Requires macOS __MINMACOS__ or newer on Apple Silicon.
- Source commit: `__COMMIT__`. Debug symbols are the separate `.dSYM.zip` asset (not needed to play).
- The app is **not notarized** yet: follow `READ ME FIRST.txt` (right-click Open, System Settings > Privacy & Security > Open Anyway, or the `xattr` command) the first time.
- Optional mods are off until enabled in the Home menu (` or F1): Odyssey movement, Cappy, collect-all-Star-Bits, shoot-nearest.
- Found a problem? Press **M** (or F9) right after it happens, run `Collect Bug Report.command`, and open an issue with the zip. Guide: [docs/TESTING.md](https://github.com/classicrob/Astral-Italian/blob/main/docs/TESTING.md).

### Known limits
- Not a full-game completion claim: free movement and mission coverage are partial. Switch 2 Pro Controller does not work yet.
- First play in a new area can hitch while shaders compile.

SHA-256 of the zip: `__SHA256__`

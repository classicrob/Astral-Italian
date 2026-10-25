# Releasing Astral Italian (maintainers)

A tester release is a zip of a **frozen** app build plus its dSYM, built by `tools/release/make_release.sh`. Nothing here
publishes anything: a person reviews the draft on GitHub and presses Publish.

## What the release build must satisfy

1. **Freeze-grade app build on the server** (the M1 lacks clang 21/xxhash) of a source commit on `main`, frozen under
   `~/MacioGalaxy2-play/<build>/` with its `FROZEN.txt`.
2. **Deployment target 15.0 is the project default** (root `CMakeLists.txt` and every preset set `CMAKE_OSX_DEPLOYMENT_TARGET`),
   so the executable and everything built with it records `minos 15.0`; the M1 (macOS 15.1) is the oldest machine we test on.
   `make_release.sh --min-macos` **fails** if the executable or any bundled library records a higher minimum.
3. **Build with the `macos-arm64-release-portable` preset** (or the same two cache variables): it hides `/opt/homebrew` and
   `/usr/local` from `find_package`, so Aurora fetches and statically links abseil, fmt, xxHash, zstd, libpng and FreeType at the
   project's deployment target. Homebrew libraries are built for the OS they were installed on (the server's carry minos 26-27),
   cannot be bundled into a 15.0 app, and would otherwise be dylibs the player does not have. Check the executable with
   `otool -L`: only `/usr/lib` and `/System` paths, and the system `libc++` (never a Homebrew LLVM libc++).
4. `Info.plist` identity stays `dev.petari.Petari` / executable `Petari`; the release script sets version, source commit,
   `LSMinimumSystemVersion`, the games category and copies the license and third-party notices into the bundle.

5. **Audio: zero underruns on the functional gates** (from 5.4.3 on). The normal-route and full-star gate runs must run with
   real audio (CoreAudio reports present, as in the tape arms) and report **0 underrun frames**; any underrun fails the run, with no
   tolerance and no rerun-until-clean. Added 2026-10-25 (head): retained M1 runs of the 5.4.2 source (00221126) showed underruns that
   passed unnoticed because these gates did not check audio (normal-r3 108 frames; fullstar-r2 1,088; fullstar-r3 16,914 over 8
   reports), and 5.4.2's notes carry it as a known limit.

## Build the package

```sh
tools/release/make_release.sh --app ~/MacioGalaxy2-play/build5.x/Petari.app \
    --dsym <path to Petari.app.dSYM> --version 0.2.0 --commit <40-hex source commit> --min-macos 15.0
```

Output in `~/MacioGalaxy2-team/scratch-release/out/` (override with `--out`): `AstralItalian-<v>.zip`, `.dSYM.zip`,
`.sha256`, `.manifest.json` and `.notes.md`. The script:

- refuses an unfrozen app and a dSYM whose UUID does not match the executable;
- copies any remaining non-system dylib into `Contents/Frameworks` and rewrites install names to `@rpath` (`bundle_libs.py`;
  a no-op for a portable build, a safety net otherwise, and the scan still enforces the deployment target on what it copies);
- ad-hoc signs the bundle (Apple Silicon will not run unsigned code);
- runs `scan_bundle.py` on the bundle **and** the final zip: only Mach-O code, plists, SQLite shader seeds, text and icons are
  allowed; disc-format names, extensions and magic numbers (Wii/GC headers, Yaz0, RARC, WBFS, RVZ, ...), the game ID in
  non-code files, directories like `files/` or `sys/`, load paths outside the bundle and a too-high `minos` all fail the build.
  The scan is a screen, not a proof: the real guarantee is that only frozen build products go in.

## Check before uploading

- On the **oldest macOS you support** (the M1, macOS 15.1): `python3 -I tools/release/check_weak_imports.py <Astral Italian.app>` lists
  weak imports that macOS lacks (an API newer than the deployment target, linked weakly because the SDK is newer). It must
  print 0; anything listed is a call that jumps to address 0 on that macOS unless the source guards it. (libusb's `pipe2`,
  macOS 26+, crashed the first 15.0 build at startup; fixed by `ac_cv_func_pipe2=no` in `native/cmake/libusb`.)

- Unzip into a fresh folder and launch it on a Mac **without** Homebrew/dev tools (and the oldest macOS you can reach).
  Run the first-run flow with a real disc, play a few minutes, press M, run `Collect Bug Report.command`.
- `spctl --assess -v` will say "rejected" until the app is notarized; that is expected for the unsigned tester flow.

## Draft the GitHub release

```sh
tools/release/draft_release.sh ~/MacioGalaxy2-team/scratch-release/out/AstralItalian-0.2.0
```

Creates a **draft prerelease** with the zip, dSYM zip and checksum, using the generated notes. Edit the notes, then publish
by hand.

## Notarization (not run yet; needs an Apple Developer account)

`tools/release/sign_notarize.sh <zip>` signs with a Developer ID Application certificate and the hardened runtime
(`tools/release/entitlements.plist`), submits to `notarytool`, staples and rebuilds the zip. One-time:
`xcrun notarytool store-credentials astral-notary --apple-id ... --team-id ...`; then
`SIGN_IDENTITY="Developer ID Application: ..." NOTARY_PROFILE=astral-notary tools/release/sign_notarize.sh <zip>`.
After notarization, drop the first-launch instructions from `READ ME FIRST.txt` and `docs/TESTING.md`.

## Tests

`python3 -I tools/release/test_scan_bundle.py`

## Measurement caveats for the 5.4.x freezes (maintainer notes, not tester-facing)

- Server (M4 Max) measurement rows in the 5.4.1 freeze (m4-cold-stall 9/9) and the 5.4.2 M4 no-regression sets ran at `nice 5`,
  not `nice 0` (zsh `BG_NICE` renices background jobs). Both arms of each comparison had the same nice, so the comparisons stand.
  Later runs launch at `nice 0`, asserted by the harness. (Head-approved wording from the PORT lead; the frozen build is untouched.)
- Correction (2026-10-25) to the 5.4.2 freeze's M1 warm-relaunch limit ("37 title frames > 50 ms in the first 300 s vs 5.4.1's 3, max 150 ms"):
  that compared ONE 5.4.2 warm pair against a single unusually clean 5.4.1 pair. The 5.4.3(b) bisect (M1 Max, warm second title launch,
  first 300 s, nice 0, compile-free rows; SOL A, M1 m1g/campaigns/sola-543-warm-pacing-r1/) shows the burst is intermittent and present
  in 5.4.1 too: 5.4.1 (07223918) 30 frames > 50 ms / max 311 ms in one pair; 840ffdba 8 and 8 frames / max 167 and 72 ms;
  5.4.2 (00221126) 7 and 11 frames / max 70 and 72 ms (an earlier 5.4.2 control: 41 / 603 ms). The EFB-capture, GX-wait/delivery
  and GPU-timestamp source paths are byte-identical across the three builds; the cause needs GPU-side timestamps (5.4.3/5.4.4).
  Tester wording since then: "On M1-class Macs the title screen can show intermittent bursts of slow frames on a relaunch (in our
  measurements 7-41 frames over 50 ms in the first 5 minutes, worst 70-600 ms); 5.4.1 shows the same intermittently, so it is not
  new in this build." build5.4.2/FROZEN.txt is untouched (it records what was measured at the freeze). (Head-approved.)

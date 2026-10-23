# Releasing Astral Italian (maintainers)

A tester release is a zip of a **frozen** app build plus its dSYM, built by `tools/release/make_release.sh`. Nothing here
publishes anything: a person reviews the draft on GitHub and presses Publish.

## What the release build must satisfy

1. **Freeze-grade app build on the server** (the M1 lacks clang 21/xxhash) of a source commit on `main`, frozen under
   `~/MacioGalaxy2-play/<build>/` with its `FROZEN.txt`.
2. Configure it with `-DCMAKE_OSX_DEPLOYMENT_TARGET=<target>` (13.0, or the lowest that compiles). `make_release.sh --min-macos`
   **fails** if the executable or any bundled library records a higher minimum. Homebrew libraries built from source on a newer
   macOS carry that macOS as their minimum too: build/install those with the same `MACOSX_DEPLOYMENT_TARGET`, or use bottles.
   Check the executable's C++ runtime (`otool -L`): it must use the system `/usr/lib/libc++`, never a Homebrew LLVM libc++.
3. `Info.plist` identity stays `dev.petari.Petari` / executable `Petari`; the release script sets version, source commit,
   `LSMinimumSystemVersion`, the games category and copies the license and third-party notices into the bundle.

## Build the package

```sh
tools/release/make_release.sh --app ~/MacioGalaxy2-play/build5.x/Petari.app \
    --dsym <path to Petari.app.dSYM> --version 0.2.0 --commit <40-hex source commit> --min-macos 14.0
```

Output in `~/MacioGalaxy2-team/scratch-release/out/` (override with `--out`): `AstralItalian-<v>.zip`, `.dSYM.zip`,
`.sha256`, `.manifest.json` and `.notes.md`. The script:

- refuses an unfrozen app and a dSYM whose UUID does not match the executable;
- copies every non-system dylib into `Contents/Frameworks` and rewrites install names to `@rpath` (`bundle_libs.py`);
- ad-hoc signs the bundle (Apple Silicon will not run unsigned code);
- runs `scan_bundle.py` on the bundle **and** the final zip: only Mach-O code, plists, SQLite shader seeds, text and icons are
  allowed; disc-format names, extensions and magic numbers (Wii/GC headers, Yaz0, RARC, WBFS, RVZ, ...), the game ID in
  non-code files, directories like `files/` or `sys/`, load paths outside the bundle and a too-high `minos` all fail the build.
  The scan is a screen, not a proof: the real guarantee is that only frozen build products go in.

## Check before uploading

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

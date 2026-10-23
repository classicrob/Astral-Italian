# PadProbe M1 handoff — 2026-10-22

Probe source commit: `0668c66c7dc1` on `pad/sol-c`, based on `lead/pad`
`9bdec601`. No push or landing. macOS 15.1 / 24B5070a, arm64, Apple clang 16,
CMake 3.27.7, Ninja. All runs were local to this M1; no server commands.

Package: `/Users/roberthaisfield/MacioGalaxy2-team/scratch-pad/pad/dist/PadProbe.zip`
(about 1.1 MB), alongside the unpacked `PadProbe/`.

SHA-256:

```text
08fdd08b330db2335f18c1bfa90187938b2218189bf8cec5457620695b809d68
```

Actual checks:

- Standalone Release build succeeded, with `-Wall -Wextra -Werror` on the
  probe/core/selftest. Default `PadDependencies.cmake`, static SDL 3.4.10,
  bundled libusb 1.0.30, Aurora's two SDL patches and app patches 0001+0002.
  `dependency-paths.tsv` records the patch/archive hashes. The supplied scratch
  SDL/libusb trees were only read; the final dependencies were built separately.
- Removed SDL's generated `include-config-release/build_config/SDL_build_config.h`,
  then configured and built successfully using the lead's corrected shared
  verifier. `build.txt` shows its build-time check; no custom verifier remains.
- `ctest --test-dir .../build-sol-c --output-on-failure`: 1/1 PASS. Pure tests
  cover descriptor hex bytes/bounds/empty input, all 29 control rows, Nintendo
  face positions/extras, directional thresholds, and verdict layer precedence
  including an enumeration error versus an empty inventory.
- Final package with an **empty environment**, `padprobe -t 5`: exit 0,
  5.338 s including process startup, report 3,380 bytes, all eleven expected
  no-pad statuses, `NO_PAD`. See `../padprobe_nopad_20261022/`.
- Extracted **final zip**, same empty-environment check: exit 0, 5.205 s,
  3,418-byte report. The SDL-phase dyld path is inside the extracted zip's
  `PadProbe/Frameworks`, after the probe's independent dlopen was closed.
  See `../padprobe_zip_20261022/`. Extraction was scratch and is deleted.
- `padprobe -t 5 -a`, empty environment: exit 0, 5.023 s, 3,381-byte report;
  **SKIP real non-pad device proof**: libusb inventory was empty. `ioreg -p IOUSB`
  showed only the three host controllers. See `../padprobe_allusb_20261022/`.
  Other vendors' handles are never opened or claimed by the probe.
- Copied the final executable to an isolated directory **without libusb** and
  ran with an empty environment, `-t 1`: exit 0, stdout equals report,
  SDL phase says `not loaded`, summary/verdict `LIBUSB_LOAD_FAILED`.
  See `../padprobe_missinglib_20261022/`. Isolated executable is deleted.
- `lipo -archs`: executable and libusb are arm64. `otool -D`: libusb identity
  `@rpath/libusb-1.0.0.dylib`. Executable rpath `@executable_path/../Frameworks`.
  `otool -L` shows no libusb link dependency in the executable, and no Homebrew
  or scratch dylib dependencies. Both extracted executable and dylib passed
  `codesign --verify --strict`. Package includes SDL/libusb licenses/notices.

The **physical Switch 2 pad has NOT been tested**, nor have game behavior,
hotplug event delivery, mapping, sensors, or felt rumble. They require the desk
checklist in `native/PADPROBE.md`. The real non-pad USB robustness proof also
remains pending until someone attaches a device. No single issue blocked work
for 30 minutes.

Cleanup: removed the owned `build-sol-c` tree (100 MB), extracted zip (3.1 MB),
isolated missing-library executable (2.8 MB), and transient configure/build
logs after retaining bounded receipts. Approximately 106 MB freed. Retained
only the final zip, unpacked package, source and small receipts. Free disk
remains above 50 GB (about 231 GiB at verification).

Files introduced:

- `native/tools/padprobe/padprobe.c`
- `native/tools/padprobe/probe_core.c`
- `native/tools/padprobe/probe_core.h`
- `native/tools/padprobe/selftest.c`
- `native/tools/padprobe/CMakeLists.txt`
- `native/tools/padprobe/package.sh`
- `native/PADPROBE.md`
- `native/tests/padprobe_tests.py`
- Receipts in `native/tests/receipts/padprobe_{nopad,allusb,zip,missinglib,package}_20261022/`:
  the run directories contain `padprobe-report.txt`, identical `stdout.txt`,
  `stderr.txt`, and (except missinglib) `validation.txt`; the package directory
  contains this handoff, dependency manifest, selftest/build/toolchain/signature
  output, and executable/libusb Mach-O dependency listings.

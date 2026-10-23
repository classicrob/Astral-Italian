# Native Apple Silicon port (MacioGalaxy2 / SMG2)

MacioGalaxy2 ports the Garigari Super Mario Galaxy 2 decompilation for the USA
`SB4E01` disc to native macOS arm64. **Status: skeleton; nothing boots yet.**
The native platform, resources, input, memory and CMake scaffolding were imported
from the SMG1 MacioGalaxy port. Game/library ABI hooks and SMG2 behavior still need
adaptation. Component compilation is separate from game compatibility.

Use `GALAXY_NATIVE` for native-only branches. Existing `PETARI_*` environment
variables, CMake targets, the `petari` namespace and app names retain their imported
names for now. Local extracted assets belong in `build/game-data/SB4E01`; no game
data is included. Do not use the SMG1 launch and smoke instructions as SMG2 validation.

From the repository root, with Xcode Command Line Tools and CMake 3.25 or newer:

```sh
nice -n 10 cmake --preset macos-arm64
nice -n 10 cmake --build build/macos-arm64 -j 6 --target \
  petari_host_runtime petari_resource_formats petari_jpa_tests petari_kcl_tests \
  petari_j3d_animation_tests petari_j3d_model_tests petari_cappy_tests petari_odyssey_move_tests
ctest --preset macos-arm64 -R '^native_(jpa|kcl|j3d_animations|j3d_models|cappy|odyssey_move|source_encoding)$'
```

The preset enables AddressSanitizer and UndefinedBehaviorSanitizer. The full
component build currently needs game/library hooks; build individual targets and
run only their available CTests until the hook transfer is complete. Source lists
and first compiler failures are recorded in the port team's Sol A `TARGETS.md`.
The game object target remains explicit and excluded from the default build.

**Editor deferred:** `native/editor` and its CMake links were deliberately omitted.
Imported editor-facing headers, tools and input interfaces remain as reference.
SMG1 native-helper tests and pipeline seed tests without their source/data inputs
are deferred in CMake. See [STUBS.md](STUBS.md) for SMG2 systems needing adaptation.

`MarioFaceShipGalaxy` is confirmed by `src/Game/System/GameSequenceFunction.cpp`
as the Starship hub. The imported Observatory routes, dome mission selector,
SMG1 stage catalogue and story/gameplay smokes need SMG2-specific data and flows;
renaming those stages alone would not make the tests valid.

## Imported SMG1 reference (unverified for SMG2)

The following material describes the donor port's architecture and historical SMG1
results. Pass counts, gameplay claims, asset names and launch procedures below are
reference only and do not describe an SMG2 run. Paths in commands now use `SB4E01`.

## Frame pacing and the display clock

The Wii's video interface (VI) runs at 59.94 Hz; a Mac display refreshes at 60 Hz
(or a multiple, such as a 120 Hz ProMotion panel). On a fixed 59.94 Hz timer the
game's frames drift against the display, and every 8 to 17 seconds a burst of frames
was shown for one or three refreshes instead of two (visible judder).

By default the VI retrace clock therefore follows the display:

- A display link from the game window tells VI when the display refreshes. VI then
  runs at the display's 60 Hz frame (16.667 ms per field). **The game runs at
  60.00 Hz, 0.1% faster than on a Wii.** VI also keeps each frame a fixed time after
  a display refresh, so every frame is shown for the same number of refreshes.
- Audio is unaffected: it follows the audio device's clock, as before. Movies show a
  video frame one extra time when the picture gets more than 25 ms ahead of the
  sound. That happens about once every 17 seconds of movie.
- VI falls back to its own 59.94 Hz timer while the refresh grid is unusable, and
  follows the display again once it is steady (about half a second):
  - the display's refresh rate is not a multiple of 60 Hz (for example 144, 165, 100
    or 75 Hz);
  - the refresh rate keeps changing (a variable-refresh display), or changes because
    the window moved to another display;
  - the window is hidden, minimised or fully covered (the display link stops);
  - macOS before 14, where the window has no display link.
- `PETARI_VI_CLOCK=timer` keeps the fixed 59.94 Hz timer.
  `PETARI_VI_PHASE_TARGET_MS` (default 2.5) sets how long after a refresh a frame
  should reach the present, for measurement only.

The log shows the mode at start and every change:

```
PETARI VI: retrace clock mode display (follows a steady 60 Hz-multiple display; PETARI_VI_CLOCK=timer opts out)
[vi clock] display link 60.00 Hz: a multiple of 60 Hz
PETARI VI: retrace clock locked to the display at retrace 30 (field 16.6667 ms)
PETARI VI: retrace clock on its own timer at retrace N (field 16.6833 ms): <reason>
```

With frame statistics on (`PETARI_FRAME_STATS=1`, `PETARI_FRAME_CSV` or `PETARI_TRACE_BOOT=1`), the exit
report also prints a `[vi clock] display:` line with the fields locked to the
display, the fields on the timer, and the lock and unlock counts. The design,
measurements and tests are in `native/include/petari/present_pacing.hpp` and
`native/tests/present_pacing_tests.cpp`.

## Build the native core

Requires Apple Silicon, Xcode Command Line Tools, and CMake 3.25 or newer.
No game assets, Wii compiler, Rosetta, or emulator are needed for these tests.
Run from the repository root:

```sh
cmake --preset macos-arm64
cmake --build --preset macos-arm64
ctest --preset macos-arm64
```

This builds `build/macos-arm64/libpetari_core.a` and
`build/macos-arm64/petari_core_tests` for arm64, with AddressSanitizer and
UndefinedBehaviorSanitizer enabled. Tests remain active in Release builds:

```sh
cmake --preset macos-arm64-release
cmake --build --preset macos-arm64-release
ctest --preset macos-arm64-release
```

The library compiles Petari's original Nerve/Spine/ActorState code, JMath and trig
tables, RNG, audio sequence reader, and the portable portions of the SDK math
code. Native replacements supply the PowerPC vector, affine matrix, quaternion,
and scalar-intrinsic operations. Tests cover state transitions, RNG overflow,
in-place math, inverse composition, projection conventions, table endpoints,
unaligned big-endian reads, and audio sequence call/return/loop behavior.

`GALAXY_NATIVE` keeps the native ABI changes separate from the matching Wii build:
`u32`/`s32` remain 32 bits, host pointers remain 64 bits, and `nullptr` and
`override` retain their C++ meanings. Native code uses Apple's standard library
instead of the original MSL C runtime. The original paired-single assembly remains
in its source files behind conditional compilation.

The scalar math replacements are not bit-exact emulations of PowerPC reciprocal
estimates or paired-single rounding. Floating-point behavior still needs gameplay
comparison with the original game. The matching Wii build has not been validated
here.

## GX/Metal integration probe

This optional development executable renders a rotating textured triangle or a disc model through
Aurora's native GX implementation and Metal. The transformation comes from
Petari's `JMAEulerToQuat` and native matrix path. It is a renderer integration
test, not a game launcher.

Vertex submission uses Petari's original GX declarations with native function
calls replacing Wii FIFO writes. Native texture and palette handles have the
storage required by Aurora. The `native_gx_abi` test compares layouts and selected
enum values across the two header sets, checks texture initialization for buffer
overwrites, and checks the BP decoder's legacy texture/palette address resolution.
The probe alternates little- and big-endian indexed vertex arrays and exercises
real J3D model loading and drawing. A corrected 120-frame Mario run on M4 Max
Metal passed with repeated scene-heap destruction/recreation. GPU readback found
31,797 non-background pixels with matching depth coverage in its first frame;
two captures in that frame preserved the image across a mid-frame submission.
The last frame submitted 11 draws, 58,014 vertex bytes and 29,268 index bytes.
Earlier runs counted draw submissions but omitted GXInit in the probe, so those
runs did not establish visible geometry. The corrected probe initializes GX.
A subsequent four-frame run delivered two real GX draw-sync callbacks that read
their ticket-specific color/depth snapshots, and its mid-frame GXDrawDone returned
before end_frame. Pixel checks establish nonempty output and capture preservation, not visual
fidelity; the window has not yet been visually inspected.

Build-time patches to the pinned Aurora BP decoder translate texture addresses
through the game memory arenas and resolve palette TMEM slots. A second patch
removes Aurora's placeholder VI exports, retaining its window helpers while the
native platform supplies real retrace timing. Patch anchors fail configuration
if the backend source changes unexpectedly. The VI bridge forwards mode changes;
Draw-sync events now use the platform interrupt thread, and mid-frame capture
submission is tested on Metal. XFB selection, blanking, dimming, and the image
rectangle are connected to the game frame loop. Focused Metal tests verify
display-copy snapshots and presentation; full-game behavior remains unverified.

Requires SQLite 3.37 or newer. On a Homebrew installation:

```sh
brew install cmake sqlite
cmake -S . -B build/macos-gx \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DPETARI_BUILD_GX_PROBE=ON \
  -DCMAKE_PREFIX_PATH="$(brew --prefix sqlite)"
cmake --build build/macos-gx --target petari_gx_probe -j 6
./build/macos-gx/native/gx/petari_gx_probe.app/Contents/MacOS/petari_gx_probe --frames 120
```

Pass `--model build/game-data/SB4E01/files/ObjectData/Kuribo.arc` to exercise
the real J3D model path. Omit `--frames` to leave the window open. Run from the repository root so the
probe's configuration and GPU caches stay under `build/gx-probe-state`.
The build pins Aurora to `08122911e8621acb7ded6563813b264bec1494b5` and downloads
its dependencies. A graphical macOS session and Metal device are required.
The development executable may link Homebrew libraries; it is not a self-contained
distributable `.app` bundle.

Aurora is MIT licensed. Its source and build instructions are at
[encounter/aurora](https://github.com/encounter/aurora). Keep its license and the
licenses of its dependencies when packaging a future application.

### Controllers: bundled libusb

Native app builds include libusb 1.0.30 (LGPL-2.1-or-later) from
https://github.com/libusb/libusb/releases/download/v1.0.30/libusb-1.0.30.tar.bz2.
The library is a separate dynamically loaded file the user can replace:
`Petari.app/Contents/Frameworks/libusb-1.0.0.dylib`. SDL tries this bundle path
before its normal library name. No Homebrew libusb is needed. Its COPYING and
THIRD_PARTY_NOTICES.txt ship in `Contents/Resources/licenses/`.

The asset-free loader proof uses the same dependency and packaging rules:

```sh
cmake -S native/tools/libusb_load_check -B build/libusb-check -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/libusb-check --parallel 4
ctest --test-dir build/libusb-check --output-on-failure
env -i build/libusb-check/check.app/Contents/MacOS/check --expect-loaded
```

`native/tests/libusb_bundle_tests.py` checks the pins and applies every SDL patch
to a fresh release extraction. Pass `--sdl-archive /path/to/release-3.4.10.tar.gz`
to use a local archive. With `--executable` and `--library`, it also verifies
Mach-O dependencies and runs clean-environment present/absent library checks.

**Offline or pinned builds.** FetchContent applies the SDL patches only to a
tree it downloads itself; a tree given with `FETCHCONTENT_SOURCE_DIR_SDL` is used
as is, and the configure then fails with "SDL patch was not applied". Make that
tree with the same patch driver instead of copying a hand-patched one:

```sh
native/tools/prepare_sdl_tree.sh <release-3.4.10 SDL tree> <new patched dir>
cmake ... -DFETCHCONTENT_SOURCE_DIR_SDL=<new patched dir> \
  -DPETARI_LIBUSB_URL=file:///path/to/libusb-1.0.30.tar.bz2
```

The SDL input may be the pristine release tarball's contents or the tree Aurora
populated (it already carries Aurora's two Android patches). `PETARI_LIBUSB_URL`
(optional) points the libusb download at a local copy; its SHA256 is verified
either way. Re-run `prepare_sdl_tree.sh` whenever `native/gx/sdl-patches/`
changes (the configure compares the stamps with the patch files). After a build,
`native/tools/check_app_bundle.py <Petari.app>` confirms the bundle: libusb in
`Contents/Frameworks`, install name, no Homebrew dependency, rpath, licences,
signatures, and that both SDL patches are compiled into the executable.

## Resource validation

The default build also produces `petari_asset_check`. It validates big-endian
RARC and U8 archive tables, resource bounds, and Yaz0/Yay0 decompression. Host metadata
is stored separately from the serialized Wii structures. Sanitizer tests include
truncated input, invalid offsets, overlapping compression runs, and compressed
resource payloads. JKR archive classes now build native heap-owned metadata
from serialized RARC tables while retaining original resource payload bytes.
A real JKR heap/decompression-thread test mounts all 2,228 disc RARC archives and
compares 24,788 resources, including lookup and cached-pointer identity. The original
SDK ARC API for U8 archives now reads big-endian fields and preserves host
pointers; validate the buffer with `parseU8` before calling `ARCInitHandle`, whose
original signature has no buffer length.

```sh
./build/macos-arm64/petari_asset_check /path/to/extracted-game/files
```

Passing a directory recursively checks `.arc` files; passing one archive checks
only that archive. Successful validation does not establish model, animation,
text, audio, or gameplay correctness.

On the extracted USA `RMGE01` revision 0 assets, validation passed for 2,228 RARC
archives, four U8 archives, and 25,108 file payloads. The native DVD service also
passed a full byte comparison of all 2,371 files (3,228 MiB) against direct host
reads, using the original disc file table. The extraction used nodtool 1.4.4
with partition hash validation enabled. Retail assets are never included in the repository. When the extracted files
exist locally, CTest enables additional read-only model, message, and table tests;
synthetic tests run without them. The full-disc DVD/archive checks are manual.

## Native source and runtime conventions

Apple Clang cannot emit Shift-JIS string literals directly. The native build
generates source/header copies under its `legacy-sjis` directory, converting
non-ASCII narrow literals to explicit byte escapes. The original editable files
stay UTF-8. Compiler diagnostics use their original paths through `#line`.
Comments retain Unicode. Legacy `wchar_t` tokens become `PetariChar16` (C++
`char16_t`), `L` literals become `u` literals, and wide-string calls use the
separate `petari_utf16_*` runtime. System headers and native host code retain the
macOS wide-character ABI. BMG message loading converts big-endian UTF-16 to host-order copies; save,
Mii, and other text-bearing formats require their own conversion. Native helper namespaces use `PetariNative`, because the
original game already has an enemy class named `Petari`.

The native memory layer reserves 128 MiB for MEM1 and 256 MiB for MEM2. It supplies
the arena boundaries and boot information used by heap initialization. The SDK
allocator's descriptor sizes and alignment arithmetic now preserve host pointers.
Physical-address conversion accepts only pointers within those arenas. The native
Aurora BP adapter uses that mapping for legacy texture and palette commands;
texture image and palette bytes retain their Wii byte order.
Fixed-size child heaps still need sizing checks against larger native objects.
`petari_boot` supplies main-thread `OSInit`, initializing the arenas and native
scheduler once. `petari_heaps` builds the real JKR expandable heap and its
allocation operators; its sanitizer test covers root/child startup, mixed head
and tail allocations, alignment, host allocation isolation, and destruction.
Its test diagnostics print and abort instead of linking the game's framebuffer
exception viewer; a native game crash UI has not been integrated.

JMap/BCSV headers, fields, and cells now decode big-endian values without changing
their on-disc layouts. The real-data test reads 11,494 stage tables and over
1.4 million cells. When the extracted RMGE01 files are present, CTest includes
this read-only check in addition to the synthetic format tests.

KCL collision resources now normalize header, prism, vector, and octree data
without changing their on-disc sizes. The integrated asset check converts all
1,016 collision files and validates about 5.2 million octree lookups. Camera
animation and other game-specific resource readers are being converted separately.

Ordinary allocation defaults to the host allocator on non-game threads. Native
OS thread startup registers the threads that should use the current JKR heap.
All JKR heaps must reside in the MEM1/MEM2 arenas; global deletion of pointers
outside those arenas bypasses the mutable game-heap hierarchy.
Calls into host libraries must use `PetariNative::HostAllocationScope` when a
game heap is current, so host objects do not accidentally get freed with a scene.
The native language selector uses the mounted disc region and constrains the
selected language to that region's available languages. This does not establish
overall compatibility for additional regions.

The approved keyboard and mouse layout is recorded in [CONTROLS.md](CONTROLS.md).
The native input component and remapping are implemented and tested against the
original KPAD and game WPad code; in-game control validation is still pending.

## Audit the remaining source

```sh
python3 native/tools/audit_compile.py
python3 native/tools/audit_compile.py --output build/libraries-audit.json src/JSystem src/nw4r
```

The JSON reports contain compiler diagnostics for every checked translation unit.
Exit status 1 means there are remaining compile failures. This is a syntax audit;
it does not link the game or establish runtime correctness. the baseline game audit covered
1,605 C++ translation units; the Wii build manifest also includes SDK
and other library units.

An explicit CMake target attempts to compile **all** game C++ sources without
silently omitting failing files:

```sh
cmake --build build/macos-arm64 --target petari_game_objects -j 6
```

All 1,605 baseline game C++ sources built with the UTF-16 source transformation.
Native-only additions and ongoing changes need subsequent full-build verification.
The target also embeds the game's two built-in BCSV tables as Mach-O assembly.
It is excluded from the default component build because it is large. It is an
object target, not a game executable.

## Remaining validation and packaging

1. Exercise gameplay beyond the opening: jumps, spins, encounters, movies,
   subsequent stages and save/reload across sessions. The complete opening smoke,
   pointer selection, both file-creation saves, movement and normal shutdown pass.
2. Continue serialized resource runtime validation. Archive, BCSV,
   BMG, KCL, models, layouts, fonts and TPL conversions have real-disc tests.
   All 1,761 disc model/material images construct real J3DModelData successfully.
   All 4,753 disc animations also load into real animation objects and evaluate
   successfully. Particle and audio conversion tests pass; their full-game
   behavior still needs integration.
3. Validate the integrated GX draw-sync, FIFO, XFB presentation, blanking and
   frame-loop bridges in actual scenes. Metal tests cover model drawing,
   token-time color/depth reads, display copies, and Mii face/icon rendering.
   Overall visual fidelity, depth effects, and shadows remain unvalidated.
4. Exercise platform and input services in the running game. Keyboard/mouse tests run
   the original KPAD and game WPad code. The DSP/AI mixer has synthetic and real
   AFC-data checks, and the SDL output device is wired into startup, but the
   complete game audio sequence still needs testing. See native/platform/STATUS.md
   for fidelity limitations. Mii boot and rendering pass focused tests; particle
   loading and movie playback also need full-game validation.
5. Validate boot, title screen, gameplay, pointer/shake/tilt equivalents, audio,
   saves, scene transitions and performance using real assets.
6. Package an arm64 `.app` with its runtime dependencies and repeat validation
   outside the development checkout.

Petari's matching build targets
Korean `RMGK01`; `config/RMGK01/config.yml` records the expected `sys/main.dol`
SHA-1. For gameplay, an extracted game tree also needs the full `files/` content,
not just `main.dol`. A user-provided USA image was extracted under the ignored
`build/game-data` directory; its disc header identifies `RMGE01`, revision 0.
Native compatibility with that regional asset set has not been established.
No retail game data is included in this repository.

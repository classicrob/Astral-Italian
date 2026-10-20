# Persistent-state inventory (schema 1)

Source audit: 2026-10-20, application base `3056913b`; Aurora
`08122911e8621acb7ded6563813b264bec1494b5` (pinned by
`native/gx/CMakeLists.txt`). This inventories runtime write destinations,
including opt-in diagnostics and transient files. Paths below are families when
filenames depend on a title, timestamp, stage, device, or caller. A frozen app
**does not imply isolated state**: most defaults are shared across builds.
No state is moved, deleted or migrated by this document or the player launcher.

## Roots and sharing

- `U`: the app's `--user` argument; direct app default is
  `~/Library/Application Support/AstralItalian`. The player launcher defaults to
  `~/MacioGalaxy2-play/user`, overridden by `ASTRAL_USER` (passed as `--user`).
  Source: `native/app/app_main.cpp`, `parseArguments`.
- `C`: normal graphics cache root `~/Library/Caches/AstralItalian`, independent
  of `U`. `PETARI_CACHE_DIR` overrides it (creation failure falls back to `U`).
  Test/fixture launches use `U`; a busy machine-cache lock also falls back to `U`.
  Source: `native/app/pipeline_cache_dir.cpp`, `PipelineCacheDir::prepare`;
  `native/app/app_main.cpp`, `testRun`/`config.cachePath`.
- Background smoke: `U/cache` (or `PETARI_CACHE_DIR`) is `C`, with locks on both
  roots. Metal and temporary namespaces are isolated below; this mode refuses
  the app's default real user directory. Source: `native/app/smoke_storage.cpp`.
- Nothing is automatically keyed by frozen build number. Separate `ASTRAL_USER`
  and `PETARI_CACHE_DIR` are needed for per-build save/settings and graphics
  cache isolation. Explicit output paths are shared if reused. Never use
  `~/Library/Application Support/Petari` (the separate SMG1 game).

## Normal app state

| Destination / family | Contents and write condition | Sharing / source witness |
| --- | --- | --- |
| `U/NAND/**` | Emulated writable NAND; title save data, system/shared data, `/meta`, `/tmp`. SB4E01 title data is `title/00010000/53423445/data/` (including game-created `GameData.bin` and banner/config files). Safe writes stage copies in `tmp/sys/<unique>/<name>` before replacement; NAND `/tmp` is cleared at boot. Nodes carry `com.petari.nand` extended attributes. | Per `U`, shared by every build using it. `native/app/platform_host.cpp`; `native/platform/nand/{nand_host,isfs_host,nand_async}.cpp`. The low-level standalone NAND fallback accepts `PETARI_NAND_ROOT` only when not already mounted; the app mounts `U/NAND` explicitly. |
| `U/settings.txt`, `U/settings.txt.tmp` | Host SC console settings; temporary replacement. | Per `U`; `native/platform/sc/sc_host.cpp`, `setStore`/flush. |
| `U/mods.txt` | Home-menu mod switches, including folder mod state. `PETARI_MODS`/`PETARI_MOD_FOLDERS` override effective state without making a separate destination. | Per `U`; `native/input/mods.cpp`, `native/platform/dvd/mod_folder.cpp`. |
| `U/controls.txt`, `.tmp`, `.bad` | Input remapping; atomic replacement and preservation of malformed old controls. | Per `U`; `native/input/remap.cpp`, `save`. |
| `U/camera.txt` | Camera/photo preferences saved from the Home menu. | Per `U`; `native/input/camera_input.cpp`, `load`/`save`. |
| `U/progress.json`, `.tmp`; `U/progress_settings.txt` | Personal mission record/badge settings; record reset may remove the JSON. | Per `U`; `native/progress/progress.cpp`; app `Progress::load`. |
| `U/mods-saves/<mod>.json`, `.json.tmp` | Extra galaxy progress beside, rather than inside, the game save. | Per `U`; `platform_host.cpp`; `native/platform/dvd/mod_galaxy.cpp`, `sidecarPath`/`saveRecords`. |
| `U/mods-cache/**` | Generated disc-file patches for enabled folder mods. | Per `U`; `native/platform/dvd/mod_folder.cpp`, `resolveForUser`. Patch builder/editor integration is deferred by root `CMakeLists.txt` in this SMG2 base; retain the configured destination in the inventory. |
| `U/imgui.ini` | ImGui window/layout settings. | Per `U`; pinned Aurora `lib/imgui.cpp`, `g_imguiSettings`. |
| `U/controller_ports.dat`; `U/keyboard_bindings.dat`; `U/<controller-name>_<VID>_<PID>.controller`; corresponding `.tmp-<unique>` siblings | Aurora port preferences and controller/keyboard mapping serialization, when those Aurora UI paths are used. Separate from native `controls.txt`. | Per `U`; pinned Aurora `lib/input.cpp`, `lib/dolphin/pad/pad.cpp`, `lib/io.cpp` atomic writer. |
| `C/pipeline_cache.db`, `C/dawn_cache.db`; SQLite `-wal`, `-shm`, `-journal` and adoption `.partial` siblings | Pipeline configurations and Dawn compiled data. First normal launch can snapshot existing `U` databases into absent machine databases. | Shared machine defaults; explicit/test roots as above. `pipeline_cache_dir.cpp`; pinned Aurora `lib/gfx/pipeline_cache.cpp`, `lib/webgpu/gpu_cache.cpp`. Bundled `initial_pipeline_cache.db` is read-only input, not an additional output. |
| `C/imgui.log` | ImGui's configured debug log destination, when logging is requested. | Same sharing as cache; pinned Aurora `lib/imgui.cpp`. |
| `~/Library/Caches/AstralItalian/.petari-instance.lock` | Machine graphics-cache flock file; file may remain after process exit. | Machine shared; `pipeline_cache_dir.cpp`. Override/test paths bypass this particular lock. Do not infer a live owner from file existence. |
| `U/Crashes/petari-crash-<time>-<pid>.txt` | Native fatal/signal report. | Per `U`; `platform_host.cpp`; `native/platform/os/os_crash.cpp`. |
| `U/Crashes/petari-hang-<pid>.sample.txt` | Watchdog's optional `/usr/bin/sample` output (`PETARI_HANG_SAMPLE`). Without a supplied hang directory the helper can use `TMPDIR` or `/tmp`. | Per invocation; `native/app/smoke.cpp`, `sampleProcess`/hang watchdog. |
| stdout / stderr | Application, game, sanitizer and trace logs. Direct app launch does not itself choose a generic log file. | Inherited descriptors; player launcher redirects both below. Sanitizer runtime env such as `ASAN_OPTIONS=log_path=...` can independently redirect its own reports. |

## Player launcher and captures

| Destination / override | Contents / sharing | Source |
| --- | --- | --- |
| `U/logs/play-<stage-or-title>-<timestamp>.log`; `ASTRAL_LOG` may replace the whole filename | Per-run redirected stdout/stderr. Directory is shared unless `ASTRAL_USER` differs; same-second names may collide. | `native/play/play.sh` |
| `~/MacioGalaxy2-play/captures/<timestamp>[-N]/{route.csv,state.txt,screen.png}`; `ASTRAL_CAPTURES` changes root via `PETARI_CAPTURE_DIR` | M/F9 capture of the preceding 300 frames and screenshot/state. Absent `PETARI_CAPTURE_DIR`, direct app capture is disabled. `PETARI_CAPTURE_LOG` is recorded as a reference, not copied/written. | `native/app/playtest_capture.cpp`; `native/gx/present/present_aurora.cpp` |
| Exact `PETARI_ROUTE_RECORD` file | Full post-frame route CSV; exclusive creation (`wx`), preserves invalid/dead frames. Shared only if caller reuses the path; does not default to a build directory. | `native/app/smoke_game.cpp`; `route_record_writer.hpp` |
| `~/MacioGalaxy2-play/smoke/<timestamp>-<stage>/{user/**,app.log,frames.csv}`; `ASTRAL_SMOKE_ROOT` changes root | Frozen smoke fixture, fresh user root; app still follows graphics-cache selection rules above. | `native/play/smoke_frozen.sh` |

## Opt-in developer outputs

These are off unless configured. Destinations are the exact caller-supplied
file/directory, not automatically per-build; relative paths use process CWD.
String-valued trace switches that only print to stderr do not add destinations.

| Selector | Output family | Source |
| --- | --- | --- |
| `PETARI_AUDIO_CAPTURE` | Raw s16le stereo audio file | `native/audio/sdl_sink.cpp` |
| `PETARI_FRAME_CSV` | Per-frame phase/timing CSV | `native/app/frame_seam.cpp`, `frame_stats.cpp` |
| `PETARI_SOAK_CSV` | CSV plus `<path>.heaps.csv` | `native/app/soak_telemetry.cpp` |
| `PETARI_SPIKE_PROFILE` | Sampled stack profile file | `native/app/spike_profiler.cpp` |
| `PETARI_COLLISION_PROBE` | Collision CSV | `native/app/collision_probe.cpp` |
| `PETARI_INPUT_SCRIPT_LOG` | Input-script delivery log | `native/app/input_script_probe.cpp` |
| `PETARI_PIPELINE_DUMP` | `<runtime-key>-<shader-key>.wgsl` files | `native/gx/pipeline_profile.hpp`, `dumpSource` |
| `PETARI_TEXTURE_PROFILE` | Texture upload/profile file | `native/gx/patch_aurora_texture_timing.py` |
| `PETARI_XFB_HASH` | Appended XFB hash records | `native/gx/present/present_aurora.cpp` |
| `PETARI_EFB_DUMP` | `<label>-<index>-t<ticket>.png` | `native/gx/efb_snapshot.cpp` |
| `PETARI_XFB_DUMP` | `xfb-<label>-<index>.png` | `native/gx/present/present_aurora.cpp` |
| `PETARI_SURFACE_DUMP` | `surface-<label>-<index>.png` | Same presenter |
| `PETARI_EDITOR_THUMB_DUMP` | `thumb-<serial>-<copy-tag>.png` | Same presenter (editor currently deferred) |
| `PETARI_FIGURE_PROBE_SHOTS` (fallback `PETARI_SKIP_PROBE_SHOTS`); skip probe's shots root | Probe-requested PNG filenames | `native/app/figure_probe.cpp`, `skip_probe.cpp` |
| Caller of `Screenshot::request(path)` | Exact PNG path; common writer for playtest/probe/photo screenshots | `native/include/petari/screenshot.hpp`; presenter. Photo controls exist, but the SMG2 base does not link an independent photo screenshot path producer. |
| Aurora texture-dump configuration | `C/texture_dumps/<texture-key>.dds` | Pinned Aurora `lib/gfx/texture_replacement.cpp`; only if `allowTextureDumps` enabled (zero-initialized/off in app config). Replacement-directory API may create its caller-selected root. |

Headless developer runs (`native/app/headless/main.cpp`, `native/tools/headless.py`)
write the caller's trace, `.timing.csv`, `.sync.json`, `.complete`,
`.manifest.json`, temporary sibling `smg2-kpad-*/inputs.kpd` tape, user NAND/settings, and launcher logs.
Later actor-enabled builds add `.actors.csv`, `.actor-frames.csv`, and `.partial`
staging siblings. The launcher uses `~/MacioGalaxy2-team/slots/native.lock`;
this is distinct from the app instance locks. These are developer diagnostic
artifacts, not destinations chosen by the frozen player menu.

## Framework / temporary state and inactive features

- Metal/Apple driver compiler caches live in the OS-selected Darwin user cache
  namespace (`confstr(_CS_DARWIN_USER_CACHE_DIR)`), under the app identity
  `dev.petari.Petari`; exact `/var/folders/...` prefixes are OS assigned, shared
  by normal builds. macOS may also manage preferences/window restoration under
  `~/Library/Preferences/dev.petari.Petari.plist`,
  `~/Library/Saved Application State/dev.petari.Petari.savedState`, and native
  crash diagnostics in `~/Library/Logs/DiagnosticReports/`. These are framework
  managed possibilities, not additional explicit native file sinks. Bundle
  identity is in `native/app/Info.plist.in` / `CMakeLists.txt`.
- Background smoke explicitly creates `U/.petari-instance.lock`,
  `C/.petari-instance.lock`, `C/metal/dev.petari.Petari/**`, `C/tmp/**`, and a
  `PetariSmoke-<cache-path-hash>` Darwin cache alias to `C/metal` plus an
  OS-managed temporary namespace with the same suffix. `TMPDIR` becomes
  `C/tmp`. Database seeding uses `.partial` files; optional
  `PETARI_SHADER_SEED_DIR` is read-only seed input. See `smoke_storage.cpp`.
- Editor `U/editor/ui-settings.txt`, dialogs' user-selected export paths and
  editable mod content are behind `PETARI_EDITOR_UI`; root `CMakeLists.txt`
  explicitly defers the SMG2 editor. Do not describe them as current player
  writes. `PETARI_MOD_DIRS` identifies additional mod search directories,
  normally read-only; the active app never rewrites the extracted disc.
- Aurora memory-card storage is compiled OFF (`AURORA_ENABLE_CARD OFF` in
  `native/gx/CMakeLists.txt`); its card-file writers are not a second native
  save destination. Game saves go through NAND above. Test executables,
  Python maintenance/conversion tools, build systems and OS-global services
  are outside the player-app inventory; their explicit output arguments are
  not app defaults.

## Updating this inventory

When adding a write sink, update this versioned file in the same source change.
Audit native `getenv`, `Paths::user`, `config.userPath`/`cachePath`, `fopen`,
`ofstream`, `open(O_CREAT...)`, SQLite, PNG/screenshot calls, atomic replacement,
and dependency configuration. Include temporary siblings, sharing policy and
source witness. A source inventory establishes possible destinations; it is
not a claim that every opt-in file exists after each run.

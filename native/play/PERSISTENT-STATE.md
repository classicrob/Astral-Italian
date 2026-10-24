# Persistent-state inventory (schema 1)

Source audit: 2026-10-20, application freeze base `fe71f35b` plus cache-policy
commit `1b122ce49a9c5f14baa0cf45b352f51b58d6c872`; Aurora
`08122911e8621acb7ded6563813b264bec1494b5` (pinned by
`native/gx/CMakeLists.txt`). This inventories runtime write destinations,
including opt-in diagnostics and transient files. Paths below are families when
filenames depend on a title, timestamp, stage, device, or caller. A frozen app
**does not imply isolated state**: most defaults are shared across builds.
No state is moved, deleted or migrated by this document or the player launcher.

## Roots and sharing

- `U`: the app's `--user` argument; direct app default is
  `~/Library/Application Support/Astral Italian`. The player launcher defaults to
  `~/MacioGalaxy2-play/user`, overridden by `ASTRAL_USER` (passed as `--user`).
  `play.sh --unlocked` (menu **u**) instead selects exactly
  `~/Library/Application Support/AstralItalian-unlocked`, overriding
  `ASTRAL_USER` for that run; it requires a non-symlink user root and existing
  `complete-save.json`. This build-5 complete save is separate from ordinary
  progress. Its NAND, settings, logs and other `U`-relative state all use the
  unlocked root; captures still use their independent root below.
  Source: `native/app/app_locations.{hpp,cpp}`, `native/app/app_main.cpp`, `resolvePaths`; `native/play/play.sh`.
- `C`: normal graphics cache root `~/Library/Caches/AstralItalian`, independent
  of `U`. `PETARI_CACHE_DIR` overrides it (creation failure falls back to `U`).
  Test/fixture launches use `U`; a busy machine-cache lock also falls back to `U`.
  Source: `native/app/pipeline_cache_dir.cpp`, `PipelineCacheDir::prepare`;
  `native/app/app_main.cpp`, `testRun`/`config.cachePath`.
- Background smoke: `U/cache` (or `PETARI_CACHE_DIR`) is `C`, with locks on both
  roots. Metal and temporary namespaces are isolated below; this mode refuses
  the app's default real user directory. Source: `native/app/smoke_storage.cpp`.
- Save/settings roots are not keyed by frozen build number: separate
  `ASTRAL_USER` values isolate them. Graphics databases share a root but use
  renderer-content-keyed filenames and metadata, described below. Identical
  renderers may share caches across frozen builds; `PETARI_CACHE_DIR` selects
  a separate root when needed. Explicit output paths are shared if reused. Never use
  `~/Library/Application Support/Petari` (the separate SMG1 game).

## First-run setup and remembered choices

The direct app needs no launcher or environment variables. Disc resolution is
`--disc`, then `PETARI_GAME_DIR`, then a quick-validated `disc` in `app.cfg`,
then an existing developer `build/game-data/SB4E01`, then native setup. An invalid
remembered disc explains the problem and returns to setup. `--setup` forces setup;
**Choose Game Disc…** in the application menu records a choice for the next launch.
Extracted folders are fully validated in place. Disc images import into
`~/Library/Application Support/Astral Italian/Game Data/SB4E01/` using the player's
own USA SB4E01 revision-0 disc. No game data ships with the app.

`app.cfg` lives at that support root, independently of `--user`. It is atomic
key=value text (temporary `.app.cfg-*` siblings) with `disc`, `disc_source=folder|imported`, and the one-time
`migration=copy|declined` choice. Unknown keys survive writes. Setup can offer to
copy (never move) old `~/MacioGalaxy2-play/user` saves, or the former direct-app
`~/Library/Application Support/AstralItalian` saves, only when no `--user` was
given and the new root has no `NAND/`. It copies absent NAND/settings/mods/controls/
camera/progress/mod-sidecar entries without merging or replacing existing entries,
stages complete entries before publishing them, preserves NAND metadata, and
writes `migrated-from.txt`. Transient `.migration-*` siblings are removed after
success/failure. Either answer is remembered.
The separate SMG1 `Petari` root is never a migration source or an allowed app root.

Without `PETARI_TAPE_RECORD` or `PETARI_ROUTE_RECORD`, the direct app records a
session in `~/Library/Application Support/Astral Italian/Tapes/session-<time>-<pid>/`
(`route.csv`, `route.kpd`, `route.actions`, `route.tape.json`). Startup keeps nine
previous app session directories plus the new one. `ASTRAL_TAPE=off` disables the
default; explicit record/capture destinations are preserved and never pruned.
Caps apply to app-owned log/session names only, not unrelated files or symlinks.
Capture histories are not automatically pruned. `--user` still controls all user
state listed below and does not change mod/settings semantics.

Automation-only hooks: `ASTRAL_SUPPORT_DIR` overrides the support root so tests
and `petari_first_run_demo` use temporary storage; `ASTRAL_FIRSTRUN_PICK` supplies
the panel choice, and `ASTRAL_FIRSTRUN_PROMPTS=off` logs alerts and quits on error
instead of waiting for a reply. The demo requires `ASTRAL_SUPPORT_DIR`.

## Normal app state

| Destination / family | Contents and write condition | Sharing / source witness |
| --- | --- | --- |
| `U/NAND/**` | Emulated writable NAND; title save data, system/shared data, `/meta`, `/tmp`. SB4E01 title data is `title/00010000/53423445/data/` (including game-created `GameData.bin` and banner/config files). Safe writes stage copies in `tmp/sys/<unique>/<name>` before replacement; NAND `/tmp` is cleared at boot. Nodes carry `com.petari.nand` extended attributes. | Per `U`, shared by every build using it. `native/app/platform_host.cpp`; `native/platform/nand/{nand_host,isfs_host,nand_async}.cpp`. The low-level standalone NAND fallback accepts `PETARI_NAND_ROOT` only when not already mounted; the app mounts `U/NAND` explicitly. |
| `~/Library/Application Support/AstralItalian-unlocked/complete-save.json` (or a permitted isolated descendant `U/complete-save.json`) | Game-written complete-save/reload receipt from the opt-in `ASTRAL_DEV_COMPLETE_SAVE` / `ASTRAL_VERIFY_COMPLETE_SAVE` development flow. A receipt is required to play with `--unlocked`; ordinary play does not regenerate it. | Shared across builds choosing unlocked progress; `src/Game/System/NativeCompleteSave.cpp`, `checkRoot`/`beforeStageLaunch`; `native/include/petari/complete_save_paths.hpp`. |
| `U/settings.txt`, `U/settings.txt.tmp` | Host SC console settings; temporary replacement. | Per `U`; `native/platform/sc/sc_host.cpp`, `setStore`/flush. |
| `U/mods.txt` | Home-menu mod switches, including folder mod state. `PETARI_MODS`/`PETARI_MOD_FOLDERS` override effective state without making a separate destination. | Per `U`; `native/input/mods.cpp`, `native/platform/dvd/mod_folder.cpp`. |
| `U/controls.txt`, `.tmp`, `.bad` | Input remapping; atomic replacement and preservation of malformed old controls. | Per `U`; `native/input/remap.cpp`, `save`. |
| `U/display.txt`, `U/display.txt.tmp-XXXXXX` | Windowed/fullscreen, window-size preset and next-launch VSync; defaults are 1280×720 windowed with VSync off. Unique temporary sibling is flushed then atomically renamed; failed writes remove it. | Per `U`; `native/app/display_settings.cpp`, Home menu Display page and host frame seam. Loaded before Aurora window creation. |
| `U/camera.txt` | Camera/photo preferences saved from the Home menu. | Per `U`; `native/input/camera_input.cpp`, `load`/`save`. |
| `U/progress.json`, `.tmp`; `U/progress_settings.txt` | Personal mission record/badge settings; record reset may remove the JSON. | Per `U`; `native/progress/progress.cpp`; app `Progress::load`. |
| `U/mods-saves/<mod>.json`, `.json.tmp` | Extra galaxy progress beside, rather than inside, the game save. | Per `U`; `platform_host.cpp`; `native/platform/dvd/mod_galaxy.cpp`, `sidecarPath`/`saveRecords`. |
| `U/mods-cache/**` | Generated disc-file patches for enabled folder mods. | Per `U`; `native/platform/dvd/mod_folder.cpp`, `resolveForUser`. Patch builder/editor integration is deferred by root `CMakeLists.txt` in this SMG2 base; retain the configured destination in the inventory. |
| `U/imgui.ini` | ImGui window/layout settings. | Per `U`; pinned Aurora `lib/imgui.cpp`, `g_imguiSettings`. |
| `U/controller_ports.dat`; `U/keyboard_bindings.dat`; `U/<controller-name>_<VID>_<PID>.controller`; corresponding `.tmp-<unique>` siblings | Aurora port preferences and controller/keyboard mapping serialization, when those Aurora UI paths are used. Separate from native `controls.txt`. | Per `U`; pinned Aurora `lib/input.cpp`, `lib/dolphin/pad/pad.cpp`, `lib/io.cpp` atomic writer. |
| `C/pipeline_cache.<renderer-key>.db`, `C/dawn_cache.<renderer-key>.db`; SQLite `-wal`, `-shm`, `-journal` and adoption `.partial` siblings | Pipeline configurations and Dawn compiled data. Exactly one matching `petari_cache_build` identity is mandatory; only empty newly created databases can acquire it at runtime. Normal/background adoption copies only matching named and tagged `U` databases. Legacy `pipeline_cache.db` / `dawn_cache.db`, foreign keys and untagged databases are ignored and left untouched, even if renamed. | Root selection/locks unchanged; namespace shared only by identical renderer keys. `native/gx/pipeline_cache_identity.{hpp,py}`, `patch_aurora_cache_policy.py`; `pipeline_cache_dir.cpp`, `smoke_storage.cpp`; Aurora pipeline/Dawn cache call sites patched by commit `1b122ce4`. |
| `C/imgui.log` | ImGui's configured debug log destination, when logging is requested. | Same sharing as cache; pinned Aurora `lib/imgui.cpp`. |
| `~/Library/Caches/AstralItalian/.petari-instance.lock` | Machine graphics-cache flock file; file may remain after process exit. | Machine shared; `pipeline_cache_dir.cpp`. Override/test paths bypass this particular lock. Do not infer a live owner from file existence. |
| `U/Crashes/petari-crash-<time>-<pid>.txt` | Native fatal/signal report. | Per `U`; `platform_host.cpp` and the headless harness; `native/platform/os/os_crash.cpp`. |
| `U/Crashes/petari-hang-<pid>.sample.txt` | Watchdog's optional `/usr/bin/sample` output (`PETARI_HANG_SAMPLE`). Without a supplied hang directory the helper can use `TMPDIR` or `/tmp`. | Per invocation; `native/app/smoke.cpp`, `sampleProcess`/hang watchdog. |
| `~/Library/Application Support/Astral Italian/logs/astral-<YYYYmmdd-HHMMSS>[-pid].log` / inherited stdout and stderr | Direct app launch redirects both only when they are closed or `/dev/null` (Finder); terminal, pipe and file redirection are preserved. Keeps the 20 newest app logs. | `native/app/app_locations.cpp`, `redirectAppLog`; player launcher redirects both below. Sanitizer runtime env such as `ASAN_OPTIONS=log_path=...` can independently redirect its own reports. |

## Player launcher and captures

| Destination / override | Contents / sharing | Source |
| --- | --- | --- |
| `U/logs/play-<stage-or-title>-<timestamp>.log`; `ASTRAL_LOG` may replace the whole filename | Per-run redirected stdout/stderr. Directory is shared unless `ASTRAL_USER` differs; same-second names may collide. | `native/play/play.sh` |
| `~/MacioGalaxy2-play/captures/<timestamp>[-N]/{route.csv,state.txt,screen.png}`; `ASTRAL_CAPTURES` changes root via `PETARI_CAPTURE_DIR` | M/F9 capture of the preceding 300 frames and screenshot/state. Absent `PETARI_CAPTURE_DIR`, the direct app uses `~/Library/Application Support/Astral Italian/Captures/`. `PETARI_CAPTURE_LOG` is recorded as a reference, not copied/written. | `native/app/playtest_capture.cpp`; `native/gx/present/present_aurora.cpp` |
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
also install the native crash handler in `U/Crashes` (the report family above), and
write the caller's trace, `.timing.csv`, `.sync.json`, `.complete`,
`.manifest.json`, temporary sibling `smg2-kpad-*/inputs.kpd` tape, user NAND/settings, and launcher logs.
The freeze includes `--actor-trace`: when enabled it adds `.actors.csv`,
`.actor-frames.csv`, and `.partial` staging siblings, finalized before `.complete`
(`native/app/headless/actor_trace.cpp`). `--existing-unlocked-user` permits the
separate unlocked save root instead of a fresh fixture; output traces still use
the caller-selected paths. The launcher uses `~/MacioGalaxy2-team/slots/native.lock`;
this is distinct from the app instance locks. These are developer diagnostic
artifacts, not destinations chosen by the frozen player menu.

## Complete-save preparation artifacts

`native/tools/complete_save.py` creates only the separate unlocked root (or a
permitted descendant), through the game's NAND APIs. It also creates
`~/MacioGalaxy2-gamedata/unlocked-validation/<timestamp>/` with
`WorldMap07Galaxy` / `MarioFaceShipGalaxy` logs, headless traces and sidecars,
and final `acceptance.json`. These are per-validation-run artifacts; the
unlocked user tree and `complete-save.json` persist across builds. The helper
refuses nonempty creation destinations; `--verify-only` reloads existing data.

The older `native/tools/make_unlocked_save.py` is a distinct opt-in preparation
workflow with caller-selected private output/work roots. It creates a
`.petari-make-unlocked-save` marker, `unlocked-save-inventory.json`,
`unlocked-save-request.json`, `unlocked-save-receipt.json`,
`unlocked-save-nand-ledger.json`, `unlocked-save.json`, variant/inventory logs and
copied `NAND/**` when that workflow's game-side provider succeeds. These are
not created by selecting the build-5 `--unlocked` player option. Its existing
source/provider limitations are unchanged by this inventory.

## Freeze delta audit

The source diff `ee0732c0..fe71f35b` changes native zero-texgen shader handling
and arena mappings/tests, adding no write destination. Auditing the earlier
inventory base `3056913b..fe71f35b` additionally identified the unlocked-user
launcher/helper flow and the finalized actor sidecars documented above. Mod
behavior changes retain `U/mods.txt`; they do not add storage roots. Cache
policy commit `1b122ce4` subsequently replaces the unkeyed database names with
renderer-keyed names and checks all adoption/fallback paths as described here.

## Renderer cache identity and seeds

The renderer key is SHA256 of the renderer/config schema, pinned Aurora input,
native GX source/patch chain and renderer packaging inputs. It excludes checkout
paths, mtimes and Git state; it is **not the whole game-source revision** or a
frozen build number. P's delivered `1b122ce4` key is
`2e0537ae4d307d5bdd63cfd5edf1ea1187d0dfa557789878a4f984bcd8a6b917`;
later renderer changes can produce a different key.

All normal machine, `PETARI_CACHE_DIR`, user-fallback and background-smoke roots
use the same filename/metadata admission. A busy or failed machine cache cannot
bypass this through `U`. Non-renderer state such as `imgui.log`, `imgui.ini`,
locks and the OS-managed Metal namespace is not renamed by this change.

Bundling writes a **build-local copy** of initial/stage seed databases, stamps
it with the target renderer identity and leaves the seed input untouched.
Pre-stamped foreign seeds are errors, not relabeled inputs. The bundled
`initial_pipeline_cache.db` and stage seed inputs are read-only at play time;
they are admitted by identity before import into the keyed runtime database.
GX preload, initial-seed and stage-manifest ingresses validate structure and
sampled texgens, skip invalid rows and log their hashes/reasons. They never
normalize or repair invalid bytes. This changes admission, not the SQLite
format version or any save location. Source contract:
`docs/port/pipeline-cache-policy.md` at `1b122ce4`.

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

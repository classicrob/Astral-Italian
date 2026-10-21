# M1 WorldMap validation helpers

Private iteration tools, not production game code. These copies preserve the reviewed diagnostic/dispatch comparison recipe. They expect the existing user-local paths under `~/toolchains/sol-t-validation/worldmap51`; install copies there before use. No game data, saves, input recordings, logs or captures are included.

Heavy work requires the lead's explicit M1 handoff and `~/MacioGalaxy2-team/slots/native.lock`, nice19, two build jobs. The scripts write one-line STATE before lock/spawn/completion. App/compiler termination belongs only to `native/tools/bounded_run.py`; no separate signal controller is supplied.

`diag_build.py SHA` applies the reviewed diagnostic onto the pinned comparison source. `ab_build.py tu13|combined` switches to the exact reviewed comparison tip while verifying the external overlay/toolchain. Both complete `petari_legacy_sources` before `petari`, check changed generated/object hashes and freshness, require a real link/new executable hash and diagnostic markers, then preserve an app and receipt. These are historical pinned recipes; do not silently substitute source tips. Source/token review is not Wii object proof.

`run_map.py` requires a completed receipt and private save/disc fixtures. It rechecks source, compiler, generated objects, build logs and executable before launch. Each run copies the save into a fresh private user directory, with a new empty app cache. OS/driver caches are not controlled. It sets `PETARI_MODS=none`; the inherited JumpDive flag remains inert while Movement is off. Preserve identical effective settings across comparisons.

`host_boot.py` sends ordinary Return, pointer hover and Space through the host event path. It does not open a game save or set game state. Its FileSelect1/Start coordinates were verified at the default measured window; inspect the actual UI if geometry/readiness differs. Scene-preparation log names do not establish foreground scenes.

Host helpers query/activate/move/click/resize only the exact app PID in the private run receipt. Right click is A; left click is B. `host_capture.py` must be launched with the exact granted Python.app through LaunchServices. `capture_wait.py` waits for its timestamped capture receipt, preventing a following pointer move from racing capture. LaunchServices may take focus; reactivate before ordinary input. Permissions are preflighted, never changed by these tools.

`compact_diagnostic.py` retains selected internal capture pairs and nearest pairs for host captures. It preserves logs, source/binary receipts, CSVs and fault evidence; it removes only that owned case's excess PNGs. Its summary requires manual functional/fault classification.

Evidence limits: `aurora_frame` is a presentation-choice ordinal. Preserve phase/sequence and the explicit route-row join. A script counter or matching image label is not a game-frame join. Host event brackets and route pointer values are not consumed KPAD/history/layout hit-test telemetry. Visible pointer movement plus actual node/scene transitions establish observed functionality only. A crash exit with no matched log expression is still failure; inspect the native fault summary.

Validation: all copied Python files parsed successfully. Actual native source/build/run receipts remain outside this repository. No standalone game acceptance is implied by this tool snapshot.

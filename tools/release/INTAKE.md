# Tester report intake

From the repository root, with Python 3.9 or newer (standard library only):

```sh
python3 -I tools/release/intake.py /path/to/AstralItalian-report.zip
python3 -I tools/release/intake.py /path/to/report-folder --work /path/to/fresh-output \
  --disc ~/MacioGalaxy2-gamedata/SB4E01-extracted
```

The report is format 1 from [REPORT-FORMAT.md](REPORT-FORMAT.md). Intake copies it into a fresh output directory, verifies its build through `builds.tsv`, obtains headless for that source, and invokes the trusted repository's `tools/port/replay_tape.py`. It never runs report attachments. The disc stays where it is; it is never copied into the report, cache, or repository.

`triage.md` starts with the primary verdict, tester note (including capture notes when `note.txt` is empty), build identity, and commands for investigating the result. `triage.json` contains the evidence, all verdicts, original build fields, warnings, commands, exit statuses, signal, log summaries, timing rows, and optional symbolicated frames. `--json` prints that JSON as well. Exit 0 means triage ran, including an unknown build, unusable tape, divergence, or crash. Exit 2 means intake could not finish: inspect its error and, when created, `triage.json`'s `tool_errors`. In that case a primary verdict can be null.

Options:

- `--headless PATH`: use a trusted developer binary outside the report. Its source identity is the caller's responsibility; a build mismatch still produces `BUILD_UNKNOWN`.
- `--commit FULL_COMMIT`: explicitly choose source for an unknown build. Intake preserves `BUILD_UNKNOWN` and the identity warnings. Only full 40-character commits are accepted.
- `--no-build`: use only an existing verified cache entry or `--headless`. A missing binary is a tool error.
- `--max-frames N`: replay a prefix of at most N playable tape frames across all segments. A match then covers only that prefix; omit this option to check the whole available recording.
- `--dsym PATH [PATH ...]`: supply dSYM search locations to `symbolicate.py` (may be repeated). Without it, the symbolicator uses its own UUID-based search. When `symbolicate.py` has not landed or the matching dSYM is missing, raw crash evidence and the reason remain in the result.
- `--keep`: retain the headless user directory for debugging. Otherwise it is removed after crash evidence has been read. All other report/replay evidence is retained until the developer removes the output directory.

## Storage and builds

Default output: `~/MacioGalaxy2-team/intake/<report-name>/`. Existing output is refused; choose a fresh `--work` when repeating intake. Zip entries and directory copies reject absolute paths, parent traversal, symlinks, special files, and case-insensitive duplicate paths. Limits are 10,000 entries and 4 GiB of expanded data. Report metadata and displayed text have separate bounded reads. Unknown tape environment variables are rejected; only the recorded `PETARI_TEST_RETURN_TO_SHIP` hook and recorded mod names are passed through.

Each output contains:

```text
report/             collected evidence, unchanged
replay-tape/        private local copy; sidecar paths point to collected sibling files
replay/             headless log, traces, timing CSV, prepared inputs, command.json,
                    first-divergence.json, and any headless crash reports
replay.log          replay driver output
build.log           configure/build output, if a build was needed
triage.md
triage.json
```

The prepared local tape is retained so the exact replay command in `triage.md` remains runnable. A partial (`complete: false`) recording is replayed through its last recorded checkpoint using `--allow-incomplete`. Intake never changes the original tape. It validates binary record lengths before loading samples. Replay uses the existing tool's play-ready seams, RNG seeds, exact KPAD samples, mod presses, recorded mods, and comparison tolerance; see [PLAYTEST.md](../../native/PLAYTEST.md#recording-a-tape).

Cache: `~/MacioGalaxy2-team/intake/headless/<full-commit>/petari_headless` plus `BUILDINFO.json`. The receipt records source commit, binary SHA-256, compiler, dependency source directory, commands and elapsed build time. Cache reuse verifies the commit and SHA-256. The executable is built from a detached worktree at `intake/src/<commit>` into `intake/src/<commit>-build`; both are removed in the build's cleanup path. Stale scratch directories are refused for manual inspection after an interruption.

Builds serialize through `lockf -k ~/MacioGalaxy2-team/slots/intake-build.lock`, run `nice -n 19`, and use Release with sanitizers/tests/GX probe disabled, headless enabled, and only `--target petari_headless -j4`. Pinned sources are reused from `~/MacioGalaxy2-team/scratch-port/solA/build-ov-gx/_deps`. Older Apple clang gets the same compatibility flags as `tools/team/headlesslink.sh`. The source's deployment target default is preserved. Intake checks for at least 50 GiB free before configure/build. It never invokes the Wii build.

## Verdicts

Multiple verdicts may apply. Primary priority is build uncertainty, headless signal crash, native crash, invalid/missing tape, divergence, then match.

| Verdict | Meaning and next action |
| --- | --- |
| `BUILD_UNKNOWN` | Missing, invalid, conflicting, unreleased, or unavailable build identity. Resolve UUID/commit through `builds.tsv`; an explicitly chosen replay is marked unverified. Source-only identity can map to a release, but cannot confirm the exact executable was released. |
| `HEADLESS_CRASH` | Headless exited on a signal or wrote a crash report. Inspect the signal, last log lines, and frames when available. |
| `NATIVE_CRASH` | The bundle includes tester app crash reports. Inspect their timestamps/builds and top frames; attachments may predate the taped session. |
| `TAPE_MISSING` | No session sidecar was collected. Ask for a report after playing, with automatic tape recording enabled. |
| `TAPE_INVALID` | Corrupt/incomplete files, unsupported tape schema/environment, no playable segment, lossy samples, a controlled headless failure, or no complete comparison. The specific error and subprocess status are retained. A killed session with valid checkpoints is supported. |
| `DIVERGED` | First trajectory/state divergence: segment, stage, route row, segment frame, positions and differing fields are in the replay JSON. The suggested command stops shortly after that frame. |
| `REPLAY_MATCHED` | Headless reproduced every available playable segment (or the explicitly requested prefix). Investigate rendering, timing and stall evidence. This establishes parity with the recording, not general correctness of the game's behavior. |

The current headless timing CSV contains emulated `frame,vi_count,ticks`, not wall-clock timing. Intake records its row count/endpoints and explicitly labeled stall/duration rows when a timing CSV provides them. Tester `[stall]` lines include actual phase/cause information; shader compile log-line counts are labeled as such, not inferred pipeline totals. Replay retains the existing limitations around fresh saves, inter-segment neutral input, and pointer-only mods documented in PLAYTEST.md.

Run synthetic tests without the disc or a native binary:

```sh
python3 -I tools/release/test_intake.py
```

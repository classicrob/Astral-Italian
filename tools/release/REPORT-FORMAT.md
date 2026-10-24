# Tester bug-report bundle, format 1

Produced by `tools/release/payload/Collect Bug Report.command` (a tester double-clicks it), consumed by `tools/release/intake.py`.
One zip, `AstralItalian-report-<YYYYmmdd-HHMMSS>.zip`, containing one folder of the same name:

| Path | What | Source on the tester's Mac (`S` = `~/Library/Application Support/Astral Italian`) |
| --- | --- | --- |
| `README-PRIVACY.txt` | what is and is not inside, in plain English | script |
| `build.json` | build identity (below) | app `Info.plist` + `dwarfdump --uuid` of the executable + `sw_vers`/`sysctl` |
| `system.txt` | the same plus display info, free disk, human readable | script |
| `note.txt` | the tester's own words: what felt different / what went wrong | typed at the script's prompt (may be empty) |
| `logs/` | the 3 newest `S/logs/astral-*.log` | |
| `captures/<time>/` | the 3 newest M/F9 captures (`state.txt`, `route.csv`, `screen.png`) | `S/Captures` |
| `crashes/` | the 3 newest Petari crash reports (`petari-crash-*.txt`) and macOS `.ips`/`.crash` files mentioning Petari/Astral | `S/Crashes`, `~/Library/Logs/DiagnosticReports` |
| `tapes/<session>/` | the newest exact session tape: `route.csv`, `route.kpd`, `route.actions`, `route.tape.json` (complete or killed-session partial) | `S/Tapes` |
| `settings/` | `mods.txt settings.txt display.txt controls.txt camera.txt` if present | `S/` |

Not included: save data (`NAND/`), the game disc or any extracted game file, `app.cfg`, other tapes, shader caches.

`build.json`:

```json
{
  "report_format": 1,
  "report_time": "2026-10-23T14:02:11-0400",
  "app_version": "0.1.0",
  "source_commit": "40 hex (Info.plist AstralSourceCommit)",
  "executable_uuid": "8-4-4-4-12 uppercase (LC_UUID of Contents/MacOS/Petari; stable across code signing, unlike a file hash)",
  "app_path": "/Applications/Astral Italian.app",
  "macos": "15.1 (24B83)",
  "model": "MacBookAir10,1",
  "chip": "Apple M1",
  "memory_gb": 16,
  "newest_tape": "session-20261023-135500-1234",
  "tape_included": true,
  "tape_skipped_reason": ""
}
```

Missing values are the empty string, never absent keys. The app also writes the same identity into every crash report header and
into the tape sidecar (`route.tape.json` keys `app_version`, `source_commit`, `executable_uuid`), so the identity survives when only one
of the files reaches us.

Builds registry (maps a build to its source): `tools/release/builds.tsv`, tab separated, one release per line, appended by
`make_release.sh`: `executable_uuid  version  source_commit  frozen_build_name  zip_sha256  date`.

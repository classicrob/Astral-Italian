M1 5.4 freeze gates

Run from the shared `port/m1-tape-gates` checkout, without building:

```sh
bash gate53/m1gates/m1gates54.sh full54-r1
```

Each label is new and never overwrites prior evidence. `--only platform,promotion,intro,cappy,cold,image,warm,cadence,audio,tape` selects independent gates. The default runs them all. `--quiet-seconds 900` matches the reference scripts' maximum wait for one-minute load ≤8; the observed load and top processes are recorded even if the wait expires. Gate thresholds remain unchanged.

All game sessions hold `~/MacioGalaxy2-team/slots/native.lock`. The app is copied with `ditto` from RELEASE's read-only `scratch-release/frozen54`, given SOL A's `dev.petari.Petari.perfm1a` identifier and re-signed. Source and perf executable hashes are both recorded. Only that bundle identifier's Metal cache is cleared, under the lock. Every app uses a private cache and save copy. Owned processes are terminated by their Popen handles; no broad process matching. Each launch checks ≥50 GiB free. Raw captures and caches are deleted after their last dependent measurement; CSVs, logs, process records and summaries remain.

PORT approved **nice 0** for all M1 gates. The server used nice 19 for promotion, normal route, intro skip and Cappy. This is recorded in the manifest and per-run evidence, because the server harness was load-sensitive (BACKLOG-541).

The frozen executable supports play-ready keyboard scripts, but cannot replay title/FileSelect/menu input. The reference scripts also use host pointer input for promotion worldmap and host Space for intro skip. Those checks remain failing when that evidence is unavailable; no AX commands are called. The no-press intro arm still runs. Promotion runs all 15 cases and preserves failed pointer checks while completing the independent cases.

PORT approved testing whole-session tape input on a **separate server-built candidate**, with source/app/tape identities labelled. To invoke SOL B's normal-route gate after the frozen tests:

```sh
bash gate53/m1gates/m1gates54.sh tape-candidate-r1 --only tape \
  --candidate-app /absolute/path/to/prepared/perfm1b/Petari.app \
  --candidate-sha FULL_PERF_EXE_SHA256 --candidate-source FULL_SOURCE_SHA \
  --tape ~/MacioGalaxy2-gamedata/port53/m1g/tapes/tape-normal-54 \
  --allow-incomplete-tape
```

`--allow-incomplete-tape` is only for the server recording harness's explicitly documented termination checkpoint. SOL B's runner owns tape validation and labels its use. Candidate results never satisfy frozen54 executable identity. Add `--candidate-original-app /absolute/path/under/m1g/candidates/Petari.app --candidate-original-sha FULL_ORIGINAL_EXE_SHA256` to run the intro-skip twins and fresh/warm pointer replay adapters too; they copy the server candidate to SOL A’s perfm1a namespace.

Evidence is under `~/MacioGalaxy2-gamedata/port53/m1g/`:

- `apps/frozen54-perfm1a/PREP.json`: immutable frozen/perf copy identity and resource hashes.
- `runs/LABEL-*`: actual gate inputs, launch metadata, process results, CSV/log summaries and `RESULT.json` or reference receipts.
- `campaigns/LABEL/MANIFEST.json`: steps and exit codes.
- `campaigns/LABEL/VALIDATION.json` and `COMPARISON.md`: all gate verdicts and M1/server comparison, including missing evidence.
- `campaigns/LABEL/receipts/`: created only when every required gate passes, using `freeze54.py`'s `{gate, passed, status, receipt, limits}` format. A failure refuses the entire receipt directory.

Revalidate evidence without running GX:

```sh
python3 -B gate53/m1gates/receipts54m.py CAMPAIGN_DIR NEW_RECEIPT_DIR
python3 -B gate53/m1gates/test_gate_validation.py
```

Stage files also record reference stall diagnostics for the image/off and warm-prime arms. Image and warm-cost verdicts use their own reference criteria; the off image arm is expected to compile pipelines synchronously. These diagnostics do not replace the nine required cold-stall runs.

The reference gate parsers (`gate533.py`, `imgdiff54.py`, `warmpairs54.py`) and thread probe are copied from the server's `srv-ref`, not the older M1 copies. Missing phase data and incomplete game processes are rejected in addition to reference thresholds. The M1 rechecks release Wii source lineage; it does not claim a new Wii build or redispatch the release oracle.

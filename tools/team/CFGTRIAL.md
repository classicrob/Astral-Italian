# Configuration archaeology

`cfgtrial.py` reuses `gametrial` score comparisons and configured-edge parsing, adds stable identities for duplicate function names, and checks unit fuzzy scores as well as all functions, noncode sections, and matched code/data bytes. The current residue list has 359 units; adding the six compiler pins gives 361 requested entries, of which 351 have source compile edges. Every skipped entry is retained in the manifest.

```sh
python3 tools/team/cfgtrial.py prepare --out /absolute/new-output
python3 tools/team/cfgtrial.py probe --out /absolute/new-output
python3 -u tools/team/cfgtrial.py run --out /absolute/new-output
python3 tools/team/cfgtrial.py summary --out /absolute/new-output
```

Preparation freezes targets and actual configured command edges without running configure, Ninja, or split. The actual Game default at d4d8efa9 is `-O4,s`. Baselines are freshly compiled. Probe enables illegal-pragma diagnostics, requires a known positive control and an unknown-pragma negative control, and records all 72 requested settings. Prefix headers use Windows absolute paths for MWCC under wibo. Short temporary paths avoid MWCC's path-length limit.

The default mode performs one compiler invocation at a time under `decomp-quiet.lock`. After the lead explicitly announces quiet over, creating `OUT/parallel-enabled` enables two unlocked workers in bounded batches. Single-object trials do not take `server-heavy.lock`. Use `--jobs 4` only when no other heavy work is running; the limit accepts 1–4. Remove the marker to return to serial after the current batch. Never create it merely because an estimated quiet end time has elapsed. `OUT/stop` stops cleanly after the current bounded batch. Remove it before resuming. Completed JSONL entries are reused; inputs must remain frozen. Source or tool drift fails closed. Do not edit the tool during a run.

`--variants NAME...` runs a subset. `--limit N` is for integration smokes; neither option emits a full-matrix completion marker. The runner additionally recompiles the six pinned units with Wii/1.3 and each selected setting, comparing to their fresh Wii/1.0 controls. These are flag trials; source variants suggested by miners remain separate owner work.

Results: `acceptance.json`, `baseline.jsonl`, per-setting JSONL, `pinned13.jsonl`, `all-trials.tsv`, and ranked `summary.md` / `summary.json`. Exact byte gains and weighted fuzzy improvements are distinct. Only a no-drop gain may be considered for a per-unit override; verify dispatch and current source before adoption. Private candidate objects are deleted immediately; frozen targets and evidence remain for resume.

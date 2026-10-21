# Yoshi raw-record replay

The opt-in `--sync-input-records` suffix consumes the strict native KPAD JSONL
contract. It retains every sample bit, button edge, acceleration/pointer delta,
connection and read ordinal. It changes only exported frame labels from suffix
coordinates to source-loop coordinates. Input row 0 affects trace row 1, after
the accepted ready anchor. The actual read schedule must agree; otherwise capture fails. By default the
read-only retail WPADProbe-equivalent connection must also agree. The explicit
`--sync-observed-connection` mode instead exports the actual observed connection
without guest probe writes, retaining all source samples/history. It records
the first synthetic/actual connection difference as an input-delivery boundary,
never gameplay divergence. A matched native replay must consume the finalized
`observed-inputs.jsonl` export. The old preconverted native trace is comparable
only through the last trace row before that input difference takes effect.
Boot-prefix connections remain scripted synthetic and are outside this receipt's
actual-connection scope.

`yoshi_replay.py --root <fresh-private-directory> --slot-held` runs selector
acceptance, a 1,200-input prefix, then one continuous 30,000-input replay under an
externally held native.lock at nice 19. It uses observed-connection mode and the existing M1 private Dolphin,
ISO, profile and pinned anchor-522 conversion. Only the copied profile's GDB
socket changes. The runner validates every delivered suffix record against the
source, retains failed outputs, and never adjusts inputs or RNG to fix a route.
The existing 480-second default and 1,500-second exception remain unchanged;
the scoped Yoshi budget uses a finite 10,800-second engineering cap for the
PORT-authorized overnight replay. A timeout is a failed continuous run.

`--clear-witness` samples the stage-result holder read-only. Retail getters and
stores bind offsets for stage name, stage-result flag, new-star flag and star
number. A Yoshi scenario-1 result followed by ready Starship is required to call
the clear reproduced. Starship alone does not establish a clear. This witness
adds no columns to the common trace and performs no progress writes.

The conversion is host intent with synthesized fields and a proxy anchor,
not original consumed GX KPAD. Natural seeds remain unforced. Callback-only
pending state, unknown timing/FP policy and absent actor census keep replay
qualification BLOCKED even when the goal is reproduced. All real inputs,
profiles, outputs and logs remain private.


After acceptance, `--full-only --accepted-prefix PRIVATE/prefix-1200/result.json`
starts one fresh continuous full capture. It reparses actual accepted input and
delivery records, verifies final trace/file hashes, unchanged source profile,
boot script, strict contract and every runtime helper, and requires the same
host plus a feasible measured projection. It reuses no guest state or partial
trace. Only orchestration skips already accepted short runs. A changed runtime
helper, input prefix, profile or host requires acceptance again.

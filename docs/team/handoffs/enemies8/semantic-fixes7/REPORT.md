# Seven DOL-backed behavior repairs

The enemies183 read-only review found seven concrete body discrepancies. This batch applies one natural form per body in six source files:

| Row | Correction | Isolated matched-code gain |
|---|---|---:|
| E018 AudSystem | Category slots 5–10 use EV/EM/SV/OJ/AT/DM | 0 B |
| E044 ParticleDrawExecutor | LightOrtho receives negative then positive half-height | 0 B |
| E071 MogucchiSpike | Reflected Star Piece message receives receiver then sender | 88 B |
| E079/E080 Popo | Both knockback vectors use receiver position minus sender position | 1032 B |
| E167 LayoutManager | Copy the newly prepared matrix into the registered pane matrix | 340 B |
| E180 SuddenDeathMeter | Initialize alpha to the target binary32 1.0 | 148 B |

Expected isolated gain: 1608 B. Official gain requires the combined full-build/all-unit guard. AudSystem and Particle remain nonexact; their behavior corrections do not claim whole-caller proofs. In particular AudSystem's unchanged final two category slots are outside the populated fourteen-slot witness.

Complete target bodies, actual DOL callees/scalars/branch roles, source snapshots and rerunnable preservation checks are frozen under `~/MacioGalaxy2-team/scratch-enemies/sol8/{audsystem-E018,particle-E044,mogucchi-E071,popo-E079-E080,layout-E167,sudden-E180}`. All other emitted bodies, effective references, allocated data and existing exact anchors were checked for each unit. No headers, symbols, splits, helper bodies, compiler flags, function order or benign lists change. No live gameplay symptom is inferred from these local behavior discrepancies.

Review baseline: 6d468b76. The hook transfer was reverted and clean main 20f71244 was verified to have the same 3480 unit measures. The batch rebases onto main before its full gate.

First combined gate (exact main39ec4744): actual NJ EXIT0, DOL SHA1 7d79810889824edd01c4ff02624713060a69818c; all 3480 units checked, +1608 matched code bytes, zero code/data drops and 47774 prior exact function entries preserved. Six fresh compiled objects equal their frozen candidates. See guard39.json. Final latest-main rebase/landing remains separate.

Final rebase gate on c40345a5: actual NJ EXIT0, same DOL hash; guard-final.json confirms +1608 code bytes, zero data/code drops and preservation of all prior exact anchors across 3480 units. All six current source objects still equal the frozen candidates.

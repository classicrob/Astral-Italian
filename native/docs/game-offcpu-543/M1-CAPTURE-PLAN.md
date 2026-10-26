# Game-thread off-CPU M1 capture specification

**NON-LAUNCHABLE.** PORT has validated the five diagnostic tests at source
`3aeb9c4a69f4cb6266fdb4f701411cc1bb80d6eb`; its server-generated Release IR
exactly matches the retained fixture. The app is not built or bound yet. This
specification authorizes no compile, cache preparation or native run. PORT
schedules the M1 after SOL A's output-AU work and supplies the immutable app.
The paired JSON is a specification, not a frozen launch PLAN. Its null build,
runner, hash and GO bindings must be resolved in a NEW final PLAN before launch.
A changed implementation requires its own validation/binding; no guessed SHA.

The question is what the main/game thread is waiting on during the intermittent
warm-title wall/CPU gap. Expected outcome is **unknown**: four warm launches may
capture the event or may capture none. The r3 500ms game-work gap is historical
evidence on a different audio-history binary, not a control for this experiment.
This branch is a diagnostic instrument, never a release candidate or pacing fix.

## Fixed arms and duration

| Order | Pair | Mode | Launch | Native minimum s | Fixed frame comparison s |
| --- | --- | --- | --- | --- | --- |
| 1 | p1 | OFF | private cold prime | 315 | 300 |
| 2 | p1 | OFF | same-cache warm second | 870 | 855 |
| 3 | p2 | ON | private cold prime | 315 | 300 |
| 4 | p2 | ON | same-cache warm second | 870 | 855 |
| 5 | p3 | ON | private cold prime | 315 | 300 |
| 6 | p3 | ON | same-cache warm second | 870 | 855 |
| 7 | p4 | OFF | private cold prime | 315 | 300 |
| 8 | p4 | OFF | same-cache warm second | 870 | 855 |

ABBA applies to the four pairs; primes and seconds are separate strata. Both
members of a pair use the same mode. Native minimums total 4740s (79min), plus
boot, quiet checks and post-exit work. Reserve approximately 90–110min if quiet;
quiet waiting can extend that. Never shorten or add arms to get a clean sample.

All eight use the SAME signed derivative, bundle id `dev.petari.Petari.perfm1b`,
resources, disc fixture, no-input title path and environment. No human/AX/key,
pointer, input script, tape, mod, debugger, full GPU trace, audio history or
injected probe. CoreAudio real audio, async audio workgroup and its existing
status diagnostics remain identical; there is no AQ count/priority override.
The audio implementation is whatever the bound main-based app contains, not a
silent merge of SOL A's sink line. Audio-history OFF in all arms is deliberate:
this tests the independent game-history observer rather than two writers at once.

`PETARI_TITLE_OBSERVE_MIN_SECONDS=315/870` uses the existing native semantics:
completion requires real global graphics 100%, at least 30s beyond that, and the
minimum elapsed time, followed by the closing seam. Its clock is steady elapsed
from the first graphics-state poll in the ordinary overlay composition
(`frame_seam.cpp`), NOT Popen or a game clock. Native timeout stays 900s. External
process cap is 945s including startup; native timeout is always a failure. The
runner must verify these source semantics in the actual app build before launch.

Frame comparisons use the same pinned title scorer and its fixed-window rule
for 300/855s; record its actual clock origin, rounding/crossing-row treatment and
source hash in the final PLAN. Full native history and audio remain whole-arm.
Do not fabricate a precise history timestamp by summing rounded CSV intervals:
ON history supplies the actual CSV open/close timestamps and phase frame IDs.

## Preconditions, cache lineage and launch

Bind immutable BUILD.json, full source SHA, source/perf exe hashes, UUID,
renderer key, seed/resources and own PREP plus runner/scorer hashes. Preflight
all eight command/env/output/cache plans without launching; all must agree
except mode, arm-private paths and duration. Recheck before every launch.
OFF sets history=0 and has no history FILE; no history output/writer is expected.
ON sets history=1 and an absolute NEW arm-local file, never an existing path.
Other experiment settings use an allowlist, removing inherited trace/probe,
script/replay and observer variables; list both values and required absences in
launch.json. Record defaults that remain inherited from the bound executable.

Each pair starts with a new private pipeline cache under its new literal run
root and a cold **owned perfm1b** Metal namespace. The warm second preserves that
exact prime cache/namespace with no clear, trim, replacement or intervening app.
Hash inventories at prime exit must equal the warm prelaunch inventory. Keep the
prime's cache until the second and lineage validation finish. A prime must exit
normally and complete its native observation/cache writes before a warm launch;
pacing/audio positives do not make a completed cache prime invalid. Never read,
enumerate or clear `~/Library/Caches/AstralItalian*`, SOL A's namespace, or any
prior evidence. Use the established owned-cache guard; any requested deletion
needs a checked literal-root script, never variable-path rm-rf.

Launch only in PORT's exclusive cohort slot after A's explicit release. Detached
bash/nohup/setsid or double-fork, confirm driver/app PID ancestry and nice0.
Individual arms take m1-heavy then native via lockf -k; no outer lock. Gaps are
quiet/cache handoffs, not permission for a concurrent build. Quiet waits at most
900s per arm: load1/load5 <=8, no active build/other app, and resident Metal
compiler CPU gain <0.05s over 10s. Retain all samples/PIDs/deltas; keep the camera
as real load, never stop unrelated processes. New cache preparation is deferred
until the slot; no B native job is queued by this specification.

## Capture and near-miss reporting

Trigger only matched RETRACE_DONE -> END_FRAME with **wall >100ms AND calling
thread CPU <10% of wall**, exactly the validated integer inequality. For EVERY
matched phase >50ms, retain one trigger or near-miss summary; CPU-heavy long
phases and 50–100ms phases are near misses and request no window. Report all
counts and matched-phase exposure, not just the worst event. OFF whole-frame
CPU counters are not substituted for the unavailable matched-phase CPU clock.

ON captures 2s before decision through 250ms after, coalescing overlaps with every
trigger frame retained. Existing caps: 65536 cells, 128 bindings, <32MiB fixed
storage, 16 windows, 64MiB output; short span filter 250us. No enlarged ring, extra
firehose, threshold change or automatic follow-up on a rare event. Independently
run `native/tools/game_wait_report.py` on each ON file, hash raw file + report,
and require complete windows/zero missing, dropped, unstable, clipped, invalid,
binding or capped records before boundary attribution. Report the footer's
filtered counts and writer copy/write work. An unavailable public WindowServer
or Mach point sample stays unknown, not a synthesized wait-channel diagnosis.

For every trigger, retain exact phase CPU/wall, CSV interval, packet/ticket and
host-generation bindings; list overlapping token/FIFO/draw-done, interrupt/main
holder, event pump, drawable/submit/present and other named waits. Annotate
request/acquire/return and baton grant/resume separately. Spans overlap: never
sum them. Report process VM deltas only as process context. Causality requires a
matching observed dependency accounting for the gap; RUNNING point samples or
low CPU alone are insufficient. If a needed boundary is absent, report that gap.

## Neutrality screen and stop rules

For each prime/second separately, then pooled warm OFF vs ON, report exposure,
median/p95/p99/max frame interval and main/render CPU where available,
>50/>100/>250ms counts/rates, game-work/retrace/draw-done/token/pipeline/capture
and API waits, graphics preparation timing, compile work and existing tier-m1
score. Preserve whole-run and fixed-window results side by side. No timing
threshold is changed by this diagnostic. Keep each audio report/positive line,
backend, report count and total underruns; existing zero-underrun failures stay
failures even though this is a diagnostic cohort.

Report audio count rates against actual printed report-window exposure with
rounding uncertainty, not invented exact drop times. Compare the paired and
pooled central cost and rare-tail/audio changes without masking outliers. There
is **no automatic neutrality PASS** from four warm samples, even if ON has zero
triggers. PORT reviews this fixed cohort before any larger capture or causal
qualification. Codegen erasure and synthetic tests do not establish runtime
neutrality; differing rare tails may reflect instrumentation, noise or both.

Stop before the next arm on identity/cache/quiet/nice, process, timeout,
incomplete observation, missing real CoreAudio/report minimum (>=50), or history
protocol/integrity/cap failure. Let an already running process self-end where
safe and preserve the incomplete/refused receipt; no attribution or substitute
run. Raw pacing stalls and audio positives alone do not terminate or extend the
predeclared cohort: retaining all eight avoids outcome-dependent exposure.
A requested stop is explicit; the driver records where it stopped and no retry.

Zero triggers is COMPLETE_NO_TRIGGER, not a fix or a root cause. After eight
arms, or a fail-first protocol stop, reap all owned children, verify PIDs and
locks, write RELEASE.json, then hand the M1 to PORT/A. No automatic build,
additional pair or full trace. All previous evidence stays intact.

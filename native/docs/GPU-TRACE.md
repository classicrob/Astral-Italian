# Diagnostic GPU boundary trace

Implemented on the 5.4.3 candidate line, base 7fcc610c, following the approved
GPU-BOUNDARY-TRACE-543-DESIGN.md. Diagnostic instrumentation only: no pacing,
render policy, input/replay, GPU command, or functional-gate change.

## Enable and bind

Default off. Both PETARI_PIPELINE_DIAG=1 and an absolute PETARI_GPU_TRACE path
are required. Also set PETARI_GPU_TRACE_BINDING_SHA256 to the lowercase SHA256
of the preserved JSON command/PREP provenance for the arm. The binding must
identify the exact source commit, executable SHA256/UUID, renderer key,
Aurora/Dawn revisions, command and environment, app/cache/Metal namespace,
prime handoff, scoring window and nice level. The trace header records that
hash, PID, steady-clock start and schema. Existing outputs are never overwritten.
A bad/missing binding disables tracing with a diagnostic; do not treat such an
arm as a trace-on arm. Keep the exact JSON beside the trace and frames.csv.

At each disabled hook the only work is one cached lock-free flag load/branch.
Arguments, timestamps, IDs, TLS context and observer registration are inside the
branch. Configuration happens once before renderer/game/VI initialization.

## Output and limits

A 65,536-entry bounded MPSC ring carries fixed records. Producer insertion makes
at most eight CAS attempts and never waits, allocates, formats, notifies or
writes files. Contention/full/recursive-context overflow increments the explicit
drop counter. A separate non-realtime utility-priority writer formats one JSONL
line per boundary, with a 256 KiB stdio buffer and periodic flush. Drop-free
transport and measured trace overhead are prerequisites for causal use.

Records carry monotonic ns, producer sequence, packet/ticket/submission/span,
thread/queue/FIFO/frontier and CSV/route identity where known. A packet slot or
thread pointer is never treated as a lifetime ID. Packet IDs are Aurora's actual
frameId; thread births bind reused addresses to distinct lifetime IDs offline.
The recorder writes the actual zero-based route CSV row, separately from seam
number. CSV interval markers use the exact existing interval timestamps; no
assumption that CSV, splash, route, present-probe and render frame numbers match.

New boundaries cover request/staging/enqueue/worker/encode/Finish/Submit,
completion-observer registration, queue completion observation, map/copy/ready,
GX interrupt lock/delivery/frontiers/waits, thread readiness/baton/actual CPU
resume, VI due/wake/lock/interrupt/wait return, drawable acquire/Present and
presentation callback observation. Generic worker items without frame identity
are independent; capture items gain their immutable packet/ticket inside the
existing work closure. Trace-only TLS has a bounded stack for inline recursion.

Dawn OnSubmittedWorkDone is a **CPU-observed queue-prefix completion**, including
callback dispatch latency. It adds no wait, polling, GPU query, command or
submission. Map may be observed before queue-done. Submit-to-observation is never
labelled pure GPU execution. Presentation callback observation is also a host
timestamp, not hardware scanout. No Metal swizzle or dependency update is added.

The existing shutdown path closes the writer after frame CSV reporting. It waits
only for the writer, never for GPU callbacks. Process-lifetime callback state
prevents use-after-free; pending callbacks, drops, I/O errors and incomplete
chains remain explicit. An unfinished exit tail does not become a complete
trace. Fatal/forced exits may lack a footer and must be reported as incomplete.

## Analysis

All analysis is file-only; it never compiles or launches an app:

```sh
python3 -B native/tools/gpu_trace_report.py \
  --trace /absolute/trace.jsonl --frames /absolute/frames.csv \
  --binding /absolute/TRACE-BINDING.json --output /absolute/new-report-dir
```

The tool checks producer sequences, interval IDs/continuity/durations (CSV's
0.001 ms rounding only), packet/ticket/submit joins, callback status and drops.
It writes JOINED.jsonl with origin and observed CSV frames, and REPORT.json with
worst rows, overlapping spans and wait/frontier/wake/baton/resume chains. Missing
causal edges remain unresolved. Existing CSV rows, original failures and trace
records are never rewritten. Mapping validity and attribution validity are
separate; no output claims game acceptance. An incomplete trace/chain returns
nonzero. Independent VI events have VI sequence/span identity and observed CSV
intervals, not fabricated render origins.

The report follows the actual waiter/frontier: draw-done may bypass unfinished
capture tokens. Token and draw-done waits stay separate. It never sums capture,
game work, draw-done and present lanes as independent costs. A GPU/queue versus
callback-dispatch ambiguity remains an ambiguity.

## Required trace overhead comparison (not yet measured)

After PORT's server build/tests and SOL B's M1 handoff, use the same immutable
trace-enabled executable for BOTH arms, nice 0, heavy -> native per arm. Retain
unchanged quiet guards, no-input title warm-prime/second protocol, exact cache
and Metal namespace handoff snapshots and fixed first-300-second window.
Set DIAG=1 in both; change only PETARI_GPU_TRACE and its binding variable in the
trace-on second launch. Preserve all individual arms and commands. Alternate
off/on pairs so startup/day drift is visible; do not select a favorable repeat.
The primary unchanged baseline remains separately retained.

```sh
python3 -B native/tools/gpu_trace_report.py \
  --off-frames /absolute/off/frames.csv --on-frames /absolute/on/frames.csv \
  --off-log /absolute/off/app.log --on-log /absolute/on/app.log --seconds 300 \
  --binding /absolute/PAIRED-ARM-BINDING.json --output /absolute/new-overhead-dir
```

Report full samples plus >50 ms count, max, p50/p95/p99, compile-row count and
post-actual-global-100 p99 (null if no actual-100 event exists). Review the whole
paired distributions and warm-burst incidence with PORT. If tracing materially
shifts them, the trace is too heavy: retain that result, reduce instrumentation
cost before diagnosing the burst, and do not call it a game fix. No performance
qualification or acceptance is currently claimed.

## Verification and server handoff

Local M1 verification is restricted to inspected pure Python/source checks:

```sh
python3 -B native/tests/gpu_trace_patch_tests.py /path/to/pinned/aurora
python3 -B native/tests/gpu_trace_analysis_tests.py
```

The patch test checks actual pinned sources, rejects duplicate/bad anchors,
retains original GPU calls/closures and proves that removing the gated native
hooks leaves the baseline operation token streams unchanged. The fake-clock
analysis tests cover delayed callbacks, source identity, bad/missing/duplicate
intervals, drops and overlapping spans. These do not substitute for runtime
proof. No local C++ fixture or native build is allowed on the measurement M1.

PORT server build: configure normally with tests, build petari_gpu_trace_tests
and the native app, and run native_gpu_trace, native_gpu_trace_patch,
native_gpu_trace_analysis plus existing native_app_seam, native_platform_os,
native_platform_vi and native_platform_gx_sync fixtures. The C++ transport test
checks disabled argument non-evaluation, exact fake-operation transcript,
bounded/full/MPSC ring accounting and writer shutdown/JSON output. Inspect the
Release petari_gpu_trace_off_probe symbol for the cached flag load/branch before
any pointer access. Complete runtime callback/CSV mapping and the overhead
comparison before any M1 causal claim. Server C++/app build results are pending.

## Holder / QoS / timer attribution extension

See [GPU-TRACE-HOLDER-543.md](GPU-TRACE-HOLDER-543.md) for schema 2, per-grant
holder durations/reasons, receiving-thread host samples, IODeviceTree topology,
monitor attempt/deadline/backoff spans and startup/callback phase boundaries.
It documents what remains unknown and the deferred native/runtime test plan.
The original ccd0a360 overhead verdict does not qualify this extension.

The source guard alone is insufficient: the38572b67 production compiler
hoisted trace TLS resolvers ahead of worker_main's guard. The noinline enabled
body correction requires the per-build
[Release codegen acceptance](GPU-TRACE-CODEGEN-ACCEPTANCE.md), including the
retained test executable's probe. Do not treat a missing probe or a source-only
check as completed codegen proof.

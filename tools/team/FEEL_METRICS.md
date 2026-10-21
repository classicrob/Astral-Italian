# Offline GX feel diagnostics

`feel_metrics.py` reads existing, quiescent launch/context JSON, frame CSV and app log. It never launches an app, builds code, takes native.lock or starts a performance run. Original artifacts remain unchanged. Output creation is exclusive; before/after input hashes must match. Invalid numeric samples, duplicate/nonmonotonic frames, malformed CSV and stale source/binary context reject rather than becoming zero measurements.

```
nice -n 19 python3 tools/team/feel_metrics.py \
  --launch PRIVATE/launch.json --context PRIVATE/feel-context.json \
  --frames PRIVATE/frames.csv --log PRIVATE/app.log --output NEW_PRIVATE/feel.json
```

Launch JSON uses the existing gate fields source (40-hex commit), binary_sha256 (64-hex), environment. Context requires matching source/binary_sha256 and nonempty case_id. Optional context fields:

- `log_complete`: true only with the completed owned-run log receipt. Absence means observed-event coverage only.
- `cache`: classification fresh/warm/UNKNOWN, verified boolean, inventory_sha256. Only a verified fresh context with a pinned inventory can label measured blocking resolves as fresh-cache events. This extractor records the upstream context assertion; it does not validate SQLite contents or replace the cache gate.
- `contamination`: structured runner observation, including status, owned observation window and competing jobs if known. Missing is UNKNOWN. CONTAMINATED measurements remain labelled and cannot become a clean comparative result.
- `real_audio_verified`: upstream sink/output receipt verified real audio. Merely requesting audio is insufficient. Missing is NOTMEASURED for real output; audio diagnostics can still measure reported counters.

Context is hash-bound with the other input artifacts. Populate it from actual gate observations, never invented defaults. No raw game assets, screenshots or private logs belong in git.

## Schema and interpretation

The output uses schema_version1, status UNQUALIFIED_FEEL_DIAGNOSTIC, qualification BLOCKED and measured_qualified_frames=null. This aligns with Sol D's visual diagnostic envelope while remaining a separate channel: image similarity/SSIM does not establish FPS, responsiveness or audio quality. No aggregate PASS is emitted.

Frame timing is grouped by the literal CSV phase, keeping startup and gameplay separate. Empty phase stays UNKNOWN. Each supported timing column has sample count, mean, nearest-rank p50/p95/p99/max, strictly >25ms count and an exhaustive histogram with upper edges16.667/25/33.333/50/100ms. The supported interval_ms channel is main-loop timing, not inferred GPU duration or displayed image FPS. present_ms is measured only when explicitly populated; present_call_ms is a distinct CPU-call duration. Missing/blank channels remain NOTMEASURED. Occlusion, unfocused frames and EFB capture counts remain visible per phase.

Pipeline diagnostics support the checked-in `[gx pipeline compile]` key/value queue_ms/build_ms/pipeline_api_ms/status records and `[gx pipeline] blocking resolve ... ms`. Async compilation latency is not relabelled as a gameplay stall. Missing timing fields stay NOTMEASURED. Compile/blocking log events without bound frame IDs cannot be assigned to startup/gameplay; that phase is explicitly NOTMEASURED. An event count of zero is reported as covered only with upstream complete-log context, PIPELINE_DIAG=1 and the policy marker. Fresh-cache blocking events require the separate verified fresh inventory; a warm log never becomes fresh coverage.

Audio diagnostics sum ONLY interval reports carrying `since previous report` (underrun frames, replayed blocks, reported seconds). DSP hold reports and supported audio warning/error line numbers remain separate. Missing underrun/DSP counters have explicit NOTMEASURED status and null values; warning-only logs are not zero-underrun evidence. No phase assignment, unlogged time or audible-quality claim is inferred. Audio warning matching is limited to supported audio/audio-dma prefixes, not an exhaustive fault scanner.

Diagnostics overhead is always NOTQUANTIFIED without a matched diagnostics-off measurement. Enabled trace/frame/pipeline/audio/capture instrumentation is listed from the launch environment; frame CSV capture counters also remain available. The tool does not subtract estimated overhead, erase contamination or admit comparative performance. Later clean feel measurements require their own uncontended owned runtime window and readiness/provenance gate.

Run asset-free controls:

```
nice -n 19 python3 -m unittest tools/team/test_feel_metrics.py
```

Real gate receipts also configure CSV writers, editor-mouse tracing, input logging and F9 capture directories. These are now retained in the configured diagnostic-overhead list; a capture-directory setting does not claim continuous capture. SDL_AUDIO_DRIVER/SDL_AUDIODRIVER selection is retained separately, including dummy versus DEFAULT. This does not infer actual hardware output without the bound sink witness.

CSV startup samples are not full launch-to-first-present latency: renderer initialization or seed precompilation can precede frame1. startup_wall_latency remains explicitly NOTMEASURED without bound process-launch/first-present timestamps. Low CSV startup p95 and zero blocking-resolve events do not establish hitch-free or quick initial preload.

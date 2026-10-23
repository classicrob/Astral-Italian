# Ubershader spike results — 2026-10-23

The hybrid path works on the M1: covered draws render with a boot-prepared generic
shader while their specialised pipeline compiles, then switch to the specialised
pipeline. This spike does **not** eliminate all cold stalls. Forced-all rendering
is substantially over the 60 Hz GPU budget, and cold fallback preparation adds a
synchronous startup hold. The flag remains off by default; landing is the PORT
lead's decision.

Implementation: `port/ubershader-spike`, through `f7f545d4`. Measurement cohorts
used the frozen `c917ec61` app. The final commit only drains GPU timestamp rows at
native exit; it changes no shader or pipeline scheduling. M1 Max, 64 GiB, macOS
15.1, Xcode 16.1; the image pairs use 2560×1440 surfaces, scenario 1.

## Coverage and image comparison

SOL 2's valid, completed `harn-v5r2-*-{off,all}-still1` pairs use the same binary,
970-frame stationary input script and capture schedule. Player/camera trajectories
match. The Yoshi stationary off/off control is byte-identical (maximum and means
zero). Earlier moving-script off/off runs diverged and were excluded from shader
attribution. IslandFleet's attempted fourth pair did not complete its input script
and contributes no successful comparison.

| Stage | Covered recorded draws | Draw coverage | Covered shader configs | Covered seed rows | Max RGB diff | Mean pixel-max diff | Mean RGB diff |
|---|---:|---:|---:|---:|---:|---:|---:|
| YosshiHomeGalaxy | 748,243 / 781,133 | 95.7894% | 152 / 160 | 168 / 183 | 15 | 0.001434860 | 0.000633108 |
| MokumokuValleyGalaxy | 1,309,034 / 1,335,636 | 98.0083% | 148 / 157 | 197 / 206 | 20 | 0.000533908 | 0.000286482 |
| MarioFaceShipGalaxy | 720,367 / 724,847 | 99.3819% | 71 / 74 | 101 / 106 | 158 | 0.003260001 | 0.001964615 |

All image differences are absolute 8-bit RGB code values (0–255); alpha is ignored.
The two means respectively average each pixel's largest channel difference and
all RGB channel differences. Rendering is not bit-exact. The Starship maximum is
one pixel in frame 780 at (902,317), on a wood/log silhouette against a bright
background. In that frame only 87 of 3,686,400 pixels differ by more than 8, and
only one differs by more than 16. Edge/rounding sensitivity is a plausible cause,
not a proven attribution. Across Starship captures, 0.1857% of pixels change.

Recorded draws count merged GX draw commands and include the stage process's
startup/UI. Runtime config counts hash ShaderConfig; seed counts are PipelineConfig
rows, which can include multiple fixed-function states for a shader. No missing
fallback keys were observed in these successful comparisons. Uncovered draws were
fog/indirect configurations and retained the existing behaviour.

Heat maps and raw comparator JSON are under:

- `~/MacioGalaxy2-team/scratch-port/gate53/harn-v5r2-yoshi-diff/`
- `~/MacioGalaxy2-team/scratch-port/gate53/harn-v5r2-fluffy-diff/`
- `~/MacioGalaxy2-team/scratch-port/gate53/harn-v5r2-ship-diff/`

## True-cold stalls and hybrid usage

The matched, capture-free SOL 2 Yoshi cases are `harn-v5-yoshi-off-cold1` and
`harn-v5-yoshi-1-cold1`. These are one pair, not a repeated statistical estimate.

| Measurement | Flag off | Hybrid (`1`) |
|---|---:|---:|
| Gameplay CSV frames | 1,967 | 1,967 |
| Gameplay intervals over 100 ms | 2 | 1 |
| Worst gameplay interval | 4,943.872 ms | 735.382 ms |
| Median gameplay interval | 16.660 ms | 16.669 ms |
| 95th percentile gameplay interval | 18.645 ms | 17.619 ms |

The count of long intervals fell by 50%; the worst interval was 85.1% shorter.
One substantial cold hold remains. Unsupported configurations can still wait or
skip, so this is not stall elimination or complete-scene coverage.

In that same hybrid case, 306 / 2,428 encoded frames used at least one fallback
(12.603%). A join to distinct gameplay `presented_frame` IDs finds 300 / 1,967
(15.252%); the encoded denominator also includes startup and non-GX frames.
It counts render-worker begin-frame packets, including an unsubmitted exit tail.

An independent SOL 1 moving-input cold/warm pair,
`uber-core-final8b-yoshi-{cold,warm}`, completed 970 frames per run. Cold draw
coverage was 741,780 / 777,752 (95.375%); 134,515 fallback draws occurred in
313 / 2,428 encoded frames (12.891%). Eight unsupported draws were skipped across
four configurations; one 718.188 ms gameplay interval contained 699.970 ms of
pipeline waiting. Warm used zero fallback frames, had zero pipeline waits and
zero gameplay intervals above 100 ms. These moving-input cases are not substituted
for the matched SOL 2 stationary pair.

## Compile and boot cost

The independent cold/warm pair uses the same signed app and private Metal cache
identity `dev.petari.Petari.perfubercore8`. The warm run copies the cold process's
Petari cache and preserves its own Metal cache; another agent's cold run cannot
clear that private identity.

| Synchronous work for 103 fixed-state variants | Cold | Warm |
|---|---:|---:|
| WGSL generation + CreateShaderModule | 8.895 ms | 0.312 ms |
| CreateRenderPipeline | 3,181.232 ms | 35.582 ms |
| Sum | **3,190.127 ms** | **35.894 ms** |

The cold slowest individual pipeline call was 802.216 ms. Metal compilation is
primarily paid in CreateRenderPipeline, so the small module number is not the
complete shader compilation cost. These sums exclude seed database scanning and
other boot work. The implementation performs this preparation synchronously:
the cold cost is a real startup hold, not a removed cost.

The existing pipeline workers already create Dawn/Metal pipelines concurrently,
so preparation could be queued on reserved workers during boot/loading. That
asynchronous scheduling is not implemented by this spike, and a cold first
pipeline taking roughly 0.8 seconds is not guaranteed to finish before the first
menu draw. When a covered draw lacks a ready fallback key, today's specialised
wait/skip policy remains active; an unready fallback is never selected.

## GPU cost

SOL 2's capture-free matched Yoshi GPU cases are
`harn-v5-yoshi-off-gpu1` and `harn-v5-yoshi-all-gpu1`. The binary, stage and
970-frame stationary script match. CSV values are actual encoder begin/end GPU
timestamp differences, not CPU frame-cadence estimates.

| Whole-process encoded GPU-clock span | Specialised | Forced all |
|---|---:|---:|
| Valid timestamp rows | 2,425 | 2,368 |
| Median | **3.580695 ms** | **34.513973 ms** |
| 95th percentile | 4.293685 ms | 37.909797 ms |
| 99th percentile | 4.503431 ms | 43.388963 ms |

Median span is 9.64× larger in forced-all and exceeds the 16.67 ms budget. The
sample scope includes loading. Forced-all had 55 `readback_busy` rows; both cases
had one invalid timestamp. Neither missing nor invalid durations were invented.
`gx_ms` is unavailable because render passes can mix GX and other commands.

Whole-frame GPU-clock spans include gaps between submissions. A cold CPU pipeline
wait can therefore appear inside the timestamp interval; the specialised case's
4,890 ms maximum is not evidence of 4,890 ms of active GPU work. These measurements
are elapsed GPU-clock spans, not utilisation or summed active-pass execution time.
The identical input-state trajectory was separately verified for this GPU pair.

The measured c917 cohort skips Aurora shutdown on normal native `_Exit`, leaving
2 specialised or 4 forced-all tail readbacks absent from its GPU CSV. That issue
is fixed in f7f545d4: an env-gated render-worker drain runs at the native exit
report. A separate 120-frame script produced 1,578 encoded frames and exactly
1,578 unique contiguous CSV rows: 1,576 valid, one invalid and one explicitly blank
`incomplete_frame` for the unsubmitted tail. CSV columns remain
`frame,gpu_ms,gx_ms,timestamp_status`.

## Limits and validation

- The supported path includes ordinary direct/indexed vertex decode, matrix
  texgen, material/ambient and none/spot/specular lighting, TEV arguments,
  arithmetic/comparison, bias/scale/clamp, konst, swaps, register routing and
  alpha compare. The coverage predicate explicitly rejects unsupported forms.
- Fog, indirect stages, line/point expansion, NBT3 and other texgen paths remain
  on the existing renderer. No broad GX correctness claim follows from three
  stage comparisons. Other target layouts/output variants have source validation
  but were not all exercised by these native scene runs.
- Generic shading carries a second 4,512-byte uniform per distinct uniform
  update. Its GPU cost is the main observed risk; transient hybrid use is much
  cheaper than forcing it indefinitely, but can still hurt a busy frame.
- Unknown fixed-state keys retain the original policy. Seed-derived precompile
  coverage is not a guarantee for every future stage/state.
- Existing specialised shader regressions pass, admission-boundary tests pass,
  and headless Dawn validates all five legal generic output variants (zero
  errors). The final native exit-drain test completed without deadlock or WebGPU
  validation errors. Fresh/incremental builds used the prescribed M1 flags.
- Source changes are confined to native code, build-time patches and native test/
  documentation files. Pinned Aurora source, frozen play builds, user saves and
  user caches were not edited. No push or landing was performed.

Implementation details and exact log formats: `UBERSHADER-SPIKE.md`. SOL 1's
retained evidence is under `~/MacioGalaxy2-team/scratch-port/gate53/uber-build/`;
SOL 2's raw case metadata/CSVs and comparator output retain their provenance.
Own case logs, frame CSVs and metadata were copied to `uber-build/evidence/`,
whose `index.json` records original paths and file hashes. Own build directories,
case caches and temporary prep arms were then removed; 314 GiB remained free.
SOL 2's `harn-*` data was not removed. Two signed apps remain in that evidence
directory for the lead, with SHA256 of `Contents/MacOS/Petari`:

- `c917ec61-Petari.app`: `1e23c96c2cec6d3643b1d1e395505e963c86d97d9508b3ae3c5b3dc441171cf7`
- `f7f545d4-Petari.app`: `72c4ec4e09366d5d2548e64912489fdc8cfcbdffbc586454857f979784058a1a`

They retain private perf bundle identities. Reproduce with `cold533.sh prep` on
copies and fresh user/cache directories, using a private `PERF_ID` consistently
for any warm measurements.

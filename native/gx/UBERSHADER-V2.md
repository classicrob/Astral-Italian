# Ubershader V2 renderer experiments

## Milestone (a): cost and variant plan

Measured on the M1 with a single executable, private bundle ID
`dev.petari.Petari.perfuberv2abl1`, both shared locks, cold533.sh stage mode,
Yoshi, and the unchanged 970-frame stationary script. The signed executable SHA256
is `569aba8361a51f4e1bffd00fae6a3b4ff64d3144be75aad531e891dac7c6127c`.
The renderer is the V1 interpreter plus diagnostic ablations, commit 44b75587.

| Mode | Valid GPU samples | Whole-run median ms | Gameplay median ms |
| --- | ---: | ---: | ---: |
| Specialised | 2426 | 3.5375 | 3.8397 |
| Full interpreter | 2411 | 33.8140 | 34.2841 |
| Fragment removed | 2426 | 3.9952 | 4.4283 |
| Texture sampling removed | 2426 | 25.3248 | 25.7822 |
| Vertex lighting removed | 2413 | 32.4894 | 33.6489 |

The full interpreter costs 9.56x the specialised whole-run median. Removing
fragment work removes almost all of the excess. Removing texture sampling still
leaves a 25.32 ms median, so sampling alone cannot explain the cost. These are
non-additive diagnostics: they change the image, alpha/discard decisions and
potentially compiler elimination of unused varyings. They establish where to
optimize, not an exact attribution of active GPU cycles. GPU timestamps are actual
whole-frame encoder spans; invalid samples and readback gaps are excluded. The
large maximum spans include CPU pipeline waits between submissions.

First variant key: exact TEV stage count and sampled texgen extent (highest used
texcoord + 1; preserve holes), in addition to every existing fixed pipeline field
and render-target layout. The shipped 64 databases contain 16 such count pairs;
Yoshi has six, Fluffy eight and Starship ten. These are seed configuration counts,
not draw coverage or a promise about total fixed pipelines. No per-material colors,
matrices, lights, TEV arguments or operations enter this first key.

Generate straight-line TEV stages, four argument evaluations and texgens; keep
configuration values in uniforms. Replace dynamically indexed local TEV register
and argument arrays with explicit register selection/writes so Metal need not
materialize per-fragment arrays. Skip samples that Aurora's texture-use predicate
says a stage does not use. Measure this first implementation against specialised
rendering before choosing any additional variant dimensions. In particular, only
consider baking texture binding dispatch if it remains a measured bottleneck.

SOL 2 owns concurrent claims/publication, boot enumeration, reserved-worker
scheduling, draw readiness and counters. prepare/key/ready API signatures stay
unchanged; SOL 1 owns the variant key, shader/module selection and packing. Boot
and readiness measurements follow integration. Fog and indirect coverage follows
the GPU-cost work, using Aurora's exact formulas and rounding.

Evidence and scripts: `~/MacioGalaxy2-team/scratch-port/gate53/uber-v2-build`.
Cases: `~/MacioGalaxy2-gamedata/port53/uber-v2-abl1-{off,full,fragment,textures,lighting}`.
The vertex-lighting control remains close to the full interpreter. All five output
variants and all three diagnostic modes passed Dawn Null WGSL validation;
the specialised shader regression and existing coverage checks passed.

## Hybrid gate clarification and wait attribution

The lead replaced the forced-all proxy with the actual gate: every hybrid frame
using any fallback must be below 20 ms GPU. Further specialization was paused to
measure cold hybrid play, then residual coverage takes priority; optimization of
Fluffy's fallback-heavy entry burst follows coverage.

On c7feaceb, true-cold private bundle, default 970-frame moving script:

| Stage / presented gameplay frames | Valid n | Median ms | p95 ms | p99 ms | Maximum ms | >=20 ms |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Yoshi, fallback | 324 | 12.6302 | 14.0252 | 15.6034 | 616.6096 | 1 |
| Yoshi, non-fallback | 1643 | 4.1028 | 5.8609 | 7.8753 | 26.4806 | 2 |
| Fluffy, fallback | 301 | 8.7572 | 21.2464 | 36.3924 | 891.6632 | 19 |
| Fluffy, non-fallback | 1619 | 3.1059 | 3.7822 | 4.0253 | 8.3591 | 0 |

The Yoshi outlier overlaps a 592.237 ms pipeline wait; Fluffy's first overlaps
889.162 ms. Fluffy's other 18 outliers (481–497 and 522) have zero pipeline wait,
so GPU cost still needs attention after coverage. The full count-variant forced
comparison was 30.7014 / 3.5611 = 8.62x, with a 15.848 s synchronous API compile
sum for 206 fixed variants. Unrolling alone did not meet the original proxy.

Frame domains are explicit: packet IDs start at one, GPU query indices at zero.
The packet begin callback calls the GX counter immediately before the query begin;
query indices increment even when readback is busy. Both cold runs have complete,
unique, contiguous query index ranges equal to encoded_frames, and event counts
match fallback_frames. The mapping is `packet ID = GPU CSV frame + 1`.
Presented-frame telemetry can overlap a wait on the upcoming recording packet;
the instrumented complete-scene waits below name recording packet 481, whereas
the presented/GPU outlier is 480. No GPU duration is inferred for invalid rows.

Diagnostics-only 5ad2368e preserved the same renderer/scheduler and logged the
pending vector before waiting. The Yoshi repeat waited 814.871 ms on these six
unique pending full PipelineConfig hashes:

| Full config hash | ShaderConfig hash | Rejection |
| --- | --- | --- |
| ac23e4212cdde693 | 5fd948b5fa8a5523 | indirect |
| 3d9f3cc056e7319e | 8295104db37885df | indirect |
| e08507645e20c20a | 64f7cba6c7571b7b | indirect |
| 754fd7e38ac30abc | cd450e9dadf90905 | indirect |
| b8ca8ce7586c7698 | 7c139a7026e92e4a | indirect |
| fbc1432f80d19d8a | 6db02a3b59cb9cf8 | fog (type 2), also indirect |

All six have one indirect stage, fallback_key=0 and fallback_ready=0. The original
592.237 ms run's four exit-miss hashes are the first three plus b8ca8ce7586c7698;
the other two had already completed before entry. That original log did not retain
the complete pending vector; the repeated direct trace is distinguished from the
original membership reconstructed from misses and completion ordering.

Fluffy's repeat waited 1203.374 ms on seven unique pending full hashes, all with
one indirect stage, fog type zero, rejection `indirect`, and no fallback association:
`1c2b12cd7e9c2f4a`, `a818eba8ee199f79`, `2f592dd1c5520404`,
`0971706f85f15af1`, `d0563da53c58b523`, `69d09007a92e8f46`,
`9238f16d1bc19d17`. They also exactly match the original Fluffy exit-miss set.
No other feature rejection class appeared in either repeat's pending set.

SOL 2 async commit ffa39f2f was cherry-picked alone as e5a335c9; its seven
compiled fake-GPU scheduling scenarios pass. Full integration runtime checks follow.


## Coverage and integrated scheduling baselines

Exact fog and indirect coverage landed in 2a9b27fb. The packed draw buffer is
4768 bytes. Fog uses Aurora's reversed-Z depth, perspective/orthographic equations,
range adjustment, exp2 modes and ordering before final TEV overflow. Indirect uses
its format rounding, bias, static/dynamic matrices, coordinate scaling/wrap/add-prev
and alpha-bump equations. Unsupported vertex/texgen modes still reject explicitly.
The three stage seed sets are covered 183/183, 206/206 and 106/106 configurations.

The a0140447 integration (int1) has asynchronous preparation on reserved workers.
The c9d27887 integration (int2) adds all-worker preparation and active-stage priority.
Both use exactly the same renderer and private bundle IDs. The default moving
970-frame scripts completed; measured draw coverage is 100% on all three stages.
These are separate cold runs whose fallback composition changes with scheduling;
GPU medians alone are not a renderer speedup ratio.

| Integration / stage | Covered draws / total | Gameplay >100 ms | Maximum gameplay ms | Fallback GPU median / p99 / max ms | Fallback GPU >=20 ms |
| --- | ---: | ---: | ---: | --- | ---: |
| int1 Yoshi | 777093 / 777093 | 1 | 1104.947 | 13.519 / 19.620 / 32.695 | 8 |
| int1 Fluffy | 1125414 / 1125414 | 1 | 1780.497 | 9.912 / 31.282 / 41.038 | 92 |
| int1 Starship | 718369 / 718369 | 2 | 906.708 | 13.160 / 24.664 / 901.181 | 289 |
| int2 Yoshi | 779657 / 779657 | 0 | 51.614 | 13.976 / 28.777 / 31.196 | 296 |
| int2 Fluffy | 1127196 / 1127196 | 0 | 40.651 | 9.484 / 40.120 / 40.474 | 166 |
| int2 Starship | 717707 / 717707 | 0 | 57.622 | 24.284 / 28.336 / 29.978 | 397 |

Int2 removes the observed gameplay waits, but fails the GPU gate. All 293 fallback
pipelines complete without failed or unknown keys. Boot only enqueues (no synchronous
pipeline compiles); an unready fallback retains the existing wait/skip behavior and
is counted separately from failed and unknown keys. The all-worker policy can delay
first title recording; SOL 2's follow-up reserves two workers until that recording,
then releases preparation to all workers and emits `event=prep_release`.

Int1 stationary specialised/forced-all image comparisons, same signed executable,
identical 970-row input traces and 15 captures per run:

| Stage | Mean RGB difference /255 | Maximum channel difference /255 |
| --- | ---: | ---: |
| Yoshi | 0.000633108 | 15 |
| Fluffy | 0.000297966 | 91 |
| Starship | 0.001964615 | 158 |

Yoshi and Starship are unchanged from V1's differences. Starship's largest outlier
is the existing sparse silhouette edge. Fluffy's larger maxima occur at one or two
pixels at the end of a bright water-reflection streak (for example frame 360,
pixel 781,760); its off/off control is exactly zero. This is a shader-associated
boundary difference, consistent with floating-point/indirect quantization, rather
than ordinary capture noise. Its precise arithmetic cause has not been proved.
These image results validate int1, not the later optimization below.

## Measured heavy-frame optimization

Profiling lives on a separate branch/app and is excluded from integration apps.
Counters record actual fallback draws, physical draws including alpha prepasses,
vertices, TEV/texgen/indirect/fog variants and fixed state per encoded frame. They
cannot measure fragment overdraw; blending and disabled depth writes are only
indicators. A Fluffy sample has 82.6% one-stage fallback draws and 77.5% with texgen
extent one. A second sample's top eight complete argument/routing programs account
for 92.2% of fallback draws and 95.1% of fallback vertices in heavy frames.

Same-binary stationary forced-all diagnostics (last 500 valid samples) give full
41.512 ms, argument-selector dispatch removed 23.814 ms, identity swizzles 31.790 ms,
operation dispatch removed 40.441 ms, konst removed 45.856 ms and texture binding
dispatch removed 40.376 ms. These image-changing ablations are non-additive and do
not establish exact cycle shares. The completed controls do not justify adding a
texture-binding-mask dimension to the key.

Renderer 6baaacea instead adds two bounded dimensions: exact matches for the eight
measured argument/output-register programs, and an identity-swap flag. The classifier
compares every full enum and register selector. Unknown programs retain the generic
interpreter; dynamic TEV operations, clamp/bias/scale, constants, lighting and all
uniform values keep their original equations. Identity specialization requires all
referenced swap tables to be identity. The complete 64-database audit counts 11608
configs, 33 explicit rejections, 337 fixed keys (previously 293), 61 shader variants,
3470 fast-program configs and 11560 identity-swap configs. The bounded key adds 15%
fixed pipelines rather than baking whole materials.

Int3 is renderer 6baaacea plus SOL 2 scheduling commit 6072fd6d, cherry-picked as
b0bd3235. Its signed executable SHA256 is
`d6de4bfedd407ad99ca5319224b3104b3f4cafaea143e0cd4d43406edcb54870`.
The immutable app is `uber-v2-build/evidence/b0bd3235-Petari.app`; the private bundle
ID is `dev.petari.Petari.perfuberv2int3`. The app and shader-test builds pass,
300 generated WGSL variants validate without errors, classifier/coverage checks
pass, and SOL 2's ten compiled scheduling scenarios pass. Cold gate and new image
results must be evaluated on this frozen app before claiming the optimization passes.


### Int3 cold hybrid gate

The same frozen app completed all three default moving scripts without WebGPU
errors. Draw coverage remains 100%; no failed or unknown fallback keys occur.
GPU query index domains and fallback event counts were checked for every run.
The strict gate **fails** despite a substantial reduction in sustained GPU cost.

| Stage | Covered / total draws | Covered / total observed configs | Fallback gameplay GPU n | Median / p95 / p99 / max ms | GPU >=20 ms | Gameplay >100 ms |
| --- | ---: | ---: | ---: | --- | ---: | ---: |
| Yoshi | 778454 / 778454 | 167 / 167 | 1481 | 12.995 / 15.644 / 16.895 / 319.564 | 6 | 1 |
| Fluffy | 1126188 / 1126188 | 162 / 162 | 1456 | 12.352 / 17.758 / 18.658 / 24.510 | 7 | 0 |
| Starship | 718065 / 718065 | 79 / 79 | 1211 | 12.536 / 14.316 / 15.722 / 910.983 | 3 | 3 |

Yoshi's frame 480 waits 293.577 ms for covered but pending fallback keys; its
maximum gameplay interval is 310.231 ms. Starship waits at frames 480/482/484
(891.666/456.960/100.300 ms), with maximum gameplay interval 909.519 ms.
These waits explain all three Starship GPU outliers. Every logged residual config
is covered with an associated fallback key, but that key is not ready; this is
preparation timing, not an unsupported fog/indirect feature or missing association.
Yoshi has five additional zero-wait GPU outliers (maximum 25.624 ms), Fluffy seven
(maximum 24.510 ms). Fluffy has no gameplay interval over 100 ms (max 31.041 ms).

Int3 prepares 337/337 pipelines successfully in each run. Pending draw attempts /
skipped-unready draws are Yoshi 9/9, Fluffy 75/6 and Starship 49/36. Its scheduling
handoff is observable: Fluffy's title first recording at 5273.489 ms has a fallback
ready, then `prep_release` at 5273.606 ms releases all four compile workers.
This does not imply all title keys are ready (15 remain pending at that moment).
SOL 2 owns cold/warm readiness comparison and the preparation regression.

Evidence: `int3.json`, `gpu-int3-{yoshi,fluffy,ship}-cold.csv`,
`measure-integrated3.sh`, `summarize_integrated3.py`; cases
`uber-v2-int3-{yoshi,fluffy,ship}-cold`. Shader, cache and source provenance remain
in each case. Same-app image comparison passes: all three int3 aggregate differences exactly
match int1 (table above), with identical 970-row trajectories and 15 captures per
run. Evidence is `diff-int3-{yoshi,fluffy,ship}/diff.json`. Further workload profiling
uses a separate branch and private app, never this immutable int3 app.


## Int4: two-tier readiness safety net

Renderer d26ec34c provides `PetariUberKeys {generic, fast}` and
`PetariUberTier::{Generic,Fast}`. The existing two-argument `key`/`prepare`
functions remain preferred-Fast wrappers, with three-argument tier overloads.
Generic clears the identity/program bits, restoring int2's 293 count/fog/indirect
keys. The 337 Fast keys share ten with Generic, for 620 distinct keys. Creation
claims and publishes under the actual tier key, with a saved/restored thread-local
shader tier. Both tiers share the unchanged 4768-byte uniform layout.

A specialised reference associates both keys. Lookup selects READY Fast, else
Generic; draw and complete-scene readiness use that same choice. A pending upgrade
cannot introduce a wait when Generic is ready. When neither is ready, the previous
wait/skip behavior remains. A draw also enqueues/promotes its upgrade when only
Generic is ready. Aliased keys queue once as Generic.

SOL 2 scheduler b92e6e78 is integrated as a9462997. Before FileSelect recording,
two workers prepare front-end Generic only. After that recording, all four workers
use this order: draw-blocking specialised work, active-stage Generic, active-stage
Fast, front-end/demand Generic, unrelated Generic, remaining Fast, then speculative
specialised backlog. The scheduler exposes per-tier readiness and completion.

Exactly one final `[gx uber frame]` event is emitted at encoder packet end when it
used any fallback, appending `generic=0|1 fast=0|1`. A mixed packet has both flags.
`[gx uber frames]` appends inclusive `generic_frames`, `fast_frames`, `mixed_frames`;
`generic + fast - mixed = fallback + forced`. `[gx uber tiers]` supplies cumulative
`generic_draws` and `fast_draws`. The final packet does not depend on a subsequent
begin to flush its flags. GPU CSV columns remain
`frame,gpu_ms,gx_ms,timestamp_status`, with `packet ID = CSV frame + 1`.

The frozen int4 app is `uber-v2-build/evidence/a9462997-Petari.app`, executable
SHA256 `142014c403dbecd140ece7b33e2e3a4a105857ffabf7ab29ead58cc736ef44c0`,
private ID `dev.petari.Petari.perfuberv2int4`. Deep/strict code signing verifies;
full app/test build, 300 WGSL variants, specialised regression, classifier/key
checks, 14 scheduler scenarios and actual tier-selection/packet-accounting tests
pass. Same-app cold and image gates are still being collected. This preparation
safety net by itself does not make Generic rendering cheap enough for the GPU gate.

A separate prof3 app (int3 renderer plus diagnostic instrumentation) completed
same-binary stationary Fluffy forced-all controls. Every last-500 valid GPU sample
is proven forced and all 970-row input traces match: full 22.303 ms, remaining
argument dispatch removed 17.077 ms, operation dispatch removed 20.176 ms, texture
samples removed 18.331 ms, lighting removed 22.001 ms. These diagnostics change the
image and are non-additive. The largest remaining removable component is still
argument dispatch; adding vertex-lighting variants is not supported by this result.
Evidence: `subfeatures3.json` and `measure-subfeatures3.sh`.


### Int4 measured cold results and 5.4 release decision

Binding head steer: hybrid ubershaders ship default ON in 5.4, with a persistent
Display-menu On/Off toggle and explicit environment `0` override. The 20 ms GPU
line becomes the v3 target. Release conditions remain no gameplay frame >100 ms
on the three gates, warm play unchanged, and SOL 2's 61-galaxy image comparison;
up to +1 s cold title latency is accepted. Int5 is one isolated uniform-program
Generic experiment, then SOL 2 gets one pre-title worker-policy pass for int6.
There are no further optimization boxes after int6.

Int4's three default moving scripts completed with 100% observed draw/config
coverage, zero failed/unknown keys and 620/620 prepared fallbacks. Packet flags,
inclusive counters and the complete contiguous GPU query mapping all validate.

| Stage | Covered / total draws | Observed configs | Gameplay >100 ms | Maximum gameplay ms |
| --- | ---: | ---: | ---: | ---: |
| Yoshi | 779746 / 779746 | 167 / 167 | 0 | 46.004 |
| Fluffy | 1126938 / 1126938 | 162 / 162 | 0 | 44.817 |
| Starship | 717663 / 717663 | 79 / 79 | 0 | 84.922 |

| Stage / fallback packet class | Valid gameplay GPU n | Median / p99 / max ms | >=20 ms |
| --- | ---: | --- | ---: |
| Yoshi / generic_only | 19 | 7.392 / 12.263 / 16.324 | 0 |
| Yoshi / fast_only | 1737 | 13.265 / 17.498 / 27.398 | 4 |
| Yoshi / mixed | 209 | 15.466 / 20.899 / 27.369 | 46 |
| Fluffy / generic_only | 106 | 32.534 / 33.601 / 40.395 | 84 |
| Fluffy / fast_only | 1679 | 11.929 / 22.408 / 26.083 | 210 |
| Fluffy / mixed | 57 | 19.166 / 31.983 / 32.205 | 16 |
| Starship / generic_only | 164 | 22.051 / 24.419 / 85.322 | 102 |
| Starship / fast_only | 29 | 6.172 / 10.991 / 11.571 | 0 |
| Starship / mixed | 1019 | 17.498 / 23.931 / 24.778 | 54 |

Starship still has a 59.937 ms frame-480 entry wait, which overlaps its 85.322 ms
Generic-only GPU outlier. Its other GPU spans have no >100 ms outlier. Title
first-recording times are 5464.980/5034.311/5069.049 ms (Yoshi/Fluffy/Starship),
with fallback ready 1/0/0 and title Generic pending 0/23/22. Generic-all readiness
is 23679.330/24047.508/22785.379 ms; all-tier readiness is
32902.491/33168.445/32217.900 ms. No synchronous boot compiles occur.

The Generic leaf worker-time sums are 62765.930/62744.879/60215.092 ms across
293 keys, with medians 0.807/0.878/0.870 ms and p95 958.006/849.641/1040.037 ms.
These sums overlap across workers and are **not boot holds**. Cached driver
functions can be reused among distinct fixed-state keys inside a true-cold run.
Each run creates 39 Generic modules; their source-assembly-plus-module-API time
sums are 294.257/294.348/303.765 ms. Legacy per-key `pipeline_ms` is residual leaf
time after timed source/module creation, including module mutex waits; it must
not be described as a pure pipeline API timer. Detailed per-key/module evidence
and readiness snapshots are in `compile-int4.json` and `summarize_tier_compile.py`.
GPU and stall evidence is `int4.json` plus the stage case directories.

Yoshi and Fluffy same-app stationary image comparisons again exactly match int1
and int3; Starship is pending. Warm play and the 61-galaxy comparison remain
release conditions, not inferred passes from these three cold measurements.

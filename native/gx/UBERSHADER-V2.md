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
match fallback_frames. The GPU query index therefore maps to packet ID +1.
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

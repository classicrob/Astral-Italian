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
A vertex-lighting ablation is still completing as a control. All five output
variants and all three diagnostic modes passed Dawn Null WGSL validation;
the specialised shader regression and existing coverage checks passed.

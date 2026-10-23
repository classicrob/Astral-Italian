# Opt-in ubershader spike

`PETARI_UBERSHADER=1` queues a covered draw's specialised shader without waiting
when its fixed-state fallback was prepared at boot. The encoder selects the
specialised pipeline if ready and uses the fallback otherwise.
`PETARI_UBERSHADER=all` selects the fallback for covered draws with prepared keys.
Unset, `0`, and other values leave the existing specialised path active.

The build patches Aurora sources into the build directory. The pinned source
checkout is never edited. A single generated copy of the GX pipeline header is
shared by all renderer translation units that use the draw payload. Its one
additional u32 offset fits the existing 128-byte inline command limit.

The generic shader reads a fixed 4,512-byte uniform: 1,024 packed config bytes,
projection, position/texture and normal matrices, TEV registers, eight lights,
material/ambient colors, konst colors, post matrices and texture size/LOD bias.
The specialized uniform stays available for draw-time pipeline switching. Uniform
binding windows grow from 3,840 to 5,120 bytes only with the flag enabled.

The predicate in `ubershader_runtime.inc` is the coverage authority. The spike
covers triangle geometry, direct and indexed ordinary attributes, direct matrix
indices, matrix texgen from position/normal/texture attributes, material and
ambient sources, no-attenuation/spot/specular lighting, TEV arithmetic and comparison,
register routing, swaps, konst and alpha compare. It rejects fog, indirect stages,
line/point expansion, NBT3, emboss/color texgen and unsupported attenuation modes. Rejected
configurations retain the existing pipeline scheduling policy. No configuration
is silently approximated. A covered shader can still miss the boot-prepared
fixed-function key; this is a separate counter.

Boot preparation reads the shipped stage seed databases and any current-build
known configurations. Fixed keys clear the entire ShaderConfig and retain every
other PipelineConfig field plus RenderTargetLayout. Shader modules are cached
only by destination-alpha mode and normal attachment. Specialised-cache keys,
seed schemas and persistent pipeline configurations are unchanged.

Logs are cumulative per process. `config` is a hash of ShaderConfig:

```
[gx uber config] config=<hex> covered=<0|1> reason=<token>
[gx uber shader] mode=<u> normal=<u> source_bytes=<n> shader_ms=<ms>
[gx uber compile] key=<hex> layout=<hex> shader_ms=<ms> pipeline_ms=<ms> success=<0|1>
[gx uber frame] frame=<encoded frame id> fallback=1 forced=<0|1>
[gx uber frames] encoded_frames=<n> fallback_frames=<n> forced_frames=<n>
[gx uber summary] draws=<n> covered_draws=<n> fallback_draws=<n> forced_draws=<n> specialized_draws=<n> unsupported_draws=<n> missing_key_draws=<n> configs=<n> covered_configs=<n>
```

The frame event is logged once on the first generic draw of each encoded frame.
`fallback_frames` counts hybrid frames; `forced_frames` counts forced-all frames.
Join the frame IDs to the frame CSV’s `presented_frame` for phase attribution;
`encoded_frames` includes startup and frames without any GX draw.

Coverage counts recorded draw commands (merged geometry remains one command),
while selection counts encoder invocations. They can differ when a recorded draw
never reaches encoding. Shader compile time is module generation plus Dawn's
CreateShaderModule call; pipeline time is the synchronous CreateRenderPipeline
work, excluding newly created modules. Neither is an asynchronous GPU duration.

`PETARI_GPU_TIMING_CSV=<path>` requests TimestampQuery and activates Aurora's
existing timestamp/readback ring without requiring Tracy. CSV columns are
`frame,gpu_ms,gx_ms,timestamp_status`; `frame` counts encoded frames. GPU duration
uses the actual encoder begin/end queries. Invalid or stale timestamps produce
blank measurements, never reconstructed timings. `gx_ms` stays empty because
Aurora render passes can mix GX and other commands. `readback_busy` marks skipped
measurements when the four-slot ring is full. Feature availability is logged.
The native exit report drains queries on the render worker because normal native
exits use `_Exit` and skip Aurora shutdown. An unsubmitted final frame receives an
`incomplete_frame` row. Query-frame indices begin at zero; encoded packet IDs begin
at one in a fresh process. Whole-frame GPU-clock spans include gaps between
submissions, so a CPU pipeline wait can appear inside the GPU-clock interval;
these spans do not measure GPU utilization or the sum of active pass durations.
With this variable unset the non-Tracy device feature request remains unchanged.

Validation entrypoints on `petari_gx_shader_tests`:

- No arguments: existing specialised shader regression suite.
- `--test-uber-coverage`: admission/rejection boundaries.
- `--validate-uber-wgsl`: headless Dawn Null-backend validation of all five legal
  output variants, without opening a game window or compiling Metal pipelines.
- `--dump-uber <file>`: generic WGSL for inspection.

Runtime image, cold-stall and GPU-cost results belong in the final spike report;
passing source validation alone does not establish rendering equivalence.

# 5.4.3 priority 2: pipeline caches survive compatible builds

Design approved by PORT/head; source implementation and server test handoff.
The initial implementation was source-only on the M1; requested follow-up checks
are recorded below.
Base: main `6702189f879a72f47df5bdf7ffce2f6b320a9aeb`.
Branch: `port/543-cache-compatibility`.

## Existing behavior and the two caches

`pipeline_cache_identity.py` currently hashes essentially all native/gx code,
both app/GX CMake files, presentation CMake and the latest seed pack. The result
names BOTH SQLite caches and stamps bundled seeds. Consequently a test-only
CMake edit, tape build wiring or unrelated diagnostic discards every warm path.
There is no compatibility reason for those inputs to own a user cache.

The caches contain different things:

- **Recipe DB**: Aurora GX/Clear serialized configs and first-use hints. It is
  not a persisted `wgpu::RenderPipeline`. Current code recreates descriptors and
  shaders from these configs under the current executable.
- **Dawn DB**: opaque compiler/translation artifacts, requested using Dawn's
  content-derived binary key. Current SQLite code retains only XXH128(key), not
  the exact key, and has no independent payload digest. Its raw load path trusts
  the stored logical size without checking equality to the blob byte count.

Simply changing the filename is insufficient. The new implementation validates
both entry kinds and the containing compatibility metadata before use.

## Compatibility inputs and their versions

Use a checked-in versioned compatibility manifest, generated header plus an
inspectable JSON receipt. No whole-source-tree, app commit, CMake contents,
shader scheduling, diagnostics, seed-pack digest or checkout path enters it.

| Input | Binding / invalidation rule |
|---|---|
| Storage protocol | `petari-pipeline-cache-v2`; explicit versioned SQL schemas and entry envelopes. Missing/duplicate/unknown metadata is a miss. |
| Generator contract | Explicit generator ABI version for specialized/Generic/Fast code generation and uniform interpretation; changing that contract changes compatibility. |
| Actual shader/WGSL content | Retain and compare the **entire Dawn request key**, whose pinned implementation includes the shader module content hash and its compilation request. Template/codegen edits that change emitted WGSL change that entry's key automatically, even without an ABI bump. A changed C++ file that emits identical WGSL need not discard that artifact. No hand-maintained list of unrelated source files is hashed. |
| Recipe encoding | Family, GX/Clear config versions, byte-order/representation, size and complete field-layout signature. A checked schema declaration plus native sizeof/offsetof/enum-value assertions prevents an incompatible C++ layout from silently retaining its old version. |
| Descriptor / vertex-layout ABI | Explicit descriptor, binding/uniform and vertex-layout schema versions. Actual requested layout/compile options remain in the current WGPU descriptor and Dawn's key. Incompatible interpretation changes must bump the schema; a mismatched row is never admitted. |
| Backend | Actual configured Aurora backend ABI/pinned revision, Dawn commit/package identity and effective backend/build API features; generated from the effective configured dependencies, not a hard-coded stale ref. Package identity is verified against the configured artifact. |
| GPU / driver | Selected backend/adapter vendor/device/architecture/model, adapter-provided driver description and relevant features; on Metal include macOS kernel/OS build because Apple updates the driver with the OS. Missing/ambiguous identity disables persistent GPU-blob reuse instead of using `Unknown`. No CPU count, scheduling policy, bundle build or display refresh rate. |

Two hashes distinguish portable recipe compatibility from device artifacts:

```
recipe_abi = SHA256(canonical storage + generator + config/descriptor/layout ABI)
backend_abi = SHA256(effective Aurora/Dawn backend identity)
device_id = SHA256(canonical adapter + driver/OS + backend identity)
artifact namespace = SHA256(recipe_abi || backend_abi || device_id)
artifact lookup = SHA256(full Dawn key), followed by full-key byte equality
```

The runtime initializes the device context after selecting the adapter and before
installing/using the device's Dawn callbacks. No filename is finalized with an
uninitialized GPU identity. This is initialization work, not per-draw hashing.
Changing generator/descriptor ABI deliberately misses. Changing shader text
misses the affected Dawn entry through its real content key. A test/source/CMake
or seed-pack change alone preserves namespace AND existing entries.

### Evidence for Dawn's content key (actual pinned version)

The configured package is Dawn `1155e0ed531126f33a1279afa029349651ca1c93`.
At that exact revision, Device.cpp builds its device key from adapter info,
enabled features, toggles/cache descriptor and Dawn version. Metal's
MslCompilationRequest includes the shader module hash, entry point, compiler
options, bindings, limits and vertex-pulling configuration. BlobCache.cpp uses
the caller's binary key and treats absence/failed validation as a miss. Dawn's
own optional blob-hash validation is supplementary; our SQLite load remains
strict independently.

Primary source files and SHA256 receipts were retained read-only at
`m1g/solb/cache-compat-543/design/dawn-source/PROVENANCE.json`:
[Device.cpp](https://github.com/encounter/dawn/blob/1155e0ed531126f33a1279afa029349651ca1c93/src/dawn/native/Device.cpp),
[ShaderModuleMTL.mm](https://github.com/encounter/dawn/blob/1155e0ed531126f33a1279afa029349651ca1c93/src/dawn/native/metal/ShaderModuleMTL.mm),
[BlobCache.cpp](https://github.com/encounter/dawn/blob/1155e0ed531126f33a1279afa029349651ca1c93/src/dawn/native/BlobCache.cpp).

## Load/store validation and fail-closed behavior

**Recipe rows**: validate SQLite types, supported family/schema, exact blob
length, internal version, stored config hash and SHA256 over immutable
compile-relevant fields/bytes before copying into a typed config. Recompute the
config hash instead of trusting the index. Apply existing semantic bounds plus
required enum/count/bool checks before shader generation. First-use rank is
separately bounded ordering metadata, never a compatibility identifier. Reject a
bad row and compile the real requested config; never publish an invalid pipeline
as ready. Use the same admission helper for user DB, seeds and stage-manifest
loaders so no side door accepts unchecked bytes.

**Dawn rows**: persist full binary key, compatibility/context, payload size,
encoding and SHA256 of the original value. Validate exact key bytes, envelope,
SQLite types, size caps, raw blob length or bounded decompression length, and
SHA256 **before either the size probe or copy returns a hit**. Start with a
64 MiB value / 1 MiB key bound; oversize is simply uncached, never truncated.
Invalid flags, short/overlong data, decompression errors, swapped keys/contexts,
digest mismatch or SQLite errors return zero (cache miss), without passing bad
bytes to Dawn. Validate both calls of Dawn's size-then-data API. Protect them
with the existing cache mutex; no game-thread/interrupt lock is acquired.

Writes are transactional and complete their envelope/digest before publishing.
Corruption cannot be made valid by adding the current identity to an old file.
An invalid new-format DB is not overwritten as though empty: preserve/quarantine
it under the selected cache lock or disable that disk cache for the process,
then compile normally. Old caches are never removed by migration.

One bounded startup report records recipe/backend/device identities, accepted /
rejected / migrated rows and reasons; end-of-run counters expose artifact hits,
misses and rejected corrupt entries. Build SHA remains separate provenance.

## Files, migration and seed-pack behavior

Suggested new names are `pipeline_cache.v2.<recipe_abi>.db` and
`dawn_cache.v2.<artifact_namespace>.db`. Existing exclusive machine-cache lock,
private PETARI_CACHE_DIR override and per-save fallback remain.

On the first new-format launch only, discover legacy build-key files **inside
the already selected cache root**. Open them read-only, validate their own
identity/path pairing and SQLite integrity, and import only individually valid,
current-schema recipe rows (stored config hash recomputed). Unknown/corrupt or
incompatible rows are skipped. Import in a transaction and seal a migration
receipt; do not relabel, truncate, rename or delete the original files. Missing
checksums on old recipe rows do not certify old compiler artifacts: the imported
recipes are regenerated under today's generator.

**Do not adopt legacy Dawn blobs.** Their format lacks exact original key bytes,
new context provenance and our value checksum, so compatibility/integrity cannot
be proven after the fact. Leave them intact and repopulate the new artifact DB.
This is an explicitly reported, one-time cold transition, not a claim that the
old build keys were compatible. Future compatible builds reuse the new cache.

Bundled v14 pack bytes and .pfd fingerprints need not change: build-local copies
receive the new schema/compatibility and row-integrity metadata without changing
the config/rank rows bound by .pfd. A seed is a recipe/prewarm source, not a
compiled-cache identity. Import the initial seed only when the durable recipe
cache is empty; later missing/new stage hints may add recipes but never evict or
invalidate compatible learned/artifact entries. Stage ordering metadata may
still be consulted; changing the pack alone cannot turn a warm compiler cache
cold. Test a warmed private cache without the seed pack too.

All tests and experiments use fresh/private roots under port53/m1g or temporary
dirs, explicit PETARI_CACHE_DIR and an isolated test HOME if exercising path
selection. They never enumerate or open the real user's
`~/Library/Caches/AstralItalian*`. Production migration is tested against
synthetic/retained **private** legacy fixtures, never the user's DBs.

## Tests and acceptance

1. Build A warms a private fixture; build B changes only an unrelated test CMake,
   smoke/tape or scheduler/diagnostic source. Namespace stays equal; real stored
   recipe and Dawn callback entries HIT, verified by byte/counter receipts.
2. Deliberately incompatible descriptor/vertex-layout/config/generator ABI must
   MISS; changed actual WGSL/request key must MISS; GPU, driver, backend/Dawn or
   enabled compile-option changes must MISS. Old unchanged entry stays intact.
3. Wrong/absent/duplicate identity, wrong type/version/length, modified config,
   bad enums/counts, full-key mismatch/collision fixture, swapped artifact,
   size-probe disagreement, every payload corruption/compression case, failed
   transaction and truncated SQLite fixtures must MISS without publishing data.
4. Migration preserves legacy file hashes, imports only valid recipes, rejects
   old opaque blobs and is idempotent. New-format foreign files cannot be
   relabelled. Seed-pack revision/content alone preserves existing cache hits.
5. Native tests exercise the patched production SQLite load/store callbacks,
   not a second toy cache; Python tests cover deterministic canonical identity
   and CMake dependency isolation. Every test root is asserted private.
6. After source/unit review, two exact app builds differing in an irrelevant
   change reuse the SAME private cache, GPU and bundle namespace. Retain hit /
   reject counters, source/key/provenance and startup/prep/compile numbers. Cold
   sanity plus warm replay/image checks must remain correct; existing audio and
   frame thresholds stay unchanged. Coordinate any M1 work with SOL A; no native
   compile/run is currently queued by B.

Implementation remains native-only plus tests/docs and Aurora patch scripts;
no root src/include/libs edits. It is a cache-format/validation change, not a
pipeline-worker scheduling or shader visual change.

## Implementation and the correctness boundary

The implementation uses `pipeline_cache_compatibility.json` as the explicit ABI
ledger; `pipeline_cache_identity.py` also emits the effective backend receipt.
CMake hashes the actual configured Dawn library, not its CMake file or path.
The historical `.key`/`PETARI_PIPELINE_BUILD_KEY` spelling is retained for build
receipt tools, but now holds `recipe_abi`, not a build digest. The adjacent JSON
records both recipe and backend identities. Adapter selection initializes the
runtime artifact identity before installing any Dawn callback.

`pipeline_recipe_validation.hpp` asserts the real GX13/Clear5 sizes and field
offsets against that ledger. Each recipe load checks the SQL envelope, raw bool /
reserved bytes, version and bounds, SHA256, recomputed config hash and existing
shader semantic validation before typed use. The same adapter is used by initial
seeding, learned recipes, stage manifests and the frontend preparation source.
Only a successful upsert transaction publishes its matching integrity row.

`pipeline_artifact_cache.hpp` implements the production callback storage. Its
entry checksum binds version, namespace, full binary key, logical length and
original decoded value. Its SHA256 key index is only an accelerator: a hit also
requires full-key equality. Both size probe and copy validate raw byte length or
bounded zstd decoding, then the complete checksum. The raw callback size/length
bug cannot survive the size probe. Unknown schemas/encodings, foreign metadata,
invalid/corrupt/truncated data and context changes miss without exposing bytes.
Old Dawn files are not opened for migration, renamed, stamped or deleted.
Session-based pruning is removed: a key unused during one session remains useful
for a later stage and is not evidence of incompatibility.

A stale recipe is **not** a persisted shader/pipeline. Today's generator runs on
it again. If a generator/template edit changes the emitted WGSL or compilation
options without an ABI bump, pinned Dawn derives a different shader-module /
compilation-request key. That request cannot match the previous full key. The
old recipe can waste a prewarm; it cannot make the callback substitute an old
shader for the current output. ABI versions still invalidate incompatible recipe
interpretation, binding and vertex-layout contracts, and static assertions catch
layout drift. `native_pipeline_cache_dawn_keys` exercises this boundary with the
real pinned Metal backend, two identical generated outputs and a changed output,
while keeping the recipe ABI unchanged; it requires actual disk callbacks, warm
hits, and misses for the newly generated full keys.

`native_pipeline_cache_compat` tests the real typed decoder, SQLite storage and
read-only legacy migration. `native_pipeline_artifact_raw` separately exercises
the production raw callback implementation without zstd. The private seed-stamp
test sends the v14 pack through Python stamping and the native typed/checksum
loader, leaving the original pack unchanged. The scheduling-only miniature GX
fixtures have explicit admission adapters; they are not used as decoder proof.

Server runner after configuring the GX test build:

```
bash tools/port/test_pipeline_cache543.sh /absolute/private/GX_TEST_BUILD
```

This builds the EXCLUDE_FROM_ALL targets explicitly, then runs the cache tests
and existing shader/storage checks. Each native cache test creates a fresh
private root and isolated HOME; the Dawn test selects Metal without launching a
normal user app. Source-only tests cover canonical identity, field rejection,
backend changes, unrelated source/CMake/seed edits, source-pack preservation and
the pinned patch chain. The source-only tests do not certify a native test or
app build; retain those server results separately. After unit admission, two
exact app builds with one irrelevant source/CMake edit must reuse the same
private cache in the runtime warm proof, with the compatibility/artifact log
lines bound to both binaries. Do not use normal user cache directories for that
proof.

Release note for 5.4.3: **The first launch after updating prepares graphics once
more; later compatible updates keep them.** The unchanged Preparing graphics
path covers this one-time legacy-blob transition. Seeds and `.pfd` fingerprints
remain byte-identical; their digest is no longer a cache-compatibility input.

### Source validation before the server build

Source-only checks passed: identity 6/6, pinned cache-patch chain 4/4, and PFD /
seed-pack tests 10/10; shell syntax and `git diff --check` passed. Inspection of
all 2,525 unique v14 recipes found no noncanonical bool/padding bytes or bounds
violations in the newly decoded fields. It also confirmed `dstAlpha=UINT32_MAX`
is the legitimate disabled sentinel, which the decoder retains. This inspection
is supplementary; native typed admission of the Python-stamped pack is a
separate required server test. No native compilation or execution was performed
on the M1. Native build/test and the two-build private-cache warm proof remain
pending until PORT runs and binds them to the pushed commit.


### C++17 follow-up after the first server build

The first server app build of `9ffc8e7c` failed because the shared SHA256 header
used C++20 `std::rotr` from C++17 app translation units. The follow-up uses a
local constexpr 32-bit rotate with the SHA algorithm's fixed counts in 1..31;
it removes unused `<bit>` / `<span>` includes and leaves app language modes and
compatibility identities unchanged. A dedicated `native_cache_sha256` target
is explicitly C++17 and checks standard vectors plus streaming.

PORT-requested local checks under heavy -> native passed: the actual
`pipeline_cache_dir.cpp` and `smoke_storage.cpp` at gnu++17, four cache-test TUs,
and the compat / Dawn-key tests with zstd enabled (8 syntax checks total). The
C++17 SHA and raw-cache CPU executables also passed (2 runs). The checks used
private fixture identity headers / isolated HOME, never a user cache or app.
The first local check additionally caught repeatable Dawn callbacks using
capturing lambdas in the integration test; these now use noncapturing callbacks
with explicit userdata, as required by the actual pinned Dawn API. This is
test-only; production callbacks already used noncapturing lambdas.

Receipts: `m1g/solb/cache-compat-543/rotate-check-r3/RESULT.json` and per-case
logs; earlier r1 failures remain preserved. These checks do not replace the
server candidate app build, configured native suite or real Dawn runtime test.
No app/GX build or gameplay measurement was performed locally.

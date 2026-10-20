# Native renderer cache admission

Pipeline and Dawn SQLite files use `pipeline_cache.<key>.db` and
`dawn_cache.<key>.db`. The compiled key is a SHA256 of the renderer schema,
Aurora 08122911, native GX source/patch chain, and renderer packaging inputs.
It excludes checkout paths, timestamps and Git state; identical renderer inputs
produce the same key. Unrelated game-source changes may reuse this renderer key.

Every database must contain exactly one matching `petari_cache_build` identity.
Only an empty newly created database can acquire that identity at runtime.
Legacy untagged or foreign databases are ignored, even if renamed. They are
neither repaired nor deleted. This intentionally starts a cold namespace after
a renderer change without discarding compatible rows from the same renderer.
Dawn uses the same namespace boundary because its compiled pipelines also depend
on the generated shaders.

The normal machine-cache root, `PETARI_CACHE_DIR` override, user-directory
fallback, and background-smoke cache roots retain their existing selection and
locking rules. Adoption copies only matching keyed databases with matching
metadata. A failed machine-cache creation/lock therefore cannot bypass identity
checks through the user fallback. Background adoption applies the same rule.

Bundling stamps a build-local copy of each initial/stage seed with the compiled
identity. The input seed remains untouched. A pre-stamped foreign seed is an
error, not an opportunity to relabel data. Release packaging must validate the
seed against the target renderer before bundling.

GX rows pass structural and sampled-texgen validation before cache precompilation,
seed import, or stage-manifest resolution. Invalid rows are skipped and their
hash/reason logged; their bytes are never normalized or repaired. Valid rows are
reused. Sampled texgen source/type admission is shared with the live shader path,
whose fatal check remains for invalid live configurations. The database format
versions remain unchanged.

Build 4 (`a3ff7df1`, Aurora 08122911) has the old unkeyed shared-cache preload path
and is exposed to poisoned GX rows. Moving that cache aside avoids the known
trigger; it does not change Build 4's loader. Older unused rows may still cost
precompilation time in older builds. The new renderer does not load their legacy
namespace.

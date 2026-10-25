# Destructor census

```sh
python3 tools/team/dtorscan.py --out /absolute/empty/output
PYTHONPATH=tools/team python3 -m unittest tools.team.test_dtorscan
```

Read-only; no compiles, build configuration or input edits. `--root` defaults to
the current configured checkout; `--base-root` defaults to `decomp3-land`.
Every configured target object is examined, including units lacking source
objects. All opaque `fn_` functions are compared to relocation-masked bodies of
existing named template destructors in available source objects. Exact relocation
identities are recorded separately from instruction-shape matches. A generic
null-receiver/deleting-epilogue scaffold provides a second candidate inventory.

Independently, all available source destructors that bind the canonical
LiveActorGroup destructor are checked against the original DOL branch destination.
All other named base-destructor calls are checked as well in `base-crossed.json`.
Masked-body equality distinguishes trustworthy aligned sites from bodies that
need manual inspection. Unanchored weak source copies are recorded separately.
Identical empty destructor bodies cannot establish their
template argument or class identity: use original caller/vtable bindings and a
private whole-unit source experiment before proposing changes.

Outputs: `candidates.tsv`, full `opaque.json`, `crossed.json`, `scaffolds.json`,
and `meta.json` with all object hashes, input hashes, coverage/skips/errors.
Unavailable source objects limit the template witness inventory and crossed-call
scan. Unsupported relocation kinds fail closed for body matching. Generic
scaffolds are heuristic and may omit unusual destructors. No automatic source or
symbol rename is emitted.

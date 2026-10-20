# Native raw-offset audit

`tools/team/offsetaudit.py` inventories source expressions before deciding whether
any access uses a Wii byte offset against a native host object. Default scanning
is source-only: it never runs Clang, MWCC, Ninja, CMake, or any subprocess.

```
python3 tools/team/offsetaudit.py --root . \
  --scope src/Game --scope include/Game --scope libs \
  --out /tmp/game-offsets.json --markdown /tmp/game-offsets.md
```

Repeat `--scope` for files or directories. Defaults are `src/Game`, `include`,
and `libs`; library source directories must be supplied explicitly by their owner.
The complete list of scanned files and SHA256 values is part of every result.
Only C/C++ source/header/include suffixes are included. Symlinks outside the root
are excluded; missing scopes fail rather than silently producing an empty audit.
Strings (including raw strings) and comments do not produce candidates. Source
positions are byte offsets, including in Shift-JIS files.

The inventory includes C pointer/integer-address casts, named pointer casts,
`this + offset`, constant/sizeof pointer-arithmetic candidates, `offsetof`, and every `_` + hexadecimal identifier (including suffixed fields such as `_2C_new`, both member
uses and declarations/unqualified occurrences). Ambiguous expressions are retained.
Each cast occurrence is independent: nested casts can describe the same actual
access. `arithmetic_in_statement` and nearby constants are discovery hints, never
an inferred base address or field identity. Macro expansion, implicit casts and
C++ type resolution are not performed by the lexical pass. Delimiter/preprocessor
gaps and unmatched imported binding IDs are reported. **This inventory is not a
complete semantic list of misreads. UNKNOWN rows and parsing gaps prevent an
all-clear conclusion.** The tool does not certify bounds, lifetime or reachability.

## Schema 1

All inputs/outputs use `schema: 1`. Each row has a stable ID derived from relative
path, byte offset and kind; line, expression, context, source SHA, expression SHA,
native guard state, domain, classification and explicit gaps are retained.
Bindings use exact source/expression hashes, so editing the source invalidates
old exemptions even when a row ID remains the same.

```
{
  "schema": 1,
  "accesses": {
    "ROW_ID_FROM_SCAN": {
      "source_sha256": "...", "expression_sha256": "...",
      "record": "StageSwitchCtrl", "field": "mUsesAutoDeadSwitch",
      "domain": "host_object", "raw_offset_bytes": 16,
      "evidence": ["exact base/type/field binding and DOL/MWCC probe paths + hashes"]
    }
  }
}
```

Bindings are reviewed type/provenance evidence, not a field-name guess from a
comment or an adjacent literal. `raw_offset_bytes` must be the offset from the
bound record itself, accounting for intermediate/base/member pointers. Use
`domain: serialized`, `wire`, or `hardware` only with evidence. In particular,
`JUTDataBlockHeader::this + mSize` traverses serialized blocks and cannot be called
a host-layout bug merely because a header contains pointers elsewhere.

Layout artifacts have this form (sizes/alignments bytes; field offsets bits):

```
{
  "schema": 1, "abi": "wii",
  "producer": {
    "kind": "clang_record_layout_dump",
    "command": ["clang++", "-target", "powerpc-unknown-eabi", "..."],
    "accuracy": "approximation", "raw_sha256": "..."
  },
  "files": {"include/Game/Map/StageSwitch.hpp": "...", "probe.cpp": "..."},
  "records": {
    "StageSwitchCtrl": {
      "size": 24, "align": 4,
      "fields": {"mUsesAutoDeadSwitch": {"offset_bits": 128, "type": "_Bool"}}
    }
  }
}
```

Use `--wii-layouts wii.json --native-layouts native.json --bindings bindings.json`.
All layout input-file pins are checked against the checkout. A missing/stale pin,
wrong ABI, unresolved field, absent producer or unreviewed domain stays UNKNOWN.
PPC Clang is an approximation of MWCC's ABI, especially for inheritance,
bitfields, packing and enums. Keep `accuracy: approximation` until actual owning
MWCC layout evidence supports the relevant record. Native Clang is measured for
the exact compiler/target/options/header state supplied, not every host ABI.
Source estimates or field comments are not accepted as measured layouts.

Classifications:

- `BOUND_LAYOUT_MISMATCH`: reviewed host-object raw offset matches the Wii field,
  differs from the native field, with both artifacts marked measured.
- `LAYOUT_MISMATCH_CANDIDATE`: same comparison, with at least one approximate ABI.
- `LAYOUT_SAME_OFFSET_CANDIDATE`: equal field offsets with an approximate ABI.
- `BOUND_SAME_FIELD_OFFSET`: measured bound field offset agrees; this proves no other safety
  property and does not certify an access width or value representation.
- `TYPED_MEMBER_ADJUSTS`: `_XX` is a typed member. The compiler moves it naturally;
  a changed offset alone is **not** a native raw-access bug.
- `NON_HOST_DOMAIN`: pinned reviewed serialized/wire/hardware access.
- `NATIVE_EXCLUDED`: lexically excluded by a known GALAXY_NATIVE guard.
- `LAYOUT_AWARE_EXPRESSION`: `offsetof` present as its own inventory item; the
  containing expression still needs inspection. This is not a whole-access waiver.
- `UNKNOWN`: the required binding, domain, guard or layout remains unresolved.

## Import or explicitly collect layouts

Import an existing Clang record dump without running any compiler:

```
python3 tools/team/offsetaudit.py --root . \
  --import-layout-dump /tmp/ppc-layout.txt --abi wii \
  --pin include/Game/Map/StageSwitch.hpp --pin /absolute/probe.cpp \
  --accuracy approximation --out /tmp/wii.json \
  --producer-command clang++ -target powerpc-unknown-eabi
```

Because `--producer-command` is an argparse argument list, pass a single quoted
command description if its flags would otherwise be interpreted as tool flags.
The complete original command should be retained with the producer packet.
Only direct named record fields are imported. Nested/base entries must be bound
to their own record, not mistaken for top-level fields.

For an explicitly authorized probe run, use `--collect-plan plan.json --out DIR`.
This is the **only** mode which starts a compiler. A plan supplies exact argv,
source pins and accuracy separately for the two ABIs:

```
{
  "schema": 1,
  "files": {"probe.cpp": "SHA", "include/Game/Map/StageSwitch.hpp": "SHA"},
  "probes": {
    "wii": {"accuracy": "approximation", "command": ["clang++", "-target",
      "powerpc-unknown-eabi", "-Xclang", "-fdump-record-layouts", "-fsyntax-only", "probe.cpp"]},
    "native": {"accuracy": "measured", "command": ["clang++", "-target",
      "arm64-apple-macos11", "-Xclang", "-fdump-record-layouts", "-fsyntax-only", "probe.cpp"]}
  }
}
```

Inputs are checked before/after collection. Nonzero compile status, incomplete
record dumps and changed pins fail; raw diagnostic dumps are retained. This is
syntax-only collection, not a game build. Projected probes must pin the original
owning declarations and explicitly retain the projection in evidence. A projected
class is not automatically equivalent to its full original header under every ABI.

The StageSwitchCtrl regression has real evidence: canonical MWCC offsetof emits
`li r3,16`, PPC Clang reports16, native arm64 reports32, and a native canonical
header static_assert verifies32. The owner fix uses the actual named field under
GALAXY_NATIVE, leaving the Wii statement verbatim. JAS MemoryChunk's +12/+16 case
belongs to the library owner. JKRDvdFile's native offsetof correction and typed
`_XX` accesses are separate nonbug regression fixtures.

## Tests and delivery

```
python3 -m unittest tools/team/test_offsetaudit.py
```

Tests include no-subprocess default scanning, stale pin rejection, raw-layout
mismatch, approximate ABI disclosure, typed-member deltas, serialized domain,
known/unknown guards and offset-aware native corrections. Every actual fix still
needs current `objsame.py`, native tests and its subsystem owner's branch/gate.
No source-only inventory grants permission to rewrite another owner's code.

## Mechanical type binding from an existing Clang AST

The scanner can consume pinned `-Xclang -ast-dump=json -fsyntax-only` output with
repeatable `--ast MANIFEST.json`; it does not invoke Clang to create that AST.
A manifest pins one primary source, the JSON payload, all owning declarations
and the command which produced it:

```
{
  "schema": 1, "primary_source": "src/Game/Example.cpp",
  "ast": "example-ast.json", "ast_sha256": "...",
  "files": {"src/Game/Example.cpp": "...", "include/Game/Example.hpp": "..."},
  "producer": {"kind": "clang_ast_json", "command": ["clang++", "..."]}
}
```

An optional `--record-domains domains.json` supplies owner-reviewed domains:

```
{
  "schema": 1, "files": {"include/Game/Example.hpp": "SHA"},
  "records": {
    "Example": {"domain": "host_object", "scope": "all_instances",
      "evidence": ["proof that all instances covered here are host objects, path + hash"]}
  }
}
```

The importer desugars concrete pointer types, traces actual byte-pointer
addition/subtraction with an integer constant expression, and looks up the field
at that exact Wii offset. For unnamed MemberExpr nodes it resolves the record
and named member. No field is guessed from its spelling, no scalar pointer stride
is assumed one byte, and overlapping/ambiguous fields stay unresolved. Macro
expansion locations, base-class displacement, pointer-to-pointer provenance and
unresolved record types remain UNKNOWN. Explicit per-access bindings take
precedence over AST-generated candidates.

A record domain is only a hint unless its evidence explicitly covers
`scope: all_instances`. A class can represent serialized bytes in one access and
a host allocation in another. Without that all-instance proof the generated
binding remains domain UNKNOWN; use a reviewed per-access binding instead.
Never apply an all-instance domain to a mixed-domain record. Merely
having a record type does not establish host-object allocation or ownership.
Generated bindings and AST producer information remain visible on each row.
The first actual probe mechanically recovers StageSwitchCtrl's +16 access and
native+32 field. It stays an approximate-layout candidate because PPC Clang is
not relabelled as MWCC evidence. A separate actual MWCC `offsetof` probe provides
the independent sample check.

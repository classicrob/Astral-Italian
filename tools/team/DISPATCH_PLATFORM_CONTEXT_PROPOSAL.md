# Exact platform-context proposal — DISABLED, source models only

Observed key: `__CF_USER_TEXT_ENCODING`. Saved `8bd08` establishes its presence
only. Its bytes, stable value, augmentation mechanism, and behavior after target
exec are UNKNOWN. Nothing in this proposal admits a ninth key or executes code.
Current active gate remains `6700`/d696 and rejects the extra entry. Prior cases
and disabled full827 are immutable. No default, UID-derived or locale-derived
value is proposed.

## Current binding chain (a4dc source; L importer snapshot 3bacfa28)

1. `dispatch_full_budget.launch_context(root)` builds exactly eight env entries
   and pins Python/gate/target/shell/runtime bytes. It does not declare platform
   extras. FullBudgetJob.run requires exact parent mapping equality and rederives
   the same launch context, then publishes the SHA-bound request containing env.
   Popen currently omits env and inherits the native environment; the child
   interpreter/import startup precedes its snapshot. This identifies a boundary,
   not the cause of the observed additional entry.
2. `dispatch_exec_gate.check_request` validates request/gate/Python pins and
   compares complete env and cwd snapshots before identity; after parent ack it
   rechecks the pinned request and complete env again before original exec.
   The diagnostic allowlist governs disclosure only, NEVER admission. Merely
   adding a key to CONTEXT_KEYS would not fix or authorize launch mapping.
3. Chosen-plan context/source hashes bind the whole proposed mapping. Fullproof
   contracts canonical-hash launch_context plus every helper/source/tool pin;
   result, monitors, package recipe and detached terminal publication join that
   exact full contract SHA. Packaging reinstalls the contract env under its
   shared native lock, even though no child is normally needed in that stage.
4. L dispatch_import.verify checks the externally operator-pinned contract SHA
   and independent compile command/phase-root expectations. Current
   operator_context_sha256 includes command_tokens/phase_roots/terminal pin, not
   an independently typed platform mapping field. The existing contract hash
   transitively binds env, but future admission requires separate explicit
   operator agreement on the value, not a producer-selected context adoption.

## Smallest future implementation changes — NOT WIRED

- Operator provides a private immutable record: exactly schema=1, full nine-key
  env mapping, SHA256(canonical full mapping), and exact ninth bytes/length/hash.
  Operator separately supplies SHA256 of raw record bytes. Existing eight values
  must equal the exact separately pinned base; extra/missing/tenth/case-changed
  keys and different ninth value reject. Value <=512 strict UTF-8 bytes, no NUL;
  if the observed value is unrepresentable, STOP for a new reviewed format,
  never normalize/substitute it. Empty is distinct from absent and can only be
  admitted if explicitly observed and pinned. No ambient getenv fallback.
- Pure proposal `dispatch_platform_context_proposal.py` models that contract
  with SYNTHETIC values only. No active caller imports it. Record has no actual
  value or operator digest yet; disabled JSON explicitly retains nulls.
- Future launch_context(root, operator_record, operator_raw_sha) validates the
  record and returns exact nine-key env plus record/full-env pins. Re-derivation
  before each launch must consume the SAME immutable bytes, never recollect a
  new environment value. Parent full mapping equality remains strict.
- Future Popen should receive env=the exact pinned mapping explicitly, subject
  to independent source/context review. This narrows arbitrary native inheritance
  but does NOT prove interpreter startup preserves the mapping. No current
  Popen semantics are changed by this packet.
- Future request schema adds platform-record/full-env pins. Gate validates those
  pins and the full requested mapping against the independently reviewed mapping,
  compares ALL nine actual keys exactly before identity AND after ack. Parent
  verifies record/request joins before ack; no extras may be ignored, deleted or
  normalized. Any missing/value-drift/extra key blocks ack/exec as before.
- First future payload qualification must record and compare its ACTUAL full
  post-exec mapping against the SAME operator record; checking only PID/PGID/SID/
  UID cannot establish environment preservation. Native targets that cannot
  expose that observation remain explicitly unqualified; do not claim universal
  post-exec stability from one Python check or a pre-exec gate.
- All plans/source/request-schema/launch-context/fullcontract/monitor joins and
  packaging context must be newly repinned in a fresh root. Canonical Wii spec,
  compile flags and six original provenance keys remain unchanged. Full827 stays
  disabled and is NOT edited. L must agree the new independent platform-record
  pin plus expected full mapping in operator trust, verify equality against
  recipe/launch evidence, and include platform pin in operator_context_sha256.
  This is an L-owned importer change, not supplied production authority here.

## Smallest separately bounded metadata observation — PROPOSAL ONLY

Observe ONLY this exact named key in (a) launcher entry mapping before env setup,
(b) parent mapping immediately before spawn after existing eight-key setup, and
(c) child mapping at observation entry. Presence plus <=512-byte strict UTF-8 hex,
byte length and SHA256; absent stays absent; no other keys/values/hashes exposed.
Private EXCL records in a new mode700 dir, mode600 files, <=16KiB per record,
three records maximum; no payload, tool exec, ack, full proof or admission path.
Unrepresentable/oversize/I/O failure records FAIL without a partial value dump.
Record source/Python pins and actual observation PID/PGID/SID/UID for each stage
with the approved parent/child join, without guessing native environment state.
No direct native environment enumeration or platform-library call is proposed.

One future review/allocation could bound this to <=30s total, <=10s work +4s
cleanup, <=64MiB/floor50/nice19/nonblocking native.lock, one child, no retry.
The child self-expires <=10s and exports the actual identity for the existing
accepted cleanup contract; if ownership is not verified, no signals and no
absence PASS. No current child observer/launcher is implemented by this design.
Exact spawn/helper/record timing, argv and allocation remain pending separate
source review; this plan is NOT executable and cannot activate a hidden retry.

A single named-value observation would disclose those selected-stage bytes but
would NOT establish stability, mechanism, effect on all tools, or allowlist
admission. Operator review must decide next steps from the retained evidence.

## Proposed bounded controls before any future admission

Missing operator record/pin, wrong raw/full-env/value pin, missing ninth/tenth or
case-changed key, altered base eight, different child/postack/postexec ninth,
absent vs empty, >512 bytes, invalid encoding/NUL, stale record, unknown extra,
observation I/O failure. All must reject independently; no record repair.
Current pure models cover schema/hash/key/value-boundary/exact-check cases only.
No process, native environment, signal, runtime lifecycle or stability test ran.

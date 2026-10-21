# FIFO diagnostic prototype: synchronization preflight

Status: **BLOCKED_SYNCHRONIZATION_REVIEW_REQUIRED**. This prototype intentionally
emits no C++ hook/patch and is not a usable capture. No source, CMake, parser,
publication, cursor, state, array mapping or fatal operation is modified. No
compiler/app/runtime or M1 heavy lock was used. The earlier first-fault recipe
remains the target; default OFF cannot be mistaken for implemented coverage.

## Delivered implementation

`fifo_firstfault_preflight.py` validates a source-pinned four-role effective profile,
unique bounded anchors, fast/slow/recording copy sites, worker mirror consumption
and buffer-rebase witnesses. It writes only a fresh private metadata report under
`~/MacioGalaxy2-team/scratch-port/solP/fifo-firstfault-prototype-output/`.
It rejects other roots, source-containing outputs, symlinks, existing outputs,
role aliases, changed hashes/profile, missing/duplicate/out-of-order anchors.
The effective-source profile has a compiled-in SHA; CLI cannot override its hashes.
No source is rewritten. The blocked report returns2; invalid inputs return1.
There is no success/capture-ready/production acceptance path.

The profile names actual T2 worldmap51 effective files (hashes only in git): FIFO
sync source, pipeline-owner parser, pipeline-owner GXDispList and original FIFO
inline header. Exact actual compiled build.ninja edge and generated-header inclusion
still require independent binding before any future generated patch is usable.
Matching a public raw source hash is insufficient. This profile is not a universal
Aurora patch contract: other generated paths/hashes fail closed and need review.

## Concrete blocking interleaving

The effective `write_data_grow` wraps the copy in a closure. Its lock helper may
call `petari_gx_sync_begin_host_wait`, release the emulated OS CPU and wait. The
closure computes `needed` from the THEN-current buffer size and performs memcpy
under the buffer lock after the wait. Another permitted game writer can run while
the original thread is waiting. Therefore:

1. A diagnostic wrapper reads write_position at A before write_data.
2. Growth waits and yields the emulated CPU; another writer can add unrelated bytes.
3. The original copy later appends at B, not necessarily A.
4. A wrapper end-position read cannot prove that [A,end) belongs to this DL.

This is a failure of the proposed diagnostic provenance, not a diagnosed bug in
Aurora or a claim that the specific crash took this interleaving. An old/approximate
position getter cannot repair the evidence. Inline fast copy and redirected DL
recording are distinct paths. Replacing the copy or adding forced publication/drain
would change behavior and is forbidden.

A correct future sidecar needs an original caller context plus exact source pointer,
length and actual destination span observed at EACH real copy site. Slow path must
record while its existing buffer lock protects the chosen destination, before the
subsequent publish. Fast path requires an independently reviewed writer-serialization
contract. The recording path belongs to a different offset domain; its later replay
must not be falsely treated as a main FIFO span. No TLS context or sidecar mutex is
added until reentrancy/thread topology is reviewed.

The worker seam is promising: process_to already has absolute `processed`, and the
actual processor reads a worker-owned private mirror. A future per-call diagnostic
context can bind `processed + reader.offset()` without retaining a pointer into the
reallocating live FIFO. But abort/restart resets, mirror extension and compaction
must be generation-bound. This feasibility observation does not implement a ledger.

## Required review checklist before emitting hooks

- [ ] Bind the actual compiled FIFO/parser/DL edges AND effective inline header.
- [ ] Enumerate every FIFO writer and interrupt context; prove single-writer/reentrant
      constraints around fast copy and whether CPU-baton release can nest a DL call.
- [ ] Approve exact fast/slow/recording copy-site hooks preserving original operations.
- [ ] Review producer context lifetime across wait, interruption and nested recording;
      distinguish producer call ID from copied stream interval.
- [ ] Fix bounded sidecar synchronization and lock order. Slow-copy hook is already
      under buffer lock: if it takes sidecar lock, no path may take those locks in
      reverse order. Worker process currently runs outside buffer lock. No disk I/O,
      allocation, symbolization, logging recursion, GX call or unbounded wait under
      diagnostic locks. Review game-CPU ownership while waiting too.
- [ ] Define immutable sidecar publication BEFORE existing FIFO publish can expose
      bytes; reject torn/evicted entries, no plain-data seqlock race.
- [ ] Bind init generation, abort, clear/rebase and recording-domain transitions.
- [ ] Approve worker ledger/primitive/VCD/VAT/array history coverage and bounded
      first-trigger private flush; all are currently explicitly MISSING.
- [ ] Prove defaultsOFF preserves source operations and approve integration ownership.
- [ ] Allocate later build/synthetic/runtime tests separately; none are claimed here.

## Lightweight validation

Run `nice -n 19 python3 -B -m unittest tools/team/fifo_firstfault_tests.py`.
Sixteen unittest cases pass, including seven individual critical-anchor mutations,
profile/source drift, duplicates, ordering, alias/output confinement and unchanged
synthetic source checks. Synthetic source tests patch the validation dependency in
process solely for testing; CLI retains the pinned profile. Tests validate the
preflight rejection behavior, NOT C++ synchronization or rendering behavior.

Actual source preflight r2 returned2 with blocked report and no patch emitted.
Initial r1 rejected an unscoped repeated raw-command anchor (the parser has helper
read sites); the corrected generator uses exact process-to-next-function boundaries
and rejects duplicate anchors INSIDE that scope. A negative verifies helper reads
outside that scope do not alias the hook. Initial test fixtures also exposed macOS
/var temporary-path symlink normalization; fixture paths were resolved, while the
production root continues rejecting symlink ancestry. Failure metadata is retained
with the producer report; no incomplete run is promoted.

Do not add a `--force`, guessed offset, placeholder PASS or behavior fallback to
bypass these blockers. The next deliverable is a reviewed synchronization contract
or narrower precise blocking evidence, not an unqualified patch for T2 to run.

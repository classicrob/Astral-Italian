# Exact writer/copy-site feasibility review

Status: **STOP_WRITER_EXCLUSION_NOT_ESTABLISHED**. This refines, and does not lift,
4e0eeb23's BLOCKED preflight. No hook/patch is emitted. No original synchronization,
publication, cursor, fatal or buffer operation is changed. No compiler/runtime,
shared source edit, integration or lock acquisition occurred.

`fifo_firstfault_writer_anchors_v1.json` pins nine effective source files and eleven
bounded source ranges. This is a source-qualified path review, not proof that the
specific failing run follows the hypothetical interleaving below. Effective sources
are the T2 worldmap51 chain, including the sync mirror and pipeline-owner patches;
raw Aurora081229 alone does not describe their locking. All byte writers in the
reviewed Aurora/native roots converge on fifo.hpp/fifo.cpp or the SDK wrappers.
Caller thread restrictions remain an unproved precondition, not an inferred fact.

## All mutation / publication paths at the FIFO boundary

| Path | Destination / byte operation | Existing exclusion / exposure | Required diagnostic witness |
|---|---|---|---|
| write_data fast | sBufferData+sBufferSize; memcpy; size+=length | No buffer mutex or runtime writer-owner check in inline function | Exact actual copy span, writer identity and overlap validity |
| write_u8 fast | buffer[size++]=value | Same; separate inline path, must not omit it | Exact single-byte span and writer identity |
| write_data_grow | recompute needed/capacity; possible realloc; memcpy at current size; assign needed | Closure runs under sBufferMutex if worker joinable; helper can release CPU before lock, reacquire CPU only AFTER copy/unlock | Record inside closure, not around wrapper; prove exclusion from unlocked fast writes first |
| write_data recording | sDlBuffer+sDlWritePos; memcpy; recording pos+=length | Global recording state, no buffer mutex | Separate recording object/generation/domain; never tag as main FIFO absolute span |
| write_u8 recording | dlBuffer[dlWritePos++]=value | Same; separate path | Same domain and unique write span |
| patch_u32 | overwrites four bytes at buffer-relative offset | Asserts below size and at/above published watermark; no added lock | Patch provenance or worker-final-byte capture; producer-copy hash alone is not final data |
| begin/end display list | changes global redirection; pads user DL with zeros | No worker-list scope emitted by GXCallDisplayList flattening | Recording lifetime/generation, padding/truncation and subsequent actual replay span |
| init | stop worker; free/reallocate/reset buffer/counters/mirror; start worker | Worker stopped, caller-writer quiescence assumed by API | New diagnostic generation; old sidecars cannot match new position0 |
| drain | publish target, waitProcessed yielding baton, then under buffer lock set base=target,size=0 | Publication/wait and rebase are distinct operations; producer exclusivity across wait not established here | Exact before/after bases and written/processed/published counters; do not infer all concurrent writes preserved |
| clear_buffer | assert published==processed; lock; base=processed,size=0 | Buffer mutex, but fast writers do not take it | Rebase generation and writer-origin restriction |
| publish / finish_draw | target=base+size; release-max published; worker wake | No copy-site sidecar exists; publication can be called from SDK paths | Sidecar metadata must already describe every exposed copy, not be appended later |
| drain / publish_all | publishes full current range, wakes worker | Separate C ABI route; not only GXCallDisplayList.publish | Same coverage, including while a different writer waits |
| abort | publishes written watermark, marks abortTo, release abort request, wakes worker | Game or documented interrupt recovery when writer blocked | Distinguish discarded spans/generation; do not equate processed jump with parsed commands |
| worker | under buffer mutex append published bytes to private mirror; unlock; process; update processed, report sync BP | Existing mirror is worker-owned; process outside buffer mutex | Absolute context processed+reader.offset, mirrorBase/generation and abort state; never cache live FIFO pointer |

The generic SDK GXCmd/GXParam functions in GXVert.cpp call these writers directly.
GXCallDisplayList adds pipeline-owner marker/dirty-state/flush writes, then bulk
write_data, then publish. Original list ownership starts at its actual bulk copy,
not at the preceding owner marker or wrapper entry. Sized/ordinary primitives and
native extensions ultimately use the same scalar/bulk APIs.

## Exact source-permitted conflicting path

The buffer-lock helper's contended branch executes:

`begin_host_wait -> lock.lock -> work -> lock.unlock -> end_host_wait`.

sync_backend.cpp forwards begin/end to the OS host-blocking API. In os_thread.cpp,
beginHostBlockingLocked marks the calling OS thread host-blocking, sets game CPU
released and gives the CPU to another ready thread (or clears the current holder).
try_begin_host_blocking returns after re-enabling interrupts. onInterruptsEnabling
does not park this host-blocking thread awaiting the baton; the explicit
end_host_blocking later requeues it and calls waitForCpu. Thus the grow closure can
execute while another OS thread holds the CPU.

A source-permitted schedule, if that other thread invokes a FIFO fast writer:

1. A enters a grow path, fails the short buffer-lock spin, releases the CPU and
   waits for the buffer mutex. B receives the CPU.
2. A obtains the buffer mutex and executes grow while its CPU remains released.
   It computes `needed=oldSize+length`; it may publish a new buffer pointer/capacity.
3. B's inline fast path sees available capacity and memcpy/increments size WITHOUT
   taking A's buffer mutex. The source contains no writer-origin exclusion check.
4. A's memcpy uses the THEN-current size, but its eventual size assignment uses the
   earlier needed value. Overlap, wrong size/span or publication during another copy
   cannot be ruled out from this contract. B may reach a publish path before A's
   grow returns to its wrapper.

This is a conditional static conflict, not an observed race or demonstrated root
cause of either invalidWorld255 or indexedXF24. No change to locks/baton is proposed.
It is exactly why even a copy-site sidecar cannot yet assert a single serialized
original byte stream merely because it ran under sBufferMutex. Fast and slow writers
must share an established exclusion property for that assertion to be valid.

A second unresolved class is arbitrary host/interrupt entry: the inline APIs do not
assert a running game CPU holder or interrupt-lock owner. The documented abort
recovery path permits interrupt activity while a writer is blocked. Need actual
producer origins; do not assume all host callers are legal or all are absent.

## Proposed exact hook locations, conditional on review

These locations preserve the order of existing operations; they are NOT emitted
as patches until the missing exclusion/caller property is established.

- Bulk fast: capture domain/start/base/size immediately before the existing memcpy;
  register a fixed metadata sidecar associated with that actual copy, complete it
  after existing size increment and before returning. No lock, allocation or
  host-wait call can be inserted in this writer critical sequence without review.
- Byte fast: analogous hook around the actual store/size increment, not an assumed
  write_data delegation. Every scalar packet can contain these writes.
- Grow: inside the existing closure AFTER needed and final allocation decisions,
  immediately before memcpy, record actual base/current size; complete after the
  existing size assignment, BEFORE releasing the existing buffer mutex. If actual
  end differs from chosen span or another writer overlaps, mark AMBIGUOUS; never
  adjust the original pointer/size or force drain/publication to make it consistent.
- Recording paths: corresponding copy/store sites with recording generation and
  dlWritePos. Record their later replay as a separate main-stream copy. Caller
  context must survive host wait and nested/interrupt reentrancy without one global
  "current DL" identity overwriting another call.
- Worker: immediately around the original process call, attach its exact absolute
  processed and mirror generation; command-start offsets derive from that immutable
  call input. Preserve all original abort checks, returns and BP reporting.
- Publish/abort/rebase: record only the already-computed original watermarks/counters
  and generation transitions, with no added publish, wait, padding or state repair.

## Lock-order and data-publishing review

Existing worker mirror-copy takes buffer mutex and releases it before parsing.
Growth holds buffer mutex while copying; its contended path can operate without
the CPU baton. A prospective sidecar lock taken by growth establishes buffer->sidecar.
No worker path may take sidecar then buffer; no output flush, symbolization,
allocator, GX operation, CPU reacquisition or arbitrary callback is allowed while
holding sidecar. Metadata must be immutable to its reader, with proper C++ memory
ordering and an eviction generation; plain non-atomic seqlock copying is a data race.
A global sidecar mutex could also block a game CPU holder, so it cannot be assumed
harmless simply because the payload is small. No new lock is approved by this review.

Registration-before-publication must cover every publish route, not just the
wrapper's final publish. If exclusion is established, registering at actual copy
sites before advancing/exposing the original write frontier is feasible. Without
it, another writer may expose an in-progress copy. A diagnostic must say this is
AMBIGUOUS instead of silently assigning the whole interval to the closest caller.

Growth reallocations change host pointers but not absolute positions. A sidecar
must store integer stream spans, not live buffer pointers. drain/clear can change
base without resetting global logical counters; init resets them and requires a
new generation. Abort can advance processed over discarded bytes: those positions
are not evidence of successful parser consumption. Existing private mirror bytes
are the correct raw snapshot source while valid, not a producer pointer reread.

## Minimal missing evidence / decision

Before generating usable hooks, obtain ONE of:

1. A source/runtime contract showing that no other CPU/host/interrupt writer can
   enter fast copy/publication while a host-blocking grow-copy is active; it must
   include generic GXCmd/GXParam and raw host callback entry, not just game DL calls.
2. A separately reviewed diagnostic-only overlap/origin observation: bounded atomic
   entry/exit records at ALL writer/copy/publication sites, with host identity,
   bound OSThread/CPU-holder state, host-blocking flag, interrupt state, copy domain,
   base/size/capacity and generation. It must not lock or serialize original writers,
   and any simultaneous writer/publication evidence makes list provenance LIMITED.
   Review C++ data-race-free observation and output first; do not sample mutable
   plain global fields from an unrelated thread as if they were an atomic tuple.

Absence of overlap in one run is not proof of a general serialization contract.
An actual concurrent writer trace would be a PORT source issue to investigate
separately; it is not permission for this diagnostic generator to fix synchronization.
No forced original ordering, guessed span, wrapper fallback or passing placeholder.

The previous sixteen lightweight generator/negative tests still pass; no new C++
patch or runtime test is claimed. The source anchor report preserves the precise
blocking witness and expected hook sites for PORT's next ownership decision.

# Isolated native draw observer: source candidate

This component is for the one proposed RedBlueExGalaxy placed scenario1 visual
pilot. It is not installed, built or runtime qualified. `drawObserverCallbacks()`
returns nullptr until explicit registration. No existing source or CMake file is
changed. The phase-v2 adapter and checked-PNG runner remain unchanged; the latter
is disabled. This component cannot produce an accepted phase-v2 provider.

## Exact GAME interface

Include `"visual/draw_observer.hpp"` under `GALAXY_NATIVE` in GameSystem.cpp.
Immutable33e71 root `petari_native_config` exposes `native/app` and is linked by
`petari_game_objects`; no include-search edit is required. GAME owner B owns all
three guarded callsites, with a fresh accessor and callback-null check per phase:

```
if (const auto* cb = PetariNative::Visual::drawObserverCallbacks())
    if (cb->loopEntry) cb->loopEntry(this, MainLoopFramework::sManager);
// existing beginRender()
if (const auto* cb = PetariNative::Visual::drawObserverCallbacks())
    if (cb->drawEntry) cb->drawEntry(this, MainLoopFramework::sManager);
// existing draw()
if (const auto* cb = PetariNative::Visual::drawObserverCallbacks())
    if (cb->drawExit) cb->drawExit(this, MainLoopFramework::sManager);
// existing unconditional endRender(), then update/calcAnim/endFrame
```

The literal original calls are preserved by GAME; these comments are only an
integration recipe. Do not wrap any original call in an observer condition. No
MainLoopFramework, preRender callback, renderer, FIFO, command worker or Wii change
is part of this component. GAME/E must independently review the exact paired pins.

The callback parameters are borrowed const pointers, valid only synchronously
while Game CPU owns the objects. `readDrawSnapshot` copies bounded strings and
integer/control metadata; it does not retain these pointers. `getCurrentSceneForExecute`
is called locally and only its nonnull/equality relations survive. Canonical
MarioHolder/actor/Mario/position-holder checks are gated on Game/init6 and scene
object-holder presence, without reading any position, RNG, input or float value.
Object initialization/lifetime at the proposed callsites still needs GAME review.

## Ownership and bounded storage

One Game-CPU owner performs begin/install, callbacks, postEndFrame and uninstall.
This is a precondition, not runtime thread authentication: there are no locks,
atomics, waits, thread changes or cross-thread registration. Install/remove must
occur at a quiescent owner boundary, never during a callback; dispatch reentry
invalidates capture. The observer and sampler must outlive installation. The
registration globals retain only the observer and function pointers; event payloads
contain no pointers. Concurrent export, registration, destruction or observation
is unsupported. No live Game traversal is permitted after the host releases CPU.

`DrawObserver` has4096 append-only POD slots. Every loop contributes loop-entry,
draw-entry, draw-exit and a separate post-endFrame context event;616 loops consume
2464 slots. There is no rolling overwrite, unbounded queue, callback allocation,
formatting, file I/O, FP arithmetic/mode change, renderer call or exception path.
Capacity, phase/order, serial overflow and invalid snapshots latch a fault. Later
callbacks stop sampling; the Game continues unchanged. Exported faulty or partial
journals remain evidence of incomplete observation, never usable captures.

Draw serials are assigned only at actual draw-entry. Loop-entry keeps the previous
draw serial. False render flags do not suppress draw events. Each phase copies the
actual framework flag/interval/counter and XFB mode/indices, so old/new flags are
observed separately. These fields are control observations, not proofs of clear,
copy, pixels or GPU execution. A null framework/XFB is explicit unavailable data.

## Source context and readiness

The caller must `begin` with detached first-ready post-endFrame context and later
call `postEndFrame` with every consecutive actual source seam before another loop.
This host bridge is NOT installed here. SourceContext carries a copied key, actual
source frame and caller-sampled epoch. No internal Game pointer map, synthetic
lifecycle ID, inferred frame offset or reanchor is created. Keys include placed
AND selected scenarios. A draw/context key mismatch is retained explicitly; it
must not inherit a qualified epoch. Even matching keys do not prove allocation
continuity or an authoritative lifecycle generation.

`initializedGame` means typed Game/init6 storage. `executedSceneIsCurrent` means the
actual selected execute scene equalled current scene, independently sampled.
Init6 can precede Normal/selected draw; conversely an unready current scene may
still be selected. Neither is collapsed into the other or into no-draw. The JSON
reviewer preserves these cases and reports only pilot metadata candidates.
Requested post-endFrame state, actual draw entry/exit, observed-key epoch and
lifecycle remain distinct. This journal is not a full drawn-state trace.

## Future build and integration recipe — NOT EXECUTED

After source review and a separate lock/build allocation, a private standalone
fixture can compile only core/exporter/tests with the existing pinned compiler:

```
cmake -S native/app/visual -B PRIVATE/draw-observer-core -DCMAKE_BUILD_TYPE=Debug
cmake --build PRIVATE/draw-observer-core -j1
ctest --test-dir PRIVATE/draw-observer-core --output-on-failure
```

Run the entire bounded job nice19 under the actual host lock and a reviewed owned
process runner. No runner is supplied or implicitly approved here. Native controls
are currently UNRUN. They exercise default-off, install lifetime, bounded storage,
phase/order/missing seam, selected-vs-initialized, false old/new flag distinction,
invalid/truncated strings, source-key mismatch, export-while-installed and writer
failure. Python journal negatives are synthetic parser evidence only, not evidence
that C++ compiled or these controls ran.

For an allocated full native integration on immutable33e71 plus reviewed patches:

1. Owner adds `add_subdirectory(native/app/visual)` after game-object creation, then
   calls `petari_link_draw_observer_to_game()`. Its PUBLIC object-target dependency
   supplies the real nullptr-default accessor to BOTH normal GX and headless final
   consumers of `petari_game_objects`. Do not link only the GX host. Explicit
   `$<TARGET_OBJECTS>`/partial-link diagnostic trap recipes do not automatically
   inherit this edge: owner must add the core archive to those inventories too.
2. The opt-in native capture driver links `petari_draw_observer_game`, preserving
   `petari_native_config`, existing sanitized/native Game compile flags and source
   encoding dependencies. Compile `draw_snapshot.cpp` in that actual context.
   No fallback accessor or guessed macro selection is provided. Both GX/headless
   default-off link checks, symbol inventory and unchanged control traces are
   mandatory before installation can be considered.
3. On the same Game CPU owner boundary, after the independent ready scheduler
   observes its actual first ready anchor, allocate/begin the fixed observer and
   install with `readDrawSnapshot`. Allocation is outside callbacks. An owned host
   bridge must deliver every subsequent detached post-endFrame context BEFORE CPU
   release/resume. Stop exactly after616 inputs; do not advance CPU for GPU drain.
   No bridge, input scheduler or stop guard is installed by this source delivery.
4. Uninstall on the same owner after the last completed loop, before destruction or
   CPU release. Only then export detached data. `writeDrawObservations` reports
   ostream failure and performs no flush/close/fsync/atomic publication or hash.
   The future capture driver owns those checked operations, closed input/census
   receipts, independently reviewed source/binary/operator pins and stop proof.

## Concrete remaining one-pilot path

GAME supplies reviewed guarded brackets; native build owner supplies all-consumer
linkage and same-owner opt-in host/seam bridge; native observer captures only this
metadata journal. The next owner task is that small bridge plus source/binary and
bounded input-stop receipts, independently reviewed before any run. Then one
allocated default-off control and one616-input RedBlueEx capture may test actual
ordering/typed metadata. Neither grants pixels or visual comparison admission.

Copy tags/current-draw+endGX/clear writer lineage, resource versions/overwrite
coverage, first policy selection, GPU completion/staging, viewport/PAR/color and
checked PNG publication remain unimplemented. They require separately reviewed
owner hooks; this delivery adds none and cannot replace them with pointer or
counter coincidence. A Dolphin host bridge has analogous missing actual evidence.
Only after these are bound can the new phase-v2 adapter inspect one content pair.

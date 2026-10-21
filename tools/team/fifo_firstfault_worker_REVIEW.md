# Worker-only first-fault source-generation handoff

This is diagnostic source generation only. No C++ compile/runtime has been run by
P. T2 owns the later allocated native build and test/run gates. E independently
reviewed worker-r3 exact bytes; do not change them without renewed delta review.

Pinned actual source b1cf672a6c51dec9e588932876ce6f963b6052f1238fa19151d0edf022c940fc;
actual compiled worker object f50d2a595781a54826c15a3e2276127befcda516c9a7b8dfdf01307f4ec2d190.
Typed bridge771440cb3794f96b622e1a38f31642cd5d66eb463535010eebc4a9dd28df06a3
binds source8c9430f8/effectivefff36666/binaryf579a534 and exact native toolchain.
The original process() runs on the existing sole FIFO worker, on its private
mirror OUTSIDE sBufferMutex (effective FIFO85c6833b). No producer reads or new locks.

Generated immutable package: scratch-port/solP/fifo-worker-generated/worker-r3/:

- command_processor.cpp: 27faa4fcf6772a7dfc4d3e2be2fcae5e1cf6b4bd678e49c9c5b84c4e3924bc41
- fifo_firstfault_worker_capture.hpp: 338e1492fd12f78374559acfe4eb0237079de95527430137cc729028b2454c4e
- worker.patch: a097ecb105e73363afb291b263de4960d544e63e5f26db262e5ca33682b7459a

CPP includes that immutable P-owned header by absolute path; preserve it through
T2 build/runtime. All11 tagged insertion blocks independently strip to the exact
original source. No original read/take/skip, warning/assert/fatal or state operation
is removed, reordered or replaced. Scope guards clear the borrowed span pointer on
all normal exits (including abort/BP returns); process abort/fatal does not return.
Primitive guard borrows its caller's still-live Reader only through draw_prim return.

Default OFF: PETARI_FIFO_FIRSTFAULT absent or anything except literal1. Runtime ON:
PETARI_FIFO_FIRSTFAULT=1. Marker required in diagnostic binary/log:
P_FIFO_WORKER_FIRSTFAULT_v1. Output is fixed in this generated header to the private
M1 directory ~/MacioGalaxy2-gamedata/solP-fifo-firstfault/worker-r3/firstfault.log.
Parent is precreated700; openat EXCL/NOFOLLOW and fchmod600 refuse existing output.
Do not reuse a runtime file. Synthetic control files must be renamed/preserved
before the one authorized app run; never remove an existing gameplay capture.

Fixed64command/32binding/2completed-primitive histories disclose totals and loss.
Extra reads are bounded within current mirror span or existing worker-owned scalar
metadata, with fmt/array indices checked. No unmapped array data/CALL target is read.
Raw current-span window cap65536. Each of at most2 first-trigger snapshots uses one
512KiB text buffer; bounded manual integer/text conversion, no Cstdio/C++ formatter,
heap call, symbolizer or game traversal.15 file write attempts plus at most1 literal
stderr write =16 TOTAL writes per flush. No fsync/retry loop/fallback parser behavior.
I/O may block/perturb timing; failed or truncated output is LIMIT, never success.

The observation reports raw opcode before masking, span-local offset/remaining,
preceding primitive VCD/VAT/cache/byte decision, CP/native-array history and index
operands. OriginalDL/caller/absoluteownership/truealignment remain UNKNOWN. A
process span is not a DL, and a local offset is not a list offset. GAME's producer
attribution contract remains an interpretation limit: material/preVCD/loopdiff may
emit before shape counters; no producer attribution follows from these snapshots.

Lightweight command: nice -n19 python3 -B -m unittest tools/team/fifo_firstfault_worker_tests.py
18 generator/source negative tests PASS; they are not C++ runtime validation.
Native compile, default-OFF behavior, bounded/truncation and forced-I/O controls,
then exact typed normal-host-route run remain PENDING PORT/T2 allocation.

r1/r2 retained: r1 lacked primitive eviction loss/totals; r2 resolved those and
removed C stdio formatting. r3's only functional delta from independently reviewed
r2 is15 file attempts (instead of16), ensuring16TOTAL with the error write. Owned
include/capture directory literals changed r2->r3; all other bytes unchanged.

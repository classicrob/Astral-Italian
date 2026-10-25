# Required Release codegen acceptance for the default-off trace

This is required for any trace build that might land on main. Source erasure
and disabled-argument unit tests do not establish compiler placement. The
38572b67 app failed this check: six trace TLS resolvers ran unconditionally
at worker startup, despite every source use being inside an enabled guard.

Run this on the EXACT new Release app and petari_gpu_trace_tests executable.
The M1 build and all native execution wait for the assigned heavy -> native
slot after SOL B r7 and the decomp Mario proof. No artifact from a different
build or a test-only follow-up substitutes for its corresponding codegen.

1. Record app source/executable SHA256, renderer key, UUID and matching dSYM;
   record the test source and executable SHA separately. Preserve the test
   executable until its disassembly is extracted and bound. Delete build dirs
   per head's rule only after recording these results. Do not run/copy/modify
   an immutable app to inspect it.
2. Find the actual trace enabled symbol and its GOT binding; inspect the image
   initializer (false). Resolve production function ranges through the matching
   app dSYM, then disassemble those ranges from the APP executable. A dSYM's
   debug image alone does not contain executable instructions. Preserve the
   bounded output and its hash, command/tool and all relevant addresses.
3. Inspect render_worker::worker_main from function ENTRY, not just its work
   item loop. Before the first enabled-byte check, there must be no trace TLS
   address resolution or trace helper call. Ordinary HostAllocationScope,
   thread identity and pre-existing worker setup are distinct; resolve GOT
   symbols rather than guessing from indirect BLR instructions. Inspect the
   enabled-false edges around begin/end-work and queue wait hooks as well.
4. Inspect giveCpu and waitForCpu from entry and at each hook. A cached relaxed
   enabled-byte load/conditional skip must dominate the trace body. Trace-only
   clock/QoS/CPU queries, trace TLS resolvers, argument dereferences, counter/ring
   work and event construction must remain on the enabled side. Pre-existing
   scheduler clocks, OS TLS and host waits are not trace work; use source/dSYM
   identity to distinguish them. Check outlined helper calls are conditional;
   their trace TLS resolutions belong inside those helpers. Inspect register
   spills/prologues honestly; this is not a zero-cycle performance proof.
5. Disassemble petari_gpu_trace_off_probe from the retained TEST executable.
   Its disabled edge must avoid pointer access, TLS, host queries, IDs/counters,
   waits and all event work. Preserve the test executable SHA and this code.
   Do not search for the test-only function in the app and infer success from
   its absence. Missing executable/symbol/binding means PENDING, not PASS.
6. Record per-function/probe status (PASS/FAIL/PENDING), flag/GOT identities,
   skip branch targets and any unconditional trace instructions. An observed
   hoist is FAIL even if it runs only once per thread. Preserve that failure;
   revise source and inspect a fresh build. A source annotation such as
   noinline is a prevention mechanism, not a substitute for machine-code proof.
7. Run the native GPU trace/patch/analysis/attribution and existing scheduler,
   seam/VI/GX/pipeline/tape fixtures without changing predicates. Record test
   results separately from this static proof. Run the same-new-executable
   off/on fixed300s prime/warm cohort and review its drop/footer/CSV mapping and
   distributions before using trace timing causally. Codegen PASS does not
   grant overhead qualification or functional acceptance.

Until all four codegen subjects pass on the new build, default-off compliance
is unproven. Until the fresh overhead cohort passes review, trace-on cost is
unqualified. No timer/scheduler fix or <=1.5 ms priority-handoff success follows
from either static proof alone.

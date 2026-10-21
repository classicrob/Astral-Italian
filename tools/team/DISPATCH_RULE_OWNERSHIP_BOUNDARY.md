# Fixed Ninja rule: missing ownership boundary

Status: SOURCE ONLY / BLOCKED. This is a refinement of 2641a55f, not an
implementation, execution plan, allocation, or cleanup qualification. L's
independent 2641 disposition (receipt SHA8bfd70ab prefix) agrees that normal
shell completion is distinct from bounded descendant cleanup. Existing caller
and compiler-gate refusals remain unchanged; full827 remains disabled.

## Actual creators and ordering

The pinned Ninja Subprocess::Start calls posix_spawn with SETPGROUP and the
non-console default pgroup=0. The OS creates the shell's group at spawn; Ninja
receives the direct shell PID in pid_. The shell executes /bin/sh -c with the
original rule string and inherited environ. The shell can run before
posix_spawn returns to Ninja. There is no controller registration or external
acknowledgement between group creation and rule execution. Registering the PID
after spawn, in StartCommand, or after the outer gate acknowledgement is too
late to establish a before-work boundary.

The rule is the exact pinned wibo wrapper -> sjiswrap -> MWCC invocation followed
by && Python transform_dep. The current wrapper performs slot acquisition and
execv(wibo.real); it neither publishes an ownership record nor waits for an ack.
Even a future wrapper-only hook would occur after shell execution and would not
cover shell startup or the later transform_dep command. The wrapper cannot be
assumed to retain the shell PID: shell execution/optimization and subsequent
commands are not an observed lineage join. No opaque wibo descendant behavior
is inferred from this source review.

## Smallest sufficient interface for this one rule (not implemented)

A before-rule barrier would have to sit at the existing Ninja spawn boundary:
the entity creating the shell must keep it from executing the rule until a
controller has registered actual kernel identity and acknowledged the exact
invocation. Registration must bind a fresh nonce, Ninja identity, direct shell
PID, actual PGID/SID/UID and parent relationship, exact edge/rule digest, target,
source/tool pins and exact operator environment digest. Requested pgroup or
self-report alone cannot substitute for actual identity. Registration failure,
timeout, malformed identity or mismatch must prevent compiler work and must
retain uncertainty rather than signal a contradicted target.

The interface would need two distinct lifecycle messages:

1. Normal path: bind that registered record to the same Subprocess/edge through
   WaitForCommand -> Finish -> direct waitpid status. Retire it only after the
   controller independently establishes strict inner-group absence; outputs,
   Ninja status0, pipe EOF and direct-child reap are not that assertion.
2. Interrupted path: bind the same record through Cleanup -> Abort -> Clear
   before Ninja's record deletion. Existing Clear signal/Finish behavior is
   preserved as evidence, but it is not bounded termination. A separately
   reviewed controller contract would have to cover BOTH outer and registered
   inner group within the existing total cleanup allowance, reap owned direct
   children, and require strict group absence. No new signal implementation or
   allocation is supplied here. EPERM/unknown identity/status remains failure.

This is an inner-group boundary only, not a claim that descendants cannot create
other groups/sessions. Such escape behavior in the actual fixed tool chain
remains unqualified. Strict absence of a registered group cannot prove absence
of an escaped process, and this proposal grants no generic descendant authority.

## Why the fixed current inputs cannot implement it

The installed pinned Ninja offers no pre-rule registration/ack API. Its
posix_spawn runs the real shell directly, with no suspended-child handshake.
The existing outer exec gate controls Ninja before exec, not Ninja's later
spawned shell. Merely adding a pipe to the outer gate provides no consumer in
the unmodified Ninja/shell. Post-spawn observation cannot close the scheduling
race. No flag in this plan supplies the missing interface.

Implementing a true pre-work barrier therefore requires changing a currently
fixed input: an explicitly reviewed Ninja spawn implementation (and new binary
pin), or a rule/shell/launcher change that introduces such a barrier and changes
the official command/tool contract. A compiler-wrapper-only barrier is
insufficient for the whole shell rule. These are future scope decisions, not
workarounds implemented or requested for execution here. No console pool,
compiler flag, toolchain replacement, signal mechanism, preload/interposition,
security setting or generic framework is changed by this packet.

Under the current no-pool/flags/toolchain-change constraints, the exact boundary
cannot be implemented. Compiler enablement stays BLOCKED. A further normal
successful compiler run would add output evidence but would not resolve this
interruption boundary.

## Invariants and future acceptance requirements

- Preserve all official command tokens/order, compiler flags, dependencies,
  source/output/dep substitutions and graph bindings already declared in2641.
- Preserve exact nine-key operator mapping at parent/child/postack boundaries;
  do not introduce an ignored environment key for transport. Any proposed FD
  transport must be separately bound without altering these mappings or leaking
  into unreviewed tool processes. No ambient/default mapping is admissible.
- Bind immutable child registration to both normal and Clear paths; do not infer
  identity from ps, wrapper positions, lock availability, output or parent exit.
- Review the actual process/group creation behavior and cleanup of the fixed
  wibo/sjiswrap/MWCC/transform chain; no all-compilers/all2426 conclusion.
- Before admission, separately allocated actual interruption controls must
  cover pre-ack interruption, post-ack interruption, ignored TERM, direct shell
  exit with surviving same-group worker, registration mismatch and strict
  absence errors. No such controls were run here.

## Reproducible source intake

All Ninja anchors and hashes are in DISPATCH_NINJA_LIFECYCLE.json and the
2641 public-source packet (immutable upstream3441b633). The installed wibo
wrapper is separately SHA-pinned by DISPATCH_TOOLS_PLAN.json. This source
correspondence is not an installed-binary reproducible-build or runtime proof.

# Separate direct official-rule component proposal

SOURCE ONLY; execution disabled; implementation not authorized. This does not
activate or amend full827, the2641 Ninja plan, or the accepted isolated6b7c scope.
It is the alternative specifically requested by PORT after1416351f. No Ninja
process runs in the proposed component. The immutable official generated graph
remains command authority; L independently approves its exact expanded command.

## Fixed invocation and authority

Proposed single gate target: pinned /bin/sh, argv exactly [/bin/sh, -c, C], where
C is the independently captured official expanded rule command string. The
adapter must compare C byte-for-byte to L's expected string after only explicitly
approved source/output/dep substitutions. It must not reconstruct C by joining
all tokens (which would quote && as an argument), introduce eval, add exec/set/mode
prefixes, or split the two commands into separately supervised jobs. Token/order
and shell-operator equality are additional checks, not substitutes for command
byte equality. Ninja graph dollar escaping is removed only by the independently
approved graph expansion; shell quoting/expansion is not guessed by the caller.

The fixed rule is the official wibo/sjiswrap/Wii1.0/MWCC command && exact pinned
Python tools/transform_dep.py dep dep. Preserve every compiler flag, tool path,
source, output directory, cwd and operator-nine mapping. For the representative
synthetic source, the same three substitutions from2641 (source token66 and dep
tokens72/73) require explicit approval; no actual game-source compile is silently
substituted. Any future actual official game-source variant is a different plan
with exact repository/headers and no synthetic substitutions. No Ninja consumed
-deps claim is available: this component can bind the actual compiler depfile and
its transform, not a Ninja dependency database or graph scheduler execution.

## Identity/ack boundary and minimal adapter

Reuse the separately reviewed owned-session pre-exec gate behavior: parent
launches the pinned Python gate with explicit exact-nine env and
start_new_session=True; register actual kernel PID/PGID/SID/UID/parent identity,
request/target/source/command digests; verify before parent ack; gate rechecks
same request, target, cwd and full env after ack, then same-PID execs /bin/sh.
The shell thus starts as the already registered session/group leader. This
removes the particular Ninja posix_spawn-created inner-group race without a new
kill mechanism or compiler-wrapper change.

The accepted isolated gate currently admits ONLY its exact Python payload.
A separate fixed-shell gate/caller profile would therefore be required; it must
not widen that live gate or accept arbitrary argv. It would add only this one
operator-pinned command and /bin/sh hash to a disabled component plan, retaining
existing launch journaling/ownership/reap-before-probe/TERM1+KILL3/strictESRCH
logic. New root/TMPDIR needs a fresh L-owned operator record; tools-v1/r1 mappings
cannot be reused with a different root. No Ninth-key value is producer-derived.
This is an implementation delta description, not an implemented shell gate.

## What source actually establishes and does not establish

1. Parent/gate code supplies new-session creation and same-PID exec for its
   admitted target. The proposed shell admission has not been implemented or
   executed. Kernel identity before ack is still mandatory.
2. The SHA-pinned Python wibo wrapper only selects/locks a compile slot, marks
   its lock FD inheritable, then execv(wibo.real, original arguments). It has no
   explicit setpgid/setsid/fork/Popen call. Its exec preserves its PID; this is
   wrapper-source evidence only, not a claim about interpreter startup or real
   compiler descendants. Blocking flock is part of work time, not an exemption
   from timeout. Existing slots/pools and wrapper bytes remain unchanged.
3. /bin/sh is a pinned platform binary, wibo.real a pinned x86_64 Mach-O binary,
   sjiswrap a PE32 executable, and MWCC a separately pinned guest executable.
   Their complete actual fork/spawn/job-control/setpgid/setsid behavior is NOT
   established by the current source packet. No assumption that noninteractive
   shell children retain a group is promoted to evidence. In particular,
   absence of explicit grouping in the Python wrapper says nothing about opaque
   wibo guest/native child behavior or the shell's implementation.
4. transform_dep is exact622a; it is a later shell command, outside the wibo
   wrapper. The shell must remain the supervised fixed-rule target across both
   halves. A compiler-wrapper-only observation cannot cover its lifecycle.
5. Existing checks suffice for the registered shell leader and any descendants
   proven to remain in its group. They do NOT establish absence of descendants
   that escape that group/session. Direct shell status0, fresh ELF/dep output,
   pipe EOF and registered-group ESRCH cannot independently close that gap.

Current disposition: the Ninja-specific extra-group boundary is removed by this
separate contract design, but whole-chain descendant/session behavior remains
UNKNOWN/BLOCKED pending L-approved evidence. This is not a generic ownership or
full-command qualification, and no claim about historical successful lanes is
made. No source behavior or signal escalation is changed to obtain a PASS.

## Minimum proposed evidence and controls for review

Before any run, L/root must approve the disabled fixed-shell adapter source,
actual command bytes+tokens, independent graph expansion/substitutions, fresh
operator record, tool/SDK/header inputs, root, outputs and limits. The review
must decide a concrete way to establish actual shell/wibo/guest/transform child
lineage and group/session behavior for this exact chain. Merely sampling ps
cannot prove a transient child never escaped. If no complete source or bounded
causal observation is available, retain that qualification gap; no generic
framework, interposition or toolchain change is supplied by this proposal.

Proposed rejecting source/artifact controls (not executed): changed command byte,
flag/order/&& operator/tool/cwd/env/source/dep/transform pin; arbitrary shell
payload; duplicate/missing/tenth operator key; target replacement before ack;
unacknowledged work; stale ELF/dep; mismatched PPC function/dep target/source;
transform output inconsistent with exact raw compiler dep input. Preserve raw
compiler dep input if the reviewed observation can do so without changing the
command; otherwise label raw-input transform join unavailable, not satisfied.

Existing accepted v7 actual same-group lifecycle controls remain historical
component evidence. For this shell adapter, separately reviewed bounded actual
controls would need pre-ack interruption/no work, shell completion, failed first
command/no transform, timeout during first command and during transform, ignored
TERM, leader exit with a surviving same-group child, and failclosed unknown or
escaped group identity. No commands, signals, compilers or controls were run
for this proposal. Normal successful chain completion and adversarial timeout
cleanup must receive separate dispositions. No automatic enablement follows.

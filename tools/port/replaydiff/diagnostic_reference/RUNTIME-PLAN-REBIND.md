# Materialized-source runtime plan rebind

`RUNTIME-PLAN.materialized.disabled.json` is a separate disabled packet. It leaves
cc68 and 9b3, their receipts, and the copied 35+5 source set unchanged. The new plan
points every source pin and wrapper argv at the verified fresh M1 materialization.
Its boot-script argument directly names the immutable scripted menu source there.
The profile/socket/input/output paths name a fresh `yoshi-idle-reference-r2` root.
No new source/helper code, payload, profile, permission probe or app is installed.

The neutral suffix SHA is now the expected deterministic source hash from two
SERVER in-memory generations, not a placeholder and not proof that a payload file
exists or was delivered. The actual M1 payload is still absent. The prescribed
prepare_idle command and live profile/socket-only preparation require separate
review and allocation; this rebind does not execute them.

The current M1 metadata record identifies the actual absolute Python.app, host
query and screencapture files by hashes, plus Dolphin Info.plist and image stats.
A fresh hash of the 47.6MB Dolphin image is still required before launch. Its
expected accepted SHA remains in the wrapper's pin list, so a future launch would
refuse differing bytes; a proposed future check is not current identity evidence.
The exact interpreter/LS/runtime dependency and environment context also remains
unqualified for this new job.

## Proposed 480-second outer boundary

T0 begins before live runtime preparation, not after boot. Work ends by T0+420;
no screenshot starts after T0+407 (10 seconds command plus 3 seconds direct-child
reap). The collector's own sampling timeout is reduced to 390 seconds, but that
is not the global deadline: its preflight/boot/finalization are independently
covered by the proposed outer owner. Owned cleanup occupies at most420..465,
and failure/report/verified release finishes by480. Missing the work boundary
means discard/retain partial evidence, not another attempt or a longer deadline.

These exact limits are requirements, not implemented guarantees. The current
collector's final kill wait is unbounded; the wrapper has no qualified global
watcher or deadline-aware screenshot-start bridge. The plan explicitly refuses
allocation until the exact outer caller and these mechanical gaps are reviewed.
No general supervisor framework is introduced by this packet.

## Required ownership handoffs

LaunchServices must yield the real Python driver identity, birth/UID/group/session
and an acknowledgement before emulator execution. `open` exit0, a Python.app
bundle name, a returned PID alone or a process-list match is insufficient. Driver,
Dolphin and screencapture registration must close signal/spawn windows and retain
actual direct-child handles. Do not assume LaunchServices group inheritance.
An approved owner must prove group absence after bounded leader reap; EPERM or
unknown ownership remains failure and forbids unverified group signals. Only then
may the real native.lock be released as a successful job.

DR1 and DR2 are source-closed by E: nonzero collector status propagates, and the
current row616 must be Game/Yoshi1/init6/player-valid. Those fixes do not qualify
LS ownership, helper cleanup, actual consumed input or the displayed pixels.
The stopped CPU source row remains distinct from selected draw, XFB copy, VI,
host composition and PNG color. Native no-XFB remains NOPAIR. Even a successful
one-window capture would be diagnostic-only with BLOCKED/null qualification.

Next action: independently review the exact disabled rebind and choose an exact
existing caller adaptation, if available, for the listed unmet lifecycle gates.
There is no slot reservation, activation request, runtime, or implied exemption
from the native SDK work priority.

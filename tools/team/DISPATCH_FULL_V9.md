# V9 exact source review, execution disabled

This supersedes v8's proposed integration only. Immutable216b, v9-r1 canonical
1f67/raw32e946, accepted v7 and failed95 evidence remain untouched. This revision's
disabled contract is canonical827b07b3 / raw8114e6f8 under exact
`~/MacioGalaxy2-team/scratch-port/solP/dispatch-full-v9-r2`. Phase roots are exactly
ROOT/execution/before and ROOT/execution/after. No candidate appends or raw FIFO
analysis are authorized. Source preparation used existing r5 objects and Git
reads only; it did not run a graph generator, compiler, app or lifecycle diagnostic.

## Per-launch leaf and accepted lifecycle

`dispatch_exec_gate.py` is the small independently reusable SOURCE leaf, not a
qualified general runner. Parent writes a private EXCL request containing a fresh
32-hex nonce, ORIGINAL argv/cwd, exact minimal environment, resolved target+SHA,
gate SHA and Python SHA. Request file SHA is the acknowledgement token; every
launch has its own fresh directory. Child self-expires10s while unacknowledged,
validates its exact target/context and publishes identity.json (actual
PID/PGID/SID/UID) plus target.json binding PID to request SHA/target/cwd. Parent
independently checks registered leader/kernel group/session/UID and target data,
then publishes ack.json={pid,request_sha256}. Only then does the child recheck,
cancel its own alarm and exec the ORIGINAL argv without flag rewriting. PID,
group, session and cwd are preserved across exec. No acknowledgement is issued
during cleanup or after the work deadline. Unknown/contradictory ownership blocks
signals, never guesses permission or absence.

FullBudgetJob wraps this leaf using the accepted atomic spawn/register/child mask
and serial group-retirement sequence. The normal direct-child wait/status is now
journaled; failure cleanup reuses accepted v7 TERM1/KILL3, bounded leader reap
BEFORE strict group probe. A reaped leader never means group absence; EPERM is
still uncertainty/error. Final budget records retain the bounded launch inventory
and exact per-op journal. It supports only the operator-pinned caller's Git,
Ninja and exact Python targets; it is not arbitrary daemon/escaping-group
management. Shared compiler wrappers/flags/outputs are not modified. Original
wibo slot/exec behavior is preserved. Actual chosen-context execution is UNRUN.

Exact minimal HOME/PATH/LANG/LC_ALL/TMPDIR/Python startup environment is pinned;
parent rechecks target and `/bin/sh`, `/usr/bin/env`, PATH-python3 resolution/hashes
before each launch. Both phase transform_dep.py hashes are622a2c65. Target M1
Ninjaa75ab11 is retained; server metadata Ninja1474a6 is not substituted. L's
independently sealed2426 rows/control tokens remain authority, never producer
commands. Python's ORIGINAL argv0 is preserved for generator command spelling.

## Finite budgets and monitor API

Scans fail at200000 entries or a sampled2s, reject missing/non-directory roots,
propagate permission errors, ignore only disappearing child entries, and never
follow symlinks. Original baseline is reused by packaging. Both stages enforce
50GiB+16MiB free and4GiB-16MiB growth, work2400/600s and cleanup4s. This is sampled
enforcement, not an OS syscall-latency or filesystem-quota guarantee.

Publisher aligns L3bacfa28: receipt.monitors has execution/packaging, each exactly
budget/cleanup/completion confined hash records. Completion is sealed only AFTER
successful with-job teardown and binds final report bytes, clocks/root/contract
and actual-result or encoded-archive identities. No producer helper runs in the
independent importer. Six original provenance keys remain unchanged.

All archives/evidence/parts and their hashes close INSIDE packaging monitor. Then
the fixed metadata postlude copies six final small monitor files and writes
receipt+transfer (eight initially absent paths). Serialized bytes are checked
before writes; actual root allocation/floor/delta are checked afterward against
16MiB and original baseline. Detached terminal-publication.json binds all eight
file bytes/hashes/allocation observations, final receipt/transfer and contract;
its own file has an explicit64KiB allocation reserve and is externally hash-pinned.
It never self-hashes or opens archives in this postlude. No generic unchanged
certificate or landing hook is produced.

## Review and next bounded step

21 new no-process/filesystem models PASS; syntax/diff checks PASS. They cover
finite scan/missing/error/baseline, context/request/ack/cleanup-admission rejection,
and completion/detached publication. They do NOT qualify actual generic launches,
compiler workers, full package or importer. Initial source-model variable-shadow
failure was corrected before this seal; no actual command or control was retried.

`DISPATCH_GATE_CHECK_PLAN.json` is disabled. Its future separately reviewed <=60s
chosen-tool proposal runs ONLY exact Python post-exec identity, Ninja --version
and Git --version once, with nice19/nonblocking lock/fresh root,20s work/4s
cleanup/64MiB/50GiB floor. No compiler, app, v7 repetition or candidate append.
Fresh root is `solP/dispatch-gate-check-v9-r1`. Source/target/context pins and
exact argv are in that plan; enabled-plan repin requires independent review and
new PORT allocation. T2's release alone does not authorize it.

After exact source/API approval and chosen-context acceptance, the full source
must be explicitly repinned/enabled and separately allocated. Existing spec,
TU13/TU14 plans,2424 strict remainder, complete references, both source controls,
26+8 negatives and4855+3 archive contracts remain mandatory. Full proof and
production importer/hook qualification are still UNRUN. Expected full pair stays
18–25min once those explicit gates close, not authorization to start now.

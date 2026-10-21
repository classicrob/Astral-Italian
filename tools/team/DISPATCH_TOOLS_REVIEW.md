# Tools-v1 SOURCE draft — compiler execution BLOCKED

The accepted 6b7c isolated Python observation and its 12 private files are unchanged.
No tool/compiler/child/signal run occurred here. All new plan execution remains
false. The new bundle caller also has an UNCONDITIONAL pre-lock refusal, and the
new gate rejects compiler_rule regardless of an enabled bool. This draft cannot
be activated by a bool-only repin. Full827/shared-eight/importer remain untouched.

## Exact disabled component and what it would establish

| Fixed profile | Evidence if separately reviewed and later successful | Does not establish |
| --- | --- | --- |
| Python identity + actual full mapping | Same gate/postexec identity and exact fresh operator9 bytes/hash | Other targets or startup stability |
| Pinned Ninja --version | Exact binary/preexec9 + version output/status | Shell/worker cleanup |
| Pinned Git --version | Exact binary/preexec9 + version output/status | Other Git commands/descendants |
| ONE Ninja representative rule | Fresh PPC REL ELF + expected global synthetic function, unchanged source/transform, pinned toolchain and official token join | Semantic return value/DOL match, all8 compilers/all2426, guest UID/native postexec env or interruption cleanup |
| Ninja -t deps for that object | Actual consumed single-source dep, retained transformed depfile, .ninja_deps/.ninja_log/output hashes | Full repository/header dependency closure |

Fresh L operator r2 raw SHA9d06ff86f3da0eb540a0af5092a0c1490769bc007afeb3d0cc0c806ba5782a00,
full9 SHA2a7504b4427fb6b9d94985d954b6ea950c168a6ae23f271c09d175dc0a8fc0c8,
TMPDIR=tools-v1/tmp. Actual bytes remain private L/M1, not committed/copied here.

Source rule derives TU13 official tokens from unchanged canonical spec3b0975.
Only token66 source CPP and tokens72/73 dependency paths change. Output directory
and EVERY flag/tool/order token stay identical. The graph edge output and depfile
binding separately change TU13.o/.d -> solPCompilerContext.o/.d. Relative fixture
cwd is explicit. Empty include-search directories contain NO game headers/assets;
probe is one ASCII extern-C function. This is synthetic compiler emission, never
an object-identity or game-code proof. Transform source remains exact622a.

The rule retains deps=gcc. Ninja -d keepdepfile is an explicitly declared artifact
retention option; the official compiler flags and postprocessor command do not
change. The subsequent fixed -t deps query is metadata-only. Upstream 1.13.2
ninja.cc1392-1394 implements this retention switch; no tool was run to test it.

Plan proposes one sequential job, 60s total/20s sampled work+4s cleanup/64MiB/
floor50GiB/nice19/nonblocking lock/freshroot, no retries/extensions. Estimates are
not measurements. Raw requests/budget provenance would remain private M1 only.
No new version-only run is allocated; a separate version-only plan is not created.

## Exact pinned upstream lifecycle source (not actual runtime evidence)

Upstream v1.13.2 resolved through the recorded GitHub tag ref to immutable commit
`3441b633c2fe2c494e958780ba0f4227b1327634`. Full public source snapshots and byte
hashes are in `ninja-1.13.2-lifecycle-source/SOURCE-PINS.json` (scratch). Installed
Ninja SHA a75ab11c is independently pinned. Version correspondence is metadata;
this is NOT a reproducible-build proof of that installed binary. Earlier chat
links to cached tag-rendered source had different line numbers; immutable commit
anchors below supersede those line numbers without changing the finding.

- [subprocess-posix.cc101-104](https://github.com/ninja-build/ninja/blob/3441b633c2fe2c494e958780ba0f4227b1327634/src/subprocess-posix.cc#L101): non-console spawn requests a distinct process group.
- [subprocess-posix.cc132-135](https://github.com/ninja-build/ninja/blob/3441b633c2fe2c494e958780ba0f4227b1327634/src/subprocess-posix.cc#L132): /bin/sh -c receives inherited environ.
- [real_command_runner.cc97-116](https://github.com/ninja-build/ninja/blob/3441b633c2fe2c494e958780ba0f4227b1327634/src/real_command_runner.cc#L97): normal completion calls Finish for the completed subprocess and returns its status.
- [subprocess-posix.cc164-183](https://github.com/ninja-build/ninja/blob/3441b633c2fe2c494e958780ba0f4227b1327634/src/subprocess-posix.cc#L164): Finish uses blocking waitpid for its direct child.
- [build.cc770-779](https://github.com/ninja-build/ninja/blob/3441b633c2fe2c494e958780ba0f4227b1327634/src/build.cc#L770): interrupted WaitForCommand enters Cleanup; [Cleanup628-631](https://github.com/ninja-build/ninja/blob/3441b633c2fe2c494e958780ba0f4227b1327634/src/build.cc#L628) calls Abort.
- [real_command_runner.cc53-56](https://github.com/ninja-build/ninja/blob/3441b633c2fe2c494e958780ba0f4227b1327634/src/real_command_runner.cc#L53): Abort calls Clear.
- [subprocess-posix.cc451-462](https://github.com/ninja-build/ninja/blob/3441b633c2fe2c494e958780ba0f4227b1327634/src/subprocess-posix.cc#L451): Clear forwards the interruption to child groups then deletes subprocess records, whose destructor calls Finish. It supplies no local deadline/KILL escalation.

## Proven gap and smallest remaining boundary

Normal Ninja zero status plus valid fresh output/dependency joins would evidence
its normal direct-shell completion path. That does NOT establish bounded cleanup
when the outer job is interrupted. The source permits this UNTESTED scenario:
Ninja forwards TERM, then waits for its separate shell group; if that group does
not terminate within the outer one-second grace, outer KILL can terminate Ninja
without targeting the shell group. Strict absence of Ninja's group would then
be insufficient. This is a source-level coverage gap, NOT an observed compiler
failure or a claim that the pinned wibo ignores TERM. No retroactive conclusion
about other lanes, historical receipts or accepted 6b7c is warranted.

For THIS one unmodified official rule, the minimum missing boundary is a reviewed
way to obtain/register its actual shell PID/PGID/SID and parent lineage BEFORE
it can execute compiler work, then independently establish bounded cleanup of
that specific child group after Ninja termination. Current unmodified Ninja
exports no such identity/ack interface to this controller. Parent-only ps/leader
reap/groupzero, normal-output success, or assuming Ninja Clear always finishes
cannot replace it. The required actual interruption/TERM-ignored tests remain
UNRUN and require separately reviewed source/interface + allocation first.

No console-pool substitution, flag/toolchain patch, alternate kill mechanism,
blind descendant signals or generic process framework is implemented/proposed as
admission here. Under current constraints compiler runtime stays BLOCKED pending
L/root disposition of that precise boundary. The slot wrapper's audited exec
path and opaque x86_64 wibo.real do not independently close it.

## Full caller/package gaps (not solved by this component)

1. Fresh FULL-root operator mapping/trust and exact official caller argv/context
   bindings, independently reviewed integration of strict9 into that caller/gate;
   tools-v1 operator is ROOT-SPECIFIC and cannot be reused with different TMPDIR.
2. The known Ninja shell-group interruption boundary above; other seven compiler
   variants, assembler/fixup and actual full graph remain UNRUN in this context.
3. Full generic launch source/command/header/environment bounds and operator phase
   roots, actual whole-job monitor budgets/completion/terminal publication, exact
   full source/contract repins. The source-level packaging lock/transform checks
   were separately accepted27b3; they are not actual full-context qualification.
4. Actual fresh2426+2426,2424strict/two improvement envelopes, full alias closure,
   BOTH source perturb/restore, primary4855+secondary3 package and L3bac importer
   qualification/hook authority remain OPEN. Canonical spec/cert never rewritten.

Six source/tmpfile/synthetic ELF/dependency/gate models pass; no process or signal
execution, no v7 repeat. They test recipe/token boundaries, pinned fixture drift,
ELF parser rejections, consumed-dep joins, fixed native-profile gate rejection of
unknown extras, and explicit-env/disabled source shape. They cannot qualify the
known separate-group lifecycle. Prior unallocated pre-block draft retained as
`dispatch-tools-preblock-draft-r1`; no accepted artifact was replaced.

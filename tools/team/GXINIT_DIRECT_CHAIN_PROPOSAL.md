# Exact 9789 direct-chain proposal — execution disabled

The four argv arrays, 31 selected input pins, cwd, outputs, dependency contract,
linker alias and expected statuses are bound in the adjacent JSON to authority
9789adae. Nothing in the refused production caller or its plan changed. This is
source/context review, not a compiler allocation or an enabled profile.

## What is bound

The two compile commands invoke pinned clang++ directly, without shell, Ninja,
bounded wrappers or a replacement toolchain. Their only token differences are
indices 45/47/48/53: arm depfile, dep target, source and binary output. Driver
flags, order, includes, SDK, explicit absolute ld64.lld, three static libraries,
-v and -MD/-MF/-MT remain exact. Actual aliases resolve clang++ to clang-20 and
ld64.lld to lld; large executable/archive bytes were not rehashed here. The two
runtime targets must be fresh compiled outputs sealed by the existing caller;
expected exits 17/0 do not independently establish their semantic acceptance.

The exact old/candidate fixture source hashes match authority. A lexical check
found no explicit process-launch primitives in those two files. This is not a
transitive-header/library/startup or escaped-process proof. B/E still own fixture
semantics; this work does not repeat the VAT/defaults cause review.

T2's installation pointer eba0b109 and existing attestation d29735b7 identify
archive a9a22f45 and LLVM commit87f0227. I independently resolved the annotated
tag and pinned source bytes at that commit. The relevant chain is conditional:

- [Clang job execution](https://github.com/llvm/llvm-project/blob/87f0227cb60147a26a1eeb4fb06e3b505e9c7261/clang/lib/Driver/Job.cpp#L324)
  dispatches ordinary external jobs through ExecuteAndWait; CC1 can execute in
  process or use that external path. Exact runtime CC1 mode is unobserved.
- [ExecuteAndWait](https://github.com/llvm/llvm-project/blob/87f0227cb60147a26a1eeb4fb06e3b505e9c7261/llvm/lib/Support/Program.cpp#L32)
  selects non-detached execution and waits for the requested job PID.
- [Unix launch](https://github.com/llvm/llvm-project/blob/87f0227cb60147a26a1eeb4fb06e3b505e9c7261/llvm/lib/Support/Unix/Program.inc#L195)
  uses spawn without attributes where enabled. Its fork fallback requests a new
  session only for detached execution. This supports the expected inherited-group
  path if installed-source/build and effective-job closure are established.

Installed clang config45eb20f2 declares no compiled system/user configuration
directory and CLANG_SPAWN_CC1=0. The toolchain bin directory has no .cfg entries.
These are bounded installation metadata observations, not proof those declarations
match the actual executable. Exact archived executable/staticlib/config members
have not yet been compared with the installed authority pins. No archive scan,
extraction, tool/version/diagnostic invocation or compilation occurred here.

## The remaining gates, in order

1. **Complete the installed-tool join.** The smallest additional supply-chain check
   is a bounded streaming comparison of the exact archived clang-20/lld members,
   their aliases, the three libraries and generated clang config with installed
   authority pins. Retain the existing attested archive identity; no LLVM rebuild
   or permissive source-version inference. This needs its own resource allocation
   if L cannot bind it from already retained evidence. Extra compiler config,
   plugins or jobs remain rejection conditions, not paths to normalize away.

2. **Bind the native-four context.** L must independently pin the full exact-nine
   mapping for the unchanged9789 output/tmp root. Before-G operator c219 is NOT
   reusable. Parent, immutable request, gate and same-PID pre-exec checks need
   the same fixed context/root/source/interpreter/tool joins. Existing code stays
   refused until that source delta is separately reviewed.

3. **Disposition post-G cleanup in the SAME helper.** The accepted before-G path
   closed stdin and reaped before calling helper0fcd. After G, stdin closure does
   not stop clang. The existing helper's initial group probe and termination/
   poll/reap ordering was not exercised by that component. L must either qualify
   this exact post-G ordering with its fail-closed boundaries or review a narrow
   reap-before-absence ordering change in the same helper, using the already
   reviewed TERM/KILL contract. No second cleanup attempt, EPERM waiver,
   alternative kill path or another supervisor is proposed.

4. **Exercise only the untested post-G boundary.** A separately reviewed source
   plan can use existing self-expiring same-group fixture patterns under the same
   registered direct-exec gate: (a) leader ignores TERM, and (b) leader exits while
   its same-group worker remains. These cover escalation and descendant closure
   that EOF controls did not. They need actual identities/status, once-only
   cleanup and strict absence/deadline evidence, stopping on first failure. This
   is not a repeat of the before-G suite and does not prove compiler no-escape.
   Fixture code, exact helper disposition/context, caps and allocation are OPEN;
   this proposal neither implements nor launches those controls.

5. **Then consider the actual four commands.** After the source/timeout gates,
   ROOT can separately allocate exact9789. Existing -v must produce a bounded
   effective-job witness: the expected cc1 path/mode and pinned absolute linker
   with all three libraries, rejecting unknown jobs/configurations. The current
   linker-only witness is insufficient to claim complete job closure. Preserve
   fresh MD intake, mandatory/allowed roots, source/tool/library pre/post pins,
   generated output identity and E/B semantic results. Accept each expected
   status only after its strict group cleanup. Output bytes, leader exit and a
   successful link never substitute for timeout/escaped-descendant evidence.

## Budget and acceptance boundary

The old refused240s proposal used210s work plus18s terminal reserve. Its verified
before-G failure path may need12s direct wait plus6s existing helper cleanup:
246s in that worst case. A future fixed240s plan must cut the sampled work
cutoff to at most204s if it retains those reserves. Previously completed cleanup
uses the same work clock before a next launch. Start at CLI entry, count source/
helper/operator work, check immediately before Popen and after final output, and
retain the existing sampled-scan/syscall-stall limitation. This arithmetic is a
source finding only; no runtime limit or production code was changed.

Accepted component47939abe/producer57c535b4 establishes only two before-G controls.
The independent review joined exact source plus retained outcomes/events; it did
not add an independent syscall timestamp/errno journal or a new lock probe. It
qualifies neither OS-signal interruption nor post-G clang/ld64/fixture descendants.
The actual full-four operator, archive-member join, post-G helper disposition,
timeout integration, runtime job ledger and semantic outputs remain OPEN. Group
absence alone cannot assert that no process escaped that group. No generic or
full827 qualification is requested or implied.

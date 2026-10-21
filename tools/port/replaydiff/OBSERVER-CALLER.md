# Exact standalone observer caller — source-only review

observer_standalone_caller.py implements only the fixed r4ddac833b fourteen-array
plan with launch contextf9518da3 and candidate input inventory397305e1. It is not
a generic executor. The71679 helper actual cwd/env guard failed before identity;
no current actual-context qualification exists. QUALIFIED_CONTEXT_PIN isNone and
all execution requests reject BEFORE any file/lock/child operation. Seal-only mode
writes execution_enabled=false. There is no fallback/new497c helper, automatic
qualification receipt or user-supplied waiver. No native execution is allocated.

The disabled execution path is reviewable: nice19, nonblocking persistent M1
native.lock, fresh private root,50GiB floor, fixed compile/link/control/otool output
paths, STATE, exact leaves and one sequential supervisor. Total clock begins at
caller entry;110s work includes input/tool verification,10s reserves inherited
cleanup/report, actual<=120s required. The output guard is sampled256MiB growth;
reports are capped2MiB and written without overwriting evidence. A blocking syscall
cannot be certified hard-real-time by source review. Lifecycle/limit controls UNRUN.

Before EVERY launch, explicit revalidation covers pinned metadata, eleven source
files, actual eight compiler/linker/runtime/config/SDK context records, Python,
gate and four mechanism leaves, chosen target resolution/hash and exact env. This
is the caller boundary that run_next does not inherit from base.launch_context.
No method assumes that base generic validation happened. All candidate input bytes
are additionally rehashed inside the same work budget before/after the command
sequence. Current tool/header drift fails without executing a substitute.

Each compile uses raw-MD (including system headers), with no dependency rewriting.
Every actual dependency must resolve to a pre-inventory record and match its SHA;
missing/changed SDK/libc++/resource/header files reject. Each pinned otool's retained
output must have the exact owned binary header and valid library rows, without
dynamic C++/unwind runtime. These checks validate only actual standalone fixtures,
not native app provenance, runtime sampling safety, GPU/color or visual admission.

Sealing requires actual M1 metadata paths and the four source leaves, but invokes
no tool or child. Example DATA-ONLY operation:

```
python3 observer_standalone_caller.py --plan PRIVATE/command-plan-r4.json \
  --launch PRIVATE/launch-context-r1.json --inputs PRIVATE/candidate-input-pins-r4.json \
  --leaves PRIVATE/supervision-leaves-v7-v9 --contract PRIVATE/fresh-source-contract.json
```

The caller's actual root/L review and a qualified exact child/tool context must
precede any future enabled contract and explicit M1 allocation. Unknown ownership,
EPERM or cleanup uncertainty remains failure with retained records; never claim
absence or successful release from leader exit alone. No fullproof caller,
monitor, transform_dep, package or publisher is used. Original31a/046/6c52, old94
runner and failed71679 diagnostic remain unchanged.

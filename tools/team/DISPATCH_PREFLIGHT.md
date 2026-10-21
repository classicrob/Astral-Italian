# Canonical three-TU dispatch preflight

SOURCE-ONLY PREPARED; execution and independent review pending. This is not a
certificate, full-tree census, native/runtime gate, or landing command.
Worker FIFO captures/logs are outside this task and must remain unread on hold.

Pinned source: 33e71ee880b426c0c7da03a724377200e874c417 ->
22d4c005612f83dc70a2a187c2b1a426ada66296. Canonical spec SHA256
3b0975d48ca4354862814f13de004121fb16fbc1b90f87fb8e9f78a65155f8d9.
Decoder: 39bf295bc07c74e4407fa44322086cc31035db8b. The prepared manifest
pins the runner, source-only graph generator, macro source, exact compiler
executables AND compiler support files, wrappers/assembler/tools and three
reviewed decoder modules. These are hash-checked again before execution and after
qualification. No old object or dependency file is copied.

Prepared M1 directory: ~/MacioGalaxy2-team/scratch-port/solP/dispatch-preflight-r3
Plan canonical-JSON SHA256:
022728212c821f48faa3778bba8de5e13dd2c63bd2e0c9f6de5a2f4f970e39b1

After independent review and PORT allocation only:

```sh
nice -n 19 python3 tools/team/dispatch_preflight.py \
  --repo /Users/roberthaisfield/MacioGalaxy2-team/scratch-port/solP/proof-wm-dispatch-source \
  --spec /Users/roberthaisfield/MacioGalaxy2-team/scratch-port/solP/dispatch-proof-design/canonical-33e71-22d4-spec.json \
  --output /Users/roberthaisfield/MacioGalaxy2-team/scratch-port/solP/dispatch-preflight-r3 \
  --dol /Users/roberthaisfield/MacioGalaxy2-gamedata/SB4E01-extracted/sys/main.dol \
  --execute-plan-sha256 022728212c821f48faa3778bba8de5e13dd2c63bd2e0c9f6de5a2f4f970e39b1
```

The script itself acquires native.lock nonblocking, requires nice19 and 50GiB
free. Six sequential fresh object compiles only, j1. Expected 30–90s; hard
global execution budget 420s, including in-process postcompile controls.
TERM/INT/HUP unwind through the owned-group cleanup; spawn+registration masks those
signals and ALRM until the new PGID is registered. Every execution child (including
git/Ninja metadata commands) is in a recorded new process group. Leader exit alone
never proves worker exit. Cleanup signals the whole group TERM, then KILL if needed,
reaps the leader and confirms killpg(PGID,0) reports absence; otherwise it fails
closed and records the remaining PGID. Up to four seconds of cleanup-only grace
may follow the work deadline; no compilation/controls continue in that grace. No script may
be run under a second wrapper owning the same POSIX lock. STATE is written before
clones/phase compiles. Interruptions preserve all outputs and require a NEW
prepare/output path; never reuse partial execution. No compiler is invoked by
prepare mode or the Python model checks.

## Exact acceptance and rejecting controls

- Two independent clones of the bound source; official source-only graph generator
  never invokes split/link/landing. Only WorldMapResultTU13, WorldMapResultTU14,
  GrandGalaxyMapSequence compile targets; require exactly three objects per phase.
- Actual graph, resolved Ninja command list, dependency list and each dependency
  content SHA256 retained for each phase. Source/config files stay at exact commits.
- TU13: ONLY Start .text#0 0x370/8 ->0x390/8, End 0x390/8 ->0x370/8.
- TU14: ONLY Wait .text#0 0x240/12 ->0x230/8, ZoomOut 0x230/8 ->0x240/12.
  For all four records require info18/other0, exact section occurrence. Normalize
  only these reviewed symbol fields in memory, then compare complete occurrence-
  keyed summaries. No raw section, relocation, pool, other symbol or body waiver.
- Actual pinned private DOL ownership decoder runs separately for both scopes.
  Known-bad baseline must expose exactly two reversed owned chains each; candidate
  must satisfy full original singleton/ctor/vtable/wrapper/selector ownership.
- Typed GrandGalaxyMapSequence is strictly occurrence-keyed Wii equivalent.
- Real ELF mutation controls per TU: first text byte; each approved symbol's value
  and size separately; both owning-slot nonzero relocation addends; both crossed
  relocation targets. All nine mutations must reject semantic envelope; crossed
  targets must additionally reject original-ownership decoder.
- Eight separate config controls: wrong address and wrong size for each of four
  mapped execute records. Unapproved config remainder cannot change.

The Python tests are preparation evidence only: five test methods cover all
four permitted records, remainder and symbol-field rejection plus map identity,
wrong address/size, duplicate and config remainder rejection. They do NOT discharge
real ELF controls. Additional lifecycle tests model an exited leader with a surviving
worker (TERM then KILL and whole-group absence), deliver real TERM/INT/HUP to the
Python test process to verify unwind, and exercise the actual global ALRM during
in-process work. The surviving-worker case is mocked: it is not actual job cleanup
evidence. Actual compile and all native/ELF controls are UNRUN at sealing.

## Output and cleanup

execution/{before,after} are private fresh source/build roots; result.json reports
COMPONENT qualification only. Retain graph/command/dependency manifests, all six
ELFs, controls, STATE and failures until L/PORT review. No private DOL/reference
ELF/instruction dumps are transferred. DOL is read only by the decoder during the
explicitly allocated execution. Clean only these own fresh clone/build trees after
retention approval, preserving compact evidence; never remove prior53 recovery or
dispatch baseline inputs.

The full 2426 fresh pair, all-other2424 reference/effective-alias closure,
Wii-active source perturb/restore controls, operator-reviewed independent TU13/TU14
external plans and secondary TU14 control transport remain REQUIRED separately.
Current remoteproof must not be invoked as if it accepted intentional deltas.

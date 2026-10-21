# Focused native header controls

These controls do not modify the reviewed worker-r3 header/CPP and do not replay a
parser or game. Native compilation/execution belongs to allocated T2, not P.

Fixture SHA cc69816a067837fdae4c1179e1a43ceae64c3357290201c6dceb0c2024c40887.
Runner SHA 52d942bca887adace84c7e8819ad127ceaa0d23c8b34ca55b85ab53de6d96b25.
Exact generated header SHA338e1492fd12f78374559acfe4eb0237079de95527430137cc729028b2454c4e.
Independent E source-review receipt SHA71b1ec9ed6f2854671b6bc92e7acc8ac3dff4a7b679ec8af5dd9f75ecd3a97e7.

The synthetic fixture includes that unchanged header. Test-only write redirection
forces EINTR or counts calls; production compilation has no such redirection.
It validates <=15 file attempts plus at most1 error write per flush, latches,
fixed rings/caps and RAII clearing. Guarded two-page host-only memory tests zero/
one-byte spans, last-byte/end/outside positions, invalid CP ends, invalid fmt and
early return. These are observer-header bounds tests, NOT actual decoder truncation.

The runner requires an absent private app output, then preserves synthetic capture
files by rename. It exercises OFF/no-file, guarded ranges/no-flush, ON/two snapshots,
forced I/O LIMIT, existing-file sentinel refusal and non700 parent rejection with
finally restoration. No gameplay/capture file is deleted or overwritten. At success
firstfault.log must be absent for the separately authorized app run. Any failure
stops; inspect preserved files/logs before retrying with fresh paths.

Exact compile/run argv and source hashes are in immutable producer handoff
scratch-port/solP/fifo-worker-generated/worker-r3/native-control-plan.json. The
compile derives actual T2 staticLLVM20 flags and libc++/abi/unwind archives from
worldmap51/actual-link-context.json SHA829db9316284652a9ab1ed393b57c4937eb297b77fe1117a2e26bf8b2d9ab89c.
Do not replace it with Apple ld or guessed dynamic C++ libraries. The plan is data;
P did not execute it. Compile in fresh T2 output under its allocated lock/nice19.

T2 reported native compile/control PASS; actual receipt is
~/toolchains/sol-t-validation/worldmap51/fifo-worker-r3-control/native-controls.json.
App default-OFF smoke and matched host replay are separate gates/results. No game
or producer/list ownership claim follows from synthetic control success.

# One Yoshi diagnostic window reference

This is a new source proposal, not runtime allocation or matched pixel qualification.
The fixed case is YosshiHomeGalaxy scenario 1, neutral input for 616 intervals after
an independently observed first Game/init6 stage seam. No forced RNG. The accepted
r4 entry selector verifies the immutable guest stage string and the original three
stage-request calls. Keep the default renderer, CPU, DSP, timing and profile.

`RECIPE.disabled.json` pins exact existing source bytes, current M1 host-query and
screencapture tools, historical emulator/disc identity and the private outputs.
It is deliberately disabled, with the one generated-input hash unresolved. The
M1 has the emulator/profile/disc but does not have the r4 source snapshot or boot
script at those exact paths. Source materialization is a prerequisite, not a
permission to transfer game assets or launch during another owner's slot.

## Exact sequence proposed for allocation review

1. Pin a fresh private work root, operator plan SHA, wrapper/generator source SHAs,
   actual interpreter/emulator/dependencies, and unchanged profile. Materialize
   the listed r4 source files and existing boot script, preserving original files.
   Copy the private profile; the only semantic change is General.GDBSocket, using
   the existing yoshi_replay.py:125-139 recipe. No saved-state restore.
2. Input preparation, in the fresh private root, with the pinned interpreter:

   `python prepare_idle.py --contract <snapshot>/native/tools/headless.py --contract-sha256 <pinned-contract-sha> --output <root>/idle616.jsonl`

   The exact generator calls the existing contract `generate(616,617)`: no jump
   edge occurs. It emits 2464 explicit four-channel read records. Freeze its actual
   SHA into a NEW operator plan; never enable the placeholder plan. The suffix
   records synthetic neutral source connections, while `--sync-observed-connection`
   exports actual read-only WPADProbe results. Preserve the original full boot
   export/read ordinals and the suffix source mapping; do not substitute the source
   JSONL as observed delivery.
3. The proposed actual wrapper argv is:

   `Python.app/Contents/MacOS/Python stopped_reference.py --plan <new-plan.json> --plan-sha256 <operator-SHA>`

   The exact collector argv array is in the recipe. This must run through a
   separately reviewed LaunchServices/owned-process caller for that exact Python.app,
   at nice19 with nonblocking native.lock and an overall 480-second ceiling,
   256MiB new-output cap and 50GiB free floor. No such new caller is qualified by
   this packet. Do not simply execute `open` and infer PID/group cleanup.
4. The small wrapper uses the existing StageSync.select result at trace row 616,
   while the accepted CPU PC/LR are 804be280/804b6ac4, before the collector's next
   resume. It requires its still-unreaped registered Dolphin child and actual
   proc_pidpath match; the existing T2 host query returns window metadata for this
   PID. Require screen-capture permission and exactly one layer-zero owned window;
   fail on ambiguity, never choose a guessed first window or activate/resize it.
   The ordinary command is `/usr/sbin/screencapture -x -l <actual-window-id>
   <capture>/host-stopped-ready-plus616.png`. Timeout 10s, direct child reap 3s.
   No emulator screenshot hotkey, guest write, graphics setting or Game hook.
5. Preserve the original collector/export/finalization path. It still completes
   its input accounting and final seam instruction after the screenshot. The
   wrapper does not claim no emulation advance during later finalization. Require
   finalized main/export/entry receipts, actual frame/read counts and natural
   ready seed; the source probe connection mismatch is an input-delivery finding,
   not a gameplay divergence. Owned-process absence/lock release is separate.
6. In a separately allocated offline read, decode/hash the single original PNG,
   join its host timestamp/PID/window bounds/CPU source row, and inspect its pixels.
   Preserve failure output without retry or arbitrary window selection.

## Evidence and limits

A stopped guest CPU seam does not freeze host presentation/GPU completion or bind
which VI/draw is visible. The screenshot includes window chrome/shadow, native
resolution/PAR/color encoding is unknown, and cannot enter the phase-v2 comparator
as an eligible content pair. Stage init6 is the collector's arrival predicate, not
proof that the sampled Game is the selected drawn scene. No fixed lag correction,
fit, crop-to-match, later-frame substitution or state forcing is permitted.

The native exact2c76 Yoshi fresh/warm input-script480 images now have actual
rendering witnesses, but their input, phase and source differ from this proposed
retail screenshot. They are descriptive references only, not a matched pair.
The native no-XFB sampler remains E NOPAIR; this proposal does not install it.
Qualification remains BLOCKED with null qualified frames even if the image looks
plausible. Requested next action: independent source/context/lifecycle review of
this one-case recipe, then an explicit host handoff/allocation. No new run yet.

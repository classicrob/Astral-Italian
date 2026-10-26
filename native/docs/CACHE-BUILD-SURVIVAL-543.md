# 5.4.3 cache build-survival runtime proof (M4 only)

PORT's first proof refused at the quiet precondition (no app launched). The
second stopped at A-cold-title because the normal power exit bypassed Aurora's
artifact report/close. The third verified that report and successful close on
the real app, then stopped on an incorrect runner timing predicate: it treated
the title observer's minimum as an exact total duration. All original outputs
remain preserved as failures. The tools-only timing correction uses the same
three r3 builds below and a new output directory; no rebuild is needed. It makes
no runtime survival claim. Round4 passed A-cold-title and stopped before the next
launch on its one-shot quiet check: the preceding cold compile work had raised
load averages. The runner now waits boundedly for the same quiet predicate;
neither a failed arm nor an old receipt is retried or overwritten.
PORT chooses the quiet build/proof window;
SOL A retains the M1 measurement slot. All results, failures and caches remain.
No rerun loop, cache clearing, host input, AX driver or TCC request is used.

## Source/build bindings

- A: `a8916f091fe1ae9463e58991fbed23bc63146518`, on 7d6a71d6's line, includes
  the real-exit artifact finish. Older apps without it are refused. There are
  no compatibility input changes; the expected recipe key remains `b36899bd...`.
- B: `port/543-cache-proof-b-exit` =
  `b6469004a6be12befec78c1ded7e407996365390`, directly on a8916f09. It changes
  only the error diagnostic string in the retained GXGetYScaleFactor path in
  `native/gx/api_compat.cpp`. Normal rendering/shader/descriptor behavior is
  unchanged. The runner requires a different executable SHA and the B diagnostic
  string in B's binary, absent from A's. This branch is a throwaway instrument.
- Negative: `port/543-cache-proof-negative-exit` =
  `7366f125b8549c3a57baa917b1a99774f19fddcc`, directly on a8916f09. Its only
  change is `descriptor_abi: 1 -> 2` in the compatibility manifest. This must
  select a new recipe/artifact namespace. It is also a throwaway instrument.

Build B and negative with the same portable release recipe/dependencies as A.
Each build directory must contain immutable `Petari.app` and `BUILD.json` with
`source` (or `source_commit`), `binary_sha256` and `renderer_key`. Original apps
are read-only inputs. Run the cache unit suites as usual. Do not land either
throwaway change in the freeze candidate.

## Exact server command

From the checkout containing this runner, in the existing detached **bash** /
quiet window (the runner rejects inherited nice5):

```sh
bash tools/port/server_cache_survival543.sh \
  /absolute/A_BUILD_DIR /absolute/B_BUILD_DIR /absolute/NEGATIVE_BUILD_DIR \
  "$HOME/MacioGalaxy2-gamedata/port53/m1g/cache-survival-543-r5" \
  "$HOME/MacioGalaxy2-gamedata/SB4E01-extracted" \
  "$HOME/MacioGalaxy2-gamedata/fixtures/unlocked-242/fixture"
```

The new output path must be below `port53/m1g` and must not exist. The wrapper
uses server-heavy -> native per preparation/arm, not an outer cohort lock. Each
arm requires nice0, load1/load5 <=8, no compiler, and MTLCompilerService CPU delta
<0.05s over10s. Before launching it polls every10s for up to600s, under the same
heavy->native locks. Both consecutive load samples must pass, every resident
Metal service must have a nonnegative CPU delta strictly below0.05s, and the
launch-side snapshot must have no compiler. A newly seen Metal PID needs a
second sample. `quiet.json` is updated after every snapshot with all raw process
lists/load samples, pair checks, CPU deltas, elapsed time and ready/timeout/error
status. The PLAN binds the wait policy. Timeout fails before Popen; this wait
does not retry any launched arm or weaken a readiness threshold.
It never kills foreign processes. An arm failure stops the chain;
a timeout stops/reaps only its owned Popen PID and is a failure.

The runner makes signed perf copies sharing one fresh random bundle ID. It
aliases only that never-used Darwin Metal namespace to `OUT/cache/metal`; only
that exact alias is unlinked on exit. All cache data remains inside OUT. The
application gets explicit private `PETARI_CACHE_DIR`, HOME, CFFIXED_USER_HOME,
TMPDIR and per-arm --user directories. No normal user cache is enumerated or
opened; migration can see only this new selected private root. The unlocked
fixture is hashed before/after and copied privately without cache data. v14 PFD
bytes must match across all three apps. No recording occurs (`ASTRAL_TAPE=off`).

## Frozen order and predicates

1. A cold title; A cold stage.
2. A warm stage; A warm title, using that exact cache/Metal namespace.
3. B on A cache: stage then title, same root/context/namespace and fixture.
4. Negative: stage then title, still the same private root, with a different ABI
   namespace and old caches preserved.

Every arm requires exactly one flushed final artifact counter report and one
successful artifact close report (`persistence=0 checkpoint=0 close=0`). These
come from the real power/quit exit before `_Exit`, or full Aurora teardown,
through the same mutex-serialized terminal operation. Duplicate finalization is
inert, and late Dawn callbacks cannot reopen the artifact DB or change counters.
A close/checkpoint failure fails the arm. The recipe writer's separate lifecycle
is unchanged; this proof does not claim full renderer shutdown occurs at power
exit. The older r1/r2 receipts and original throwaway branches stay preserved.

Title arms explicitly set `PETARI_TITLE_OBSERVE=1` and
`PETARI_TITLE_OBSERVE_MIN_SECONDS=120`. The latter is a **minimum**, not an exact
duration. `frame_seam.cpp` logs `elapsed_s` from its first graphics-observation
composition seam (the `initial` event at zero), not Popen, title-first-draw, or
the end of frontend preparation. The unchanged native `TitleObservation` waits
for both that minimum and 30 seconds after actual global 100%, then requests quit
on the subsequent seam. Its own default minimum is 315 seconds if the env is
absent; the runner supplies 120 explicitly in every title launch receipt.

The scorer requires one `initial`, one valid `actual-global-100`, and one valid
`title-observation-complete`, in order with the same version/clock/source. Both
completion markers must report polled/valid/complete and done>=total>0, matching
the native observer's predicate. Completion
must be at `max(120, global_100.elapsed_s + 30)` through one second later (the
existing final-seam allowance). Raw markers, first-eligible time, post-100
duration and final-seam delay are retained under `evidence.title_observation`.
Missing/duplicate/invalid events, early/late completion or native timeout fail.
For example, global 100% at157.48s correctly completes near187.5s. This changes
the runner's interpretation, not native behavior or cache/render predicates.

Stage is scenario1
YosshiHomeGalaxy, 60 stable ready frames then **1800 neutral script frames**,
with the existing script's in-app quit. All runs have a240s hard timeout. There
is no control-bearing script input and the route must show zero move/button /
camera controls throughout. Quiet/focus conditions and dummy audio are explicit;
this is not an audio or frame-performance acceptance gate.

B must have accepted recipe rows, zero rejected rows, full-key Dawn hits, the
same recipe/backend/GPU-driver-OS/artifact identities and the exact same private
root/namespace. The already warmed artifact rows must be unchanged. Because the
current shipped counters are aggregate, the scorer requires **zero aggregate
Dawn stores and zero misses** for B: stronger than zero new stores for just the
warmed set. A failure is retained and diagnosed; unrelated/background requests
are not silently subtracted. The negative must have a new namespace absent
before its first stage arm, misses and stores >0, and preserve old artifact rows.

XFB captures are decoded and compared pixel-for-pixel (no mean-error tolerance):
A-warm vs B title at boot-frame1200, and stage input-script frames120/240. PNG
CRC/length/format, non-degenerate pixels and dimensions are checked; missing or
unaligned captures fail instead of being searched for a match. File and pixel
SHA256 are retained. Rendering equality is relative to the same-code A control,
not a new retail-visual certification.

The prep result must remain ready, with no pending/failed/rejected keys. Its
elapsed time, splash elapsed time and presented-frame count are **observations**
for A-cold / A-warm / B, not new performance thresholds. Recreating process/device
pipeline objects from cached artifacts still presents at least3 splash frames.
There is no literal no-splash predicate. The release wording is “later updates
start faster,” not “skip preparation.”

`PLAN.json` binds sources, original/perf executable hashes, BUILD receipts,
fixture inventory, v14 PFD hashes, root/namespace, constants and script hash.
Every `launch.json` binds that plan. Each arm keeps native logs, frames/route,
images, process/quiet data, logical recipe/artifact snapshots (including full
binary Dawn keys), Metal inventory and RESULT. DRIVER/STOPPED and RELEASE close
the cohort honestly. Native runtime results are not claimed by the scorer tests.

Source-only scorer tests:

```sh
python3 -B native/tests/cache_build_survival543_tests.py
```

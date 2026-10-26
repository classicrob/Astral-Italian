# AQ / SDL transport diagnostic implementation

PORT authorized source only after the 88b587af design. This is an instrument,
not an underrun fix or acceptance binary. There have been no new audio/app
runs. The original stopped 2293cdfe history fault and all three positive
underrun windows remain unchanged. Server build/review precedes any new runs.

## Exact scope and activation

`native/gx/sdl-patches/0003-audio-boundary-diagnostic.patch` is the full diff
against pinned SDL release-3.4.10. It changes three audio TUs and adds two small
headers. Existing ordered SDL patches and pin/archive hash remain unchanged.
The retained upstream audio files were independently hash-bound during the
88b review. `native_audio_transport_patch` checks **all operation tokens** of
those three TUs after erasing only this named instrumentation, the explicit
count-only block and the equivalent put-return temporary. It also checks the
null observer / format refusal / existing SPSC path. This is not a substitute
for Release disassembly.

Only `PETARI_AUDIO_WORKGROUP=async` + `PETARI_AUDIO_WORKGROUP_DIAG=1` installs
the observer, before SDL opens the stream. With it disabled, each SDL boundary
is an inline atomic null-pointer load/branch, without clock queries, trace TLS,
allocation, locking or I/O. Petari fill uses one cached observing flag and an
outlined observer. Every native hook is diagnostic only; the original device,
stream locks, resampler, callback order, ring and thread priorities are retained.
The custom static-SDL C extension is local, outside upstream dynapi, and is
configured only before opening devices, never during a live callback.

`PETARI_AUDIO_AQ_BUFFERS_DIAG=6|18` is explicit and default-off; unset preserves
SDL's original buffer-count calculation. Any other value refuses. An override
requires the history observer, AUDIO_DIAG, unchanged 48 ms ring setting,
32000 Hz guest source and 256-frame device setting. The SDL backend additionally
refuses an actual output format other than 256 frames at 44100 Hz. It changes
**only the AQ buffer count**. Registration wait calculation, producer cadence,
SPSC target and request size are unchanged. The actual device format and
packed AQ count/frames/rate are logged, not inferred from the request.

Six buffers contain 34.830 ms at 256/44100; eighteen contain 104.490 ms:
**+69.660 ms potential queued latency**. This arm is a discriminator, not a
shippable remedy. No device format is forced to make the arm pass.

## Boundaries and identity

Every retained record has the same-history `mach` and `steady_ns`, pthread,
global sequence, AQ callback `ticket`, object pointer and numeric value.
Mach time remains the workgroup ledger/deadline clock. `steady_ns` uses
`std::chrono::steady_clock`, the same epoch/unit as FrameTelemetry/GPU trace;
there is no wall-clock or guessed frame join. Tickets are allocated at AQ entry,
retained through nested calls, and cleared at AQ exit. Object identifies device
or stream; they are never assumed equal. Gap history is per callback pthread
and device; device changes do not inherit another device's previous timestamp.

| Kind | Boundary / value |
|---|---|
| 1–16 | Existing guest/DSP/baton/fill-gap/registration ledger, unchanged meaning |
| 17 / 18 | AQ callback entry (AQ buffer pointer) / exit (iterate success) |
| 19 / 20 / 21 | Device lock request / acquired / released |
| 22 / 23 / 24 | Stream get begin / stream-lock request / acquired (requested bytes) |
| 25 / 26 | Petari fill entry (additional bytes) / exit, including early error return |
| 27 / 28 | Post-fill get/conversion loop begin / end (wanted / delivered bytes) |
| 29 | Get end, after stream unlock (result bytes, signed -1 on failure) |
| 30 / 31 / 32 / 33 / 34 | Actual SDL put begin / lock request / acquired / released / end |
| 35 | AQ format: count in bits 48–63, frames in 32–47, rate in 0–31 |
| 36 | Signed32 pair: self QoS / pthread requested QoS |
| 37 | Signed32 pair: QoS query error / relative priority |
| 38 | Signed32 pair: pthread scheduling policy / sched_priority |
| 39 | Signed32 pair: Mach THREAD_EXTENDED_INFO pth_curpri / pth_priority |
| 40 | Signed32 pair: CPU query error / calling CPU number |
| 41 | Signed32 pair: scheduling query error / Mach query error |
| 42 | Signed32 pair: Mach pth_policy / pth_run_state |

Entry readbacks are taken **before fill's existing first-call RT promotion**,
at each AQ entry, not borrowed from a game thread or earlier cohort. Unknown
fields are -1 with the actual error. QoS requested/readback is not proof of an
effective override; CPU is an instantaneous self sample, not residency. No
P/E cluster order is inferred. The sampler's own elapsed time is visible
between entry and its metadata/next boundary and is not subtracted.

The producer already fills a lock-free SPSC ring through AI::pull. It never
puts to SDL. The traced put runs inside fill while the enclosing stream lock
is held; SDL mutexes are recursive. No extra lock is inserted. Error returns
retain matched get/put outcome boundaries and lock-release boundaries.

AQ entry interarrival includes the previous callback's work and later
run-loop/device/AQ availability. Entry minus prior AQ exit removes observed
prior callback work, but still does **not** isolate OS scheduling. Device-lock
and stream-lock request→acquired spans localize contention; conversion has its
own begin/end; fill contains SPSC read and callback-side put. These spans nest:
**do not sum them**. `audio_transport_report.py` reports their separate spans,
entry/exit completeness, exact overlap deduplication and missing fields as
null. An incomplete edge window is never described as a completed callback.

## History repair and publication limits

Each of the 32768 cells publishes a complete current and previous generation
under a single-attempt writer flag. It publishes the old complete payload to
previous **before** modifying current. Reader attempts are bounded. All payload
words are atomic; no producer allocation, wait, formatting or I/O is added.
A post-trigger wrap therefore retains the actual old payload, including time,
kind, thread and value. This fixes the old false count of 252 overwritten,
out-of-window prefix cells without discarding an in-window error.

Filtering happens only after a stable exact-sequence read. If neither retained
generation supplies the expected payload, membership is **unknown**, not
outside-window. `retention_unknown` refuses the dump/controller/validator just
as drops, unstable or clipped do. Up to 128 rejected cell identities are
emitted with expected/current/previous sequence, versions and stable flags;
kind/thread remain unknown if the original payload is unavailable. Additional
rejections have an explicit `rejected_omitted` count and still refuse. The
`unstable` field is retained for schema compatibility; uncertain membership
is reported in `retention_unknown`, never counted as known in-window data.

A dump contains the 2 s before its trigger and **already available** events up
to the earlier of reporter snapshot time or trigger+500 ms. It does not wait
for post-roll on any thread. This captures the work after a delayed AQ entry
instead of freezing permanently before its first device-lock attempt. Both
actual `end_mach` and the 500 ms cap are explicit; validators enforce them.
No future or unavailable callback completion is invented.

At most 24576 event rows plus 128 rejection rows are formatted per reporter
iteration (regular iterations are at least one second apart; shutdown can
emit one additional bounded dump). This leaves room over the expected
~6.4k recorded boundaries/s without reviving the mutex firehose. At typical
row lengths the regular-iteration cap is below 10 MB/s and 30k streamed rows/s. Actual rate,
zero loss/clipping, writer time and overhead still require measurement. In
memory storage is fixed, and only pre-touched when observing. A reporter
stall beyond retention can refuse honestly; it cannot silently qualify.

## Verification and next review

Local receipt root: `port53/m1g/build-reports/sola-aq-localize-r1/`.
All native work takes heavy→native at nice 0. No app/GX build, device-open,
playback, cache mutation or acceptance run is part of these checks.

- Actual optimized object compilation: workgroup, sink, new SDL CPU test and
  all three modified SDL C/Objective-C TUs with the macOS SDK.
- Optimized and UBSan/unsigned-overflow history fixtures: post-trigger wrap,
  actual payload recovery, genuine in-window loss refused with identities,
  concurrent writer/reader snapshots, clipping and bounded available post-roll.
- Pure Python source/token and schema-2 intake/report tests. Legacy schema-1
  failures remain failures; no old log is rewritten to schema 2.
- Server pending: full app/test build; the new `native_audio_transport_sdl`
  CPU-only stream test (no device) proves disabled/on converted bytes identical,
  recursive get→put ordering, error unlocks and explicit configuration refusal.
  Existing workgroup/audio/GPU-trace/tape suites remain required.
- After build review: production Release codegen and retained off-probe checks,
  including patched SDL callback/device/get/put and native fill; then this
  instrument's **own** repeated alternating overhead qualification, STOP.
- Only after review: the predeclared 6/18/6/18 long discriminator, same fixed
  256/44100 device, 48 ms SPSC base, guest rate, request and timeout settings.
  Real CoreAudio >=50 reports / zero underruns, all positives retained,
  callback/lock/conversion contexts and the >=175 ms baton discriminator,
  per-5 s coreaudiod CPU, no reruns until clean, STOP for PORT review.

AUHAL / HAL IOProc remains a structural alternative in DEVICE-AUDIO-DESIGN-543;
no sink replacement, buffering fix, scheduler fix or K/R promotion is made here.

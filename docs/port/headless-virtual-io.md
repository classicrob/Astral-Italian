# Virtual I/O and reload repeatability

The headless virtual clock must not race host storage latency. Previously the
DVD/NAND/GP interrupt threads published completion whenever a host read finished, while main
advanced frames independently of a lower-priority scene loader. An initial ready
anchor discards variable boot rows but does not solve a subsequent scene reload.

In virtual-clock mode, DVD service bursts, NAND jobs and GP/progress callbacks
receive a common ticket at submission, including during bootstrap. Dispatch
initially retains legacy host-timed behavior: the initial ready scene and its
gameplay are unchanged. At the first post-ready reload request (`init_state=0`),
the headless frame driver releases the emulated CPU, drains all registered
in-flight legacy jobs and runnable OS work, then atomically enables ordered
delivery before the next scene-controller update can start async destroy/init.
With stage sync, only the selected target-ready anchor arms this transition;
earlier file-select scenes do not. Without stage sync, the first Game/init6
scene arms it. Once activated, ordering persists for all subsequent scenes.

Ordered native I/O workers wait for the front ticket and an idle emulated CPU
before doing work. DVD preserves its priority queues within a burst.
After a callback readies game threads, the next DVD command waits for those
threads to sleep or finish. The ticket stays pending across host I/O, so host
latency consumes wall time without advancing virtual ticks.

After activation, virtual GP callbacks cannot arrive early while main owns the
CPU. Otherwise a
frame-end draw-done wait may be skipped in one run but yield to a lower-priority
scene destructor in another, moving its observed null scene by one frame. GP
events and FIFO progress wakes share the ordered queue; real-time token/capture
bypass rules remain unchanged.

The headless driver fences runnable OS threads and pending tickets before
advancing devices, after each device quantum, and after VI delivery. Its main
thread remains host-blocked throughout the fence. Loader poll preemption points
sleep on one OS queue while I/O is pending. Completion wakes them together in
OS priority order; the frame fence waits for them to settle. Synchronous SDK reads
already sleep their OS thread, so CPU-idle service can finish a read without
requiring another VI. All these fences are inactive before reload activation.

The scheduling boundary is an explicit SDK sleep, loader poll or driver/device
boundary. It is a native replay policy, not a model of measured Wii disc latency
or arbitrary-instruction interrupt timing. Post-ready reload loading-frame
counts deliberately become deterministic;
they are not Dolphin-faithful. DVD latency is not emulated. Bootstrap and
initial ready source origins retain legacy behavior. No reload rows are
removed, input labels shifted, or RNG seeds forced. Real-time/GX mode uses no-op
tickets and retains its existing asynchronous workers.

`native_virtual_async` tests reversed worker readiness and varied host delay,
callback order, loader execution before the following completion, unchanged
virtual ticks, legacy out-of-order bootstrap dispatch, in-flight activation fencing,
target-ready arming, progress from an explicit polling point, and GP delivery
after a lower-priority scene worker despite an earlier host delivery opportunity.

The private-data acceptance runner is intentionally separate from CTest:

```sh
lockf -k "$HOME/MacioGalaxy2-team/slots/native.lock" nice -n 19 \
  python3 native/tests/headless_reload_repeat.py \
  --executable /private/path/petari_headless \
  --disc /private/path/extracted-disc --inputs /private/path/inputs.kpd \
  --out /private/path/fresh-repeat-output --stage YosshiHomeGalaxy --frames 30000
```

It runs the unchanged tape twice into fresh isolated users, checks finalized
30001-row dense traces, retains natural ready origins/seeds, and compares whole
trace SHA256 digests without excluding fields. It also records timing-sidecar
identity and reports the first unequal field. Each run has a 540-second bound.
The caller owns the host lock. An explicit `--asan-options` records any chosen
sanitizer policy; repeatability alone is not a sanitizer or gameplay-equivalence
qualification.

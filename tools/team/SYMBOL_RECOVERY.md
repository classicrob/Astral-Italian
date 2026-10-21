# Future freeze prevention

Every new frozen executable must satisfy [the debug-symbol freeze gate](FROZEN_SYMBOLS.md) before promotion or object pruning: adjacent UUID-qualified dSYM, representative file:line lookup, hash-bound source/toolchain receipts and an independently verified private M1 archive. This does not assert historical dSYM availability.

# Exact frozen-symbol qualification

`symbol_recovery.py` is read-only tooling. It extracts historical configure argv without executing old shell scripts, binds original compile/link argv, verifies dependency archives and the two pinned SDL literal patches in memory, and emits preservation/restoration plans. It never configures, compiles, checks out, renames, deletes or acquires locks. The CLI refuses existing output; allocation flags do not enable an executor.

The first proposed target is the previous default frozen GX image, then its headless image, then the older frozen GX image. Every target must use its original absolute source/build paths and its own original source, cache and command metadata. A transaction must snapshot the ACTUAL checkout and build images at allocation time, including any newly landed work; never hardcode a historical current HEAD as the state to restore.

`restore_preflight` validates an owned journal, current source identity, original protected file hashes, canonical paths and an unoccupied attempt destination. Its ordered action list preserves the failed attempt and restores the original checkout/build before hash/UUID rechecks. This is not an executable transaction runner. A separately reviewed executor with durable write-ahead checkpoints and restart recovery is still required before changing shared paths. No passing placeholder claims that transaction or exact rebuild has run.

Current tool SHA values are prospective evidence only. Historical version/path/SDK/linker metadata without historical file hashes must remain explicitly UNESTABLISHED. Exact dependency archives plus approved transformations can qualify those source contents independently. An absent historical stage-seed directory is valid only when historical source has no seedpack and the frozen resource closure confirms no per-stage seeds.

The final acceptance requires full frozen executable SHA256 AND Mach-O UUID equality, exact recovered object hashes, matching dSYM UUID and line-resolution qualification. Later object mtimes can change original debug-map entries; identical source and compiler version alone do not establish link reproducibility. Do not patch images, override UUIDs or silently restore timestamps to force equality. Report any mismatch and retain the attempt.

Run the source-only controls:

```
python3 -m unittest discover -s tools/team -p test_retention_paths.py
python3 -m unittest discover -s tools/team -p test_symbol_recovery.py
```

## Durable transaction engine

`recovery_transaction.py` now supplies the preservation/restore engine, qualified only on synthetic paths. It requires an explicit unexpired exclusive lease pinned to the original HEAD, source root and historical target. It journals and fsyncs rename intent before mutation, reconciles interrupted atomic moves by device/inode, restores the actual original checkout and both canonical build directories, and retains failed builds at distinct paths. No destination is overwritten. SIGINT/SIGTERM/SIGHUP unwind through restoration; abrupt death is recovered by reopening the original journal and calling restore. Unexpected dirty/source/directory/file identity requires manual review instead of a destructive guess. Restore remains available after allocation expiry or a disk-floor failure.

The engine exposes no CLI mutation entry point. A caller owns the native lock and bounded configure/build processes. `run_owned` guarantees an unwind for ordinary errors and handled signals; journal recovery handles termination between rename and commit. Build5.1 GX is the first proposed attempt, with maximum26minutes,8GiB additional allocation, total jobs<=6 and50GiB floor. The caller must check budgets while supervising each bounded child, not only after it exits. No real recovery execution has been qualified or performed.

Current synthetic coverage additionally includes a real Git adapter against a disposable fixture repository, SIGTERM delivery, success/error/restart before and after rename/checkout, restart during restoration, occupied destinations, altered protected image/directory, dirty checkout, stale allocation, space-floor/budget, terminal-journal reuse and independent hash/UUID rejection. Temporary fixture repositories/directories are removed by their owning test fixtures.

## Bounded first-target caller

`recovery_caller.py` is the executable caller for ONLY the reviewed build5.1 GX attempt. It requires a separately approved packet-hash-bound allocation and the exact33e71 snapshot, historical82434e85 source, frozen1542894f SHA/244128be UUID, both canonical build directories and all eight protected images. It takes a nonblocking POSIX record lock on the shared native.lock without truncation. It does not accept blanket three-target allocation.

The owned-session supervision follows bounded_run.py's TERM/KILL/wait pattern, adding an exec gate so ownership is journaled before child execution, exact nice19, live disk/deadline polling and whole-group quiescence checks. Configure is bounded180s and build1200s; total allocation<=1560s, additional space<=8GiB, floor50GiB. Before compilation it requires the archived4931 generated files, additional retained C inputs and original compile/link argument tokens to match. No provider/flag, timestamp, UUID or relink workaround is attempted on mismatch.

The private `STOP` file means incoming; SIGINT/SIGTERM/SIGHUP are also handled. Cleanup MUST terminate all owned groups and reap direct children, then confirm group IDs absent BEFORE calling transaction.restore. If quiescence cannot be established, it leaves original directories protected for manual recovery rather than changing source paths under a live writer. The same restore-after-children function is exercised by a disposable process/transaction test. An unrelated process survives; ignored TERM forces KILL. After abrupt daemon death, inventory the durable children ledger and original journal under the native lock; do not invoke engine.restore while any recorded group remains. The caller does not blindly kill stale/reused PIDs after restart.

This caller records exact command vectors, child ledger, pre-build comparisons, candidate image and559 missing-object measurements. Exact image reproduction remains separate from dSYM/line-resolution acceptance. Every failed attempt and original receipt is retained. It performs no promotion or pruning. Synthetic controls create only disposable Python processes, temporary source/build paths and tiny fixture files, and clean them through their owning fixtures.

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

# Checked capture PNG publication leaf

This isolated C++17 helper publishes a caller-supplied RGB8 buffer. It has no
renderer, GX, FIFO, Game or screenshot hooks and changes no existing PNG writer.
It does not establish GPU completion, draw/readiness alignment, viewport or color
encoding. Every visual qualification remains BLOCKED.

`publishRgbPng(path, width, height, rgb, bytes, io)` returns either a completed
`Receipt` or an error with no receipt. Pass an ordinary `Io` in production; virtual
syscall methods exist for failure injection and must follow POSIX conventions and
not throw. Dimensions must be positive PNG dimensions, bytes must match exactly,
and RGB data is capped at 16 MiB. The caller must supply a valid readable buffer
of that length and a destination inside its own private directory.

The encoder checks arithmetic before access/allocation, produces RGB8 PNG with
stored-deflate blocks, CRCs and Adler32, and handles allocation/encoding exceptions.
The file protocol is:

1. Create a unique 0600 temporary file beside the final path with `mkstemp`.
2. Check every write, handling short progress and EINTR, rejecting zero progress.
3. Check `fsync` (retry EINTR), then read back the exact encoded length and content.
4. Check writer close. Never retry close: descriptor state after an error can be
   platform-dependent. A close failure is an unsuccessful capture, including EINTR.
5. Atomically publish with a same-directory hard link. Unlike replacing rename,
   `link` atomically rejects any existing destination, including symlinks and a
   collision created after writing began. Existing files are never overwritten.
6. Open the published inode without following symlinks, verify identity/size/bytes,
   compute SHA256 with system CommonCrypto, and check read-descriptor close.
7. Remove the temporary link, then return path/dimensions/byte count/SHA256.

On failure, remove only paths still matching this call's inode. Cleanup failures
are included in the error; no receipt is returned. The private-directory contract
excludes hostile concurrent writers changing file contents or ancestor paths.
This is atomic visibility with checked file-data flush, not a claim of directory
entry durability across power loss. No color profile or sRGB/gamma claim is added.
The returned receipt is in memory; a future caller may serialize it after success.

## Standalone controls

Use the existing Python with Pillow and the system Apple toolchain, no packages:

```
nice -n 19 /opt/homebrew/bin/python3.14 tools/port/replaydiff/native_capture_png/run_fixture.py --output ~/MacioGalaxy2-gamedata/sold/checked-png-FRESH
```

The runner acquires the actual server POSIX `native.lock` nonblocking before any
compiler/fixture process. It records compiler/SDK/source/executable hashes, uses
one compiler driver, ASan+UBSan, and a 180-second compile/run bound. Private output
is checked below 256 MiB; successful disposable binaries/images are removed after
independent Pillow CRC/decode/dimension/pixel and Python SHA256 checks. Receipt and
logs remain. The process group is owned and reaped before explicit lock release.

29 native controls cover multiblock and one-pixel encoding; short writes/reads;
write/read/flush EINTR; ENOSPC/EIO; zero progress; truncation/corruption; flush,
partial-close, final-close, close-EINTR and publication failure; existing and racing
collisions; null/zero/oversized/overflow dimensions and byte-count mismatch.
Every failed case checks absence of a receipt and unexpected final/partial files;
collision controls verify sentinel content survives. Descriptor counts must match.
Seven successful files receive independent PNG decode and hash checks. These are
synthetic I/O and encoder controls, not GPU or visual-provider acceptance.

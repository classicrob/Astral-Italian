# DVD host file query: directories are not files

This fixes the existing native-only header contract in
`native/platform/include/petari/platform/dvd.hpp`: directories, invalid entries
and unmounted entries return an empty path. The disc-FST loader retains directory
paths internally, but hostPathForEntry omitted an isDir guard. Scanned/overlaid
mounts already stored empty directory paths, making the query inconsistent.

This API has no Wii counterpart. SDK DVDFastOpen in
`src/RVL_SDK/dvd/dvdfs.c` rejects directories, as native DVDFastOpen already does.
The separate case-insensitive lookup is correct and unchanged. The fixture's
original directory/invalid assertion, exposed by server stale-fixtures r2, is
also unchanged on its test-only branch.

The sole external caller found, `native/platform/tools/dvd_smoke.cpp:92`, skips
empty paths then DVDFastOpens the remaining entries. Its assumption agrees with
the documented contract. The missing guard made it report false file-open
failures for directories on disc-FST mounts; no caller change is needed.

The new native_dvd_host_path test creates one private synthetic disc and checks
root / directory / negative / out-of-range / file paths in disc-FST, scanned and
overlaid modes. It also verifies replacement-file paths, unmounted behavior, and
that a nonempty file path corresponds to an SDK-openable file entry. No assets or
user caches are used. Internal FST paths, entry numbers and SDK behavior stay
unchanged. Native execution is pending PORT's server build/run; no M1 build/run.

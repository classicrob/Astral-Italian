# Disc import

`petari_disc_import` is a static C++17 library for first-run setup on macOS. Its public API is
[`disc_import.hpp`](disc_import.hpp). Game data must come from the player's own USA Super Mario Galaxy 2
Wii disc, SB4E01 revision 0. No disc data is included in the library or synthetic tests.

The importer uses encounter/nod, statically linked through Aurora's dependency provider. The nod version
comes from `AuroraDependencyVersions.cmake` in the same pinned Aurora revision used by GX. ISO/GCM,
WBFS, RVZ/WIA, CISO, GCZ, NFS and TGC are read by nod; the public `Format` enum groups the additional
containers with `GcmIso`. GameCube discs and other games, regions or revisions are rejected.
`probe()` uses nod's disc-header API without opening file readers. In the pinned nod version, opening a
Wii disc also inspects partition boot/FST metadata internally; probe does not read game file payloads.

The DATA partition produces exactly this layout:

```
<destination>/files/<original FST paths>
<destination>/sys/boot.bin
<destination>/sys/bi2.bin
<destination>/sys/apploader.img
<destination>/sys/fst.bin
<destination>/sys/main.dol
```

Import requires free space for the DATA partition plus 1 GiB. It writes to `<destination>.partial`, verifies
Wii partition hashes where available, validates the extracted tree, then atomically renames it into place.
The destination must be absent or empty. Cancellation or failure removes only the temporary directory this
import created; an existing partial directory is preserved and reported. Existing extracted folders are
validated in place, with no copying. FST names retain their original bytes; unsafe path components and
conflicting names are rejected.

Validation checks the game header, the repository's expected main.dol SHA-1 (CommonCrypto), system files,
and the shared DVD FST parser. Full validation checks every directory and file size. Quick validation
checks the same metadata and DOL plus `SystemData/ObjNameTable.arc` and `ObjectData/Mario.arc` against
FST sizes, without scanning every host file. Progress callbacks run on the importing thread roughly ten
times per second, plus initial and final notifications; returning false cancels before publication.

Build only this library, CLI and synthetic tests:

```sh
cmake -S native/disc_import -B build/disc-import -DCMAKE_BUILD_TYPE=Release
cmake --build build/disc-import -j6
ctest --test-dir build/disc-import --output-on-failure
build/disc-import/petari_disc_import probe /path/to/disc.iso
build/disc-import/petari_disc_import import /path/to/disc.rvz /path/to/SB4E01
build/disc-import/petari_disc_import validate /path/to/SB4E01
build/disc-import/petari_disc_import validate /path/to/SB4E01 --quick
```

The CMake library target is `petari_disc_import`; the executable target is `petari_disc_import_cli`, with
output name `petari_disc_import`. Normal root configurations expose both targets. The headless build
excludes the importer and does not download or link nod. Link the library into the first-run UI with
`target_link_libraries(<ui-target> PRIVATE petari_disc_import)`.

CLI progress goes to stderr. Ctrl-C cancels safely; `import ... --cancel-at 10` is a deterministic cancellation
check (exit 130). Failure exits 1; invalid command syntax exits 2. To compare an extraction with a reference
manifest, run `native/disc_import/compare_manifest.sh <root> <manifest>`; manifest lines are
`<sha256>  <relative path>`. This checks all listed `files/` and `sys/` paths without modifying them.

Synthetic tests cover ISO, GCM, CISO and WBFS headers, extracted trees, invalid/truncated images (including
RVZ/WIA), FST path safety, file sizes, hashes, progress, cancellation and injected low free space. A private
validation helper accepts a synthetic DOL digest for successful fixture validation; the public API always
requires the repository's real digest. Valid RVZ/WIA conversion is not exercised by those fixtures.

# Debug-symbol gate for every future freeze

This is a mandatory promotion gate for every frozen GX app and headless executable. Complete it while the exact final objects and archive members still exist at the paths recorded by the final link. A successful game smoke or an intact function symbol table does not establish file:line coverage.

## Seal before promotion

1. Pin the final source revision/tree, clean-build receipt, compiler/linker/SDK identities and hashes where available, dependency identities, exact compile/link arguments, executable SHA-256 and each architecture's Mach-O UUID. Record any unavailable historical identity explicitly.
2. Read the final executable's debug map. Resolve EVERY object path relative to its recorded working directory and normalize it with realpath, including paths containing `native/app/../../`. For an archive reference, resolve the archive path separately and bind the named member. Missing entries remain required unresolved inputs; they are never evidence that an object is unused. Hash the exact object/member closure before running dsymutil.
3. Run dsymutil against the final executable with that exact closure present. Retain its command, tool identity, output and diagnostics. Missing-object or incomplete-debug-information diagnostics block acceptance until investigated. Never rebuild approximate objects, alter timestamps or patch the frozen image to manufacture acceptance.
4. Keep the output adjacent to the frozen image: `Petari.app.dSYM` beside `Petari.app`, and `petari_headless.dSYM` beside the headless executable. The archive layout must preserve this association. Rehash the executable afterward and require it to be unchanged.
5. Use `dwarfdump --uuid` on both the final executable and the dSYM DWARF image. Require the complete architecture/UUID sets to match exactly, with no missing or unexpected slice. File existence alone is insufficient.
6. Qualify representative file:line resolution from final linked addresses for native application/platform code, game code, and GX/Aurora code where linked. Select addresses from the final image, bind each address to the matching UUID/architecture, and retain the lookup output plus expected source file/line. A function name without file:line is insufficient. Document any legitimately unavailable category instead of fabricating coverage.
7. Write a per-file SHA-256/size manifest for the complete dSYM bundle and a symbol receipt binding the executable hash/UUIDs, closure manifest, lookup results, source/toolchain/build receipts and final adjacent paths. Record the symbol receipt and dSYM locations in the new build's FROZEN.txt before making that build available.
8. Archive the executable/dSYM association, receipt and manifests privately on the M1. Verify the transferred archive's checksum, member hashes and UUIDs by reading the destination copy. Keep a recoverable manifest-bound copy there independently of server build trees. Do not put game data or private debug artifacts in Git or chat attachments.

## Retention interlock

No compiler object, archive member, generated source or backing directory may be pruned until the adjacent dSYM AND independently verified M1 archive pass the gate above. Re-evaluate normalized debug-map references and active investigation consumers for every proposed deletion; a copied app does not make its debug inputs dispensable. Preserve source mappings and lookup evidence required to interpret the dSYM. Keep the frozen binary and symbol archive independently of disposable build directories.

The promotion checklist and FROZEN.txt must name each frozen image separately; a GX dSYM does not cover headless, and one build's symbols do not cover another build. A missing receipt, mismatched UUID/hash, failed lookup, unverified archive or unresolved debug-map entry blocks symbol acceptance and object pruning. This document does not authorize cleanup or a runtime/build allocation.

## Existing frozen builds

This requirement is prospective. It does not claim that older frozen builds already have dSYMs. Their executables and function symbols remain intact; the recorded incident has 1,321 missing historical object files, limiting file:line detail. Preserve that limitation in the incident/sibling record without rewriting immutable historical FROZEN.txt files. Any later recovery needs its own exact-image and symbol qualification.

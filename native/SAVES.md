# SMG2 unlocked saves

**Status: runtime verification pending port GO. No generated SMG2 save has been
verified or published by this implementation.** Real-disc execution is server-only;
no game assets, saves or disc-derived catalogs belong on the M1, in attachments,
in a repository/build tree or in a team evidence directory. The Python transport
and native contract tests pass with explicitly synthetic data. The engine adapter has a
native compile check; it has not run in the full application.

The former instructions described SMG1: 120-star Mario/Luigi files, observatory
domes, Grand Finale and specific Hungry Luma seeds. They are not SMG2 facts and
are not supported variants. This tool does not parse, construct, patch or invent
save-format bytes, or install anything into a personal NAND.

## Requested profiles

| Variant | Exact selected star identities | Required game readback |
|---|---|---|
| `worlds1-6` | Every non-green star in actual worlds 1 through 6 | Worlds 1–6 open |
| `postgame` | Every non-green star in actual worlds 1 through 7 | Worlds 1–6, postgame and World S open |
| `all-stars` | All 242 actual SB4E01 identities, including green stars | Previous checks, green stars, and Luigi/Master Luma if applicable |

Counts for the smaller profiles come from the loaded SMG2 catalog and save
storage. A scenario row is not a star. `PowerStarId` is a bitmask; multiple
actual `ScenarioNo` values can refer to the same `(galaxy, starId)` record.
Neither the 242 acceptance guard nor a reported positive maximum creates an
inventory: the adapter must independently resolve all 242 records, check the
game's maximum and reverse-check every allocated galaxy record. Partial or
ambiguous inventories fail.

Profiles are exact. An isolated seed with any owned star outside the requested
profile is rejected before mutation, including a green/World S star retained
from a higher-progress seed. Use a lower-progress seed created by the game.
Selected bronze stars must become real stars. This implementation does not reset
a file or silently erase progress. Config/icon data and every unrelated save
record must remain byte-identical through the game's own serializers.

**All three runtime policies currently remain unsupported.** SMG2 world-map
point/path transitions and the World S/Luigi/Master Luma setter/getter rules
must be established from the actual engine before enabling them. A star count
alone does not establish those unlocks. Only Luigi and Master Luma may be marked
not applicable, with explicit SMG2 evidence and readback; this cannot waive a
world or ending requirement.

## Engine adapter and remaining integration

`src/Game/System/NativeUnlockedSave.cpp/.hpp` are entirely `GALAXY_NATIVE` and
are not added to the Wii configure list. They consume the level-select owner's
read-only `CatalogGalaxy` API, not an authored SMG1 table. Native build
integration must include that catalog change first.

The catalog is derived from SMG2 `ScenarioData.bcsv`, `GalaxyInfo.bcsv` and
`GalaxyWorldOrderList.bcsv`. The single shared publication seam is the end of
`GameSystem::initAfterStationedResourceLoaded`, after `mDataHolder->init()`.
The level-select owner publishes first; saves must require `catalogReady()`.
The adapter compares raw masks with the actual parser, validates save-array
bounds before the unchecked engine accessor, checks hidden/green placement
with `MR::getPlacedHiddenStarScenarioNo` and actual mask membership, and reads
owned/bronze state from the actual holder. A helper's fallback scenario 1 is
not accepted without membership.

`Readback` is an asynchronous persistence adapter. The normal event loop must
continue pumping `SaveDataHandleSequence::update` and the NAND worker. It:

1. Loads with the existing `SaveDataHandler::requestLoadGameData`, checks its
   actual result and calls `requestVerifyAfterLoadGameDataFile`. That canonical
   function covers version/header, file size and checksum.
2. Selects an existing file through `startLoadUserFile`, checking both corruption
   flags and creation state. Slot membership comes from the validated game
   directory, not an assumed SMG1 six-slot layout.
3. Saves the selected holder/config with `storeUserFile` and
   `requestSaveSaveData`, preserving config and every other game record.
4. Waits for actual completion, reloads through the same regular game path and
   compares all stored records plus the holder's own serialized readback.

This transport is not a progression certificate. Complete semantic getter
checks, actual policy mutation and bound JSON inventory/request/receipt handling
still need the coordinated runtime frontend. No callbacks are installed into
shared GameSystem/app_main/SaveDataHandleSequence files by this branch; the port
lead owns that integration and the full-app gate. The generic synchronous
`Backend` header is a testable contract, not an implementation of asynchronous
NAND completion.

## Publication protocol

`native/tools/make_unlocked_save.py` requires a cooperating executable and an
explicit `--run`. Until the runtime integration and port GO exist, generation
fails; old CLI/log output cannot satisfy this protocol. Once approved on the
server (these are private-path examples, not local assets):

```sh
python3 native/tools/make_unlocked_save.py \
  --variant all-stars --seed /private/server-gamedata/isolated-game-created-seed \
  --output /private/server-gamedata/new-isolated-all-stars \
  --app /absolute/built/Petari --disc /private/server-gamedata/extracted-SMG2 --run
```

The production driver checks LocalHostName `MacBook-Pro-52` and the existence
of `~/MacioGalaxy2-gamedata/README.txt` before reading/copying a seed. Other hosts,
including the M1, are refused; it reads no asset to perform this check. It refuses
seed/disc/output paths inside any git checkout/worktree or a build/team/upload
area. The executable itself may live in a build tree; assets and generated
artifacts may not. All working copies, catalogs, receipts, logs and NAND ledgers
remain in the private server output parent; do not mirror them as team evidence.

The seed and output must be disjoint. The output must not exist. Personal
`~/Library/Application Support/Petari` and its descendants, symlink NAND entries
and unmarked CLI work directories are refused. The tool copies the seed's NAND
into a new private work directory; the seed is never passed as `--user`.

The first application run is `--make-unlocked-save inventory`. Its JSON, retained privately on the server, must
identify `SB4E01`, the fresh random run ID, provenance, a complete engine-validated
inventory and supported profile/getter rules, plus the actual selected `slot`,
`seed_owned` identities and `seed_owned_count` from the regular loader. The inventory run may not alter
NAND. The tool writes an exact identity plan to `unlocked-save-request.json`
then launches the requested variant. The application must refuse out-of-profile
seed stars **before** changing the holder.

A successful game-written `unlocked-save-receipt.json` must bind the run, plan,
app, seed, inventory and saved-file hashes; contain exact before/after owned
identities and bronze state; and report actual save completion, regular loader,
header/version/checksum, serialized readback, unchanged other slots, clear
corruption flags and every required semantic getter. The Python side rejects
missing, duplicate, stale, unsupported or failed results, invented totals and
extra/lost stars. Receipt provenance is an audit contract with the reviewed game
provider, not proof that an arbitrary executable is truthful.

Only after those checks pass does Python publish the isolated NAND, logs,
inventory, request, receipt and `unlocked-save-nand-ledger.json`. The ledger
contains SHA-256 for every NAND file before inventory, after inventory and after
game save, plus the exact changed-path list. Only `GameData.bin` may change.
`unlocked-save.json` binds the ledger and other artifacts and is written last.
A failed run keeps a private diagnostic work directory, without a published
manifest; an interrupted copy can leave an incomplete output directory which
must not be treated as verified. No `VERIFIED` log line substitutes for readback.

## Small checks

```sh
python3 -B native/tests/unlocked_save_tests.py
clang++ -std=c++17 -Wall -Wextra -Werror -I native/include \
  native/tests/unlocked_save_contract_tests.cpp -o /tmp/unlocked-save-contract
/tmp/unlocked-save-contract /tmp/unlocked-save-contract-test
```

These tests cover identity deduplication, exact profiles/high-progress seed
rejection, incomplete inventory, unresolved flags, save/loader/checksum failure,
corruption, stale receipts, bronze/readback errors, isolation, timeout, refusal
to overwrite and NAND hash-ledger publication. **Their fixtures and receipts are
test doubles, not game data or an end-to-end pass.** Do not commit disc assets,
NAND contents or generated saves. Save-dependent replay coverage remains
`PENDING_GAME_VERIFICATION`; fresh-NAND CPU replay is a separate test.

# SMG2 stage requests

The native frontend accepts `--stage NAME --scenario N` (default `N=1`).
Run `--stage list` first to obtain names and actual scenario IDs from your
SMG2 disc. There is no built-in SMG1 galaxy or mission table.

```sh
Petari --disc /private/extracted/SB4E01 --stage list
Petari --disc /private/extracted/SB4E01 --user /private/test-user --stage '<name from list>' --scenario 2
```

The second command requests the listed **actual ScenarioNo**, not its position
in a carousel. If a galaxy contains IDs 2, 7 and 12, requesting 1 is rejected;
requesting 12 selects 12. IDs are not capped at the donor game's eight missions.
Names may be canonical internal stage names, the English galaxy labels, or the
generated English-name aliases printed by `list`. ASCII case, punctuation,
spaces and a trailing `Galaxy` are ignored. Ambiguous friendly names fail.
Unknown names, missing scenario IDs and incomplete catalogs fail explicitly.

## Runtime catalog and provenance

Before boot, `configureCatalog` reads the extracted disc's own English message
archives through the native RARC/Yaz0/Yay0 resource reader:

- `LocalizeData/UsEnglish/MessageData/SystemMessage.arc`
- `LocalizeData/UsEnglish/MessageData/LayoutMessage.arc`

The bounded MSBT reader obtains `GalaxyName_<internal stage>` labels, decodes
big-endian UTF-16 to UTF-8 and refuses missing/conflicting labels, malformed
records and unsupported formatting codes. It does not invent English names or
fall back to a hand-written translation table. These resource paths and actual
label coverage still need server real-disc validation.

After `GameSystem::mDataHolder->init()`, the native publication hook combines
`ObjectData/SystemDataTable.arc/GalaxyWorldOrderList.bcsv` (`EnName`, `WorldNo`)
with the loaded `ScenarioDataParser` and `GalaxyStatusAccessor`. It requires the
world values to agree with each galaxy's `GalaxyInfo.bcsv`, then reads the actual
rows of `StageData/<stage>/<stage>Scenario.arc/ScenarioData.bcsv`.
Only positive-world galaxy records are included, following the engine's
`ScenarioDataIter` world predicate. Non-galaxy scenes, hub destinations, missing
world-order entries and folder-mod destinations are not fabricated into this
catalog. Completeness against the shipping disc remains a runtime check.

Each `CatalogGalaxy` carries its resource provenance and raw scenario rows:
actual `ScenarioNo`, **PowerStarId bitmask**, `PowerStarType`, comet identifier
and scenario-name identifier. The mask is not a unique star ID, and the number
of rows is not a save-record count. Save tooling must independently validate
actual `(galaxy, starId)` records with the engine. This catalog does not supply
an unlock policy. `list` prints complete integer IDs, tagged C=comet,
H=hidden and G=green, grouped by SMG2 world.

`--stage list` needs game metadata initialization; it is not a static preboot
listing. After successful publication it prints and exits before file selection.
A failed publication must not be treated as a successful list or startup jump.
No game catalog, message data or disc-derived metadata belongs in this repository,
on the M1 or in team attachments. Local tests contain synthetic data only;
real-disc validation runs on the private server.

## Canonical transition and save behavior

An explicit request is consumed once at the normal
`GameSequenceFunction::requestChangeStageAfterFileSelect` seam, after a real
file loads. It may override the initial destination for **any** selected file,
including one with an unfinished prologue/tutorial. The adapter logs that
bypass when World 1 has not been entered; it does not award tutorial progress,
set unlock flags or synthesize a completed file. Without an explicit request,
the normal destination selection is unchanged.

The adapter uses the same selected/placed distinction as
`ScenarioSelectScene::trySetCurrentScenarioNo`. Normal requests use the selected
ID as the placed scenario. Hidden/green requests use the game's
`MR::getPlacedHiddenStarScenarioNo`, then require the returned scenario to exist
and its raw mask to contain the requested star. Unsupported mask bounds or the
helper's unverified fallback fail closed. Dispatch uses only
`GameSequenceFunction::requestChangeStage(stage, placed, selected, nullptr)`;
its normal user-file backup, next-scene state, reset and background handling
remain in the engine. No return destination or earned-star behavior is emulated.
Actual hidden/green placement, subsequent saving and returning from a stage
still require server runtime validation.

The obsolete donor `--test-fixture observatory|stage` and its stage-smoke entry
are rejected with `--stage` guidance. They do not provide SMG2 progression.
Unlocked-save actions are a separate integration; this adapter does not claim
that the imported save CLI generates valid SMG2 saves.

## Shared headless entry

Both frontends use `petari/launch_stage.hpp` and the runtime
`native/app/launch_stage.hpp`. Headless accepts a raw request of the form
`galaxy <name, optionally quoted> scenario <positive actual ID>` and calls
`configureStartupRequest(raw, &error)` followed by `configureCatalog(disc, &error)`
before OSInit. The shared parser owns the request string and resolves it only
once real catalog data has been published.

The status is `Idle`, `PendingCatalog`, `PendingFileLoad`, `Applied` or `Failed`.
`resolvedStartupRequest()` retains the canonical selected destination;
`startupPlacedScenario()` reports the dispatched placed scenario separately.
`Applied` means the canonical change-stage request was dispatched, **not** that
a stage has started. A requested harness run must also observe an initialized
Game scene with a valid player, the resolved stage and the placed scenario
before claiming stage-start arrival. Pending/failed requests cannot qualify.
The headless owner supplies that observer/exit integration separately.

## Home menu and validation limits

The Home menu's catalog labels, world grouping, C/H/G badges, personal-record
lookups and `Go` output use actual scenario IDs. The selection ordinal remains
an internal UI index; an empty catalog has no launchable destination. The donor
HBM bridge still publishes an in-game warp request; an SMG2 GameScene consumer
for that separate in-game warp path has not been integrated or run here.
Startup CLI/headless dispatch is the scoped engine integration above.

Local validation covers synthetic sparse-ID catalogs, aliases and ambiguity,
invalid membership, command parsing and request-state transitions; synthetic
MSBT/RARC/Yaz0 messages and malformed/truncated inputs; and single-TU native
compile checks. These tests do not prove shipping-disc catalog completeness,
engine hook execution, successful stage arrival, live saving or native gameplay.
The engine hooks are delivered as GALAXY_NATIVE patches for the port lead to
apply and gate. No full application, harness or real-disc run was performed by
this lane.

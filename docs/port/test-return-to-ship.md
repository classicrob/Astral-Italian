# Test-only return-to-ship request

Set `PETARI_TEST_RETURN_TO_SHIP=N[:startIndex]` with a positive frame count and
optional nonnegative start index (default 0). The GX and headless frame seams
share the same hook. With the variable absent/empty it does not inspect the game.
Malformed values disable it with a diagnostic.

Use an explicit startup stage request (`--stage` in GX, `--request` in headless).
After that request is applied, only consecutive ready/action frames in its exact
placed stage/scenario count. Loading, demos, pause, wipes, player death/damage,
and an existing transition interrupt the count. On the Nth eligible frame the
hook logs one line and calls
`GameSequenceFunction::requestChangeStageMarioFaceShip(startIndex, -1)` once.
It does not change heaps, sequence nerves, save flags, or scene readiness itself.

The game's first argument is a spawn/start index, not a scenario selector. The
second argument `-1` makes the game select ship scenario 2. The transition runs
through SceneController's ordinary teardown and heap recreation lifecycle.

For the 5.1 investigation, combine this native-only commit with heap diagnostic
`9c3c68ab` in a private M1 build. Run YosshiHomeGalaxy scenario 1 with the normal
synthetic file-select tape and `PETARI_TRACE_BOOT=1`; retain raw traces privately
and report the hook firing, ship arrival or failure, and actual heap diagnostic.
